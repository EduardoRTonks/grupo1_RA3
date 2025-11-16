# Arquitetura do Resource Monitor

Este documento detalha a arquitetura do software, os componentes e o fluxo de dados do `resource_monitor`.

## 1. Visão Geral

O `resource_monitor` é um programa em C que opera lendo arquivos dos pseudo-filesystems `/proc` e `/sys/fs/cgroup`. Ele é estruturado em módulos independentes (para CPU, Memória, I/O, etc.), que são orquestrados por um loop principal (`main.c`).

O projeto não utiliza bibliotecas externas, dependendo apenas da `libc` e das bibliotecas padrão do sistema, conforme os requisitos da rubrica.

## 2. Estrutura de Diretórios

A estrutura segue o padrão C e os requisitos da atividade:

* `bin/`: Contém os executáveis finais (o monitor e os testes).
* `build/`: Contém os arquivos-objeto (`.o`) intermediários.
* `docs/`: Contém esta documentação.
* `include/`: Contém os arquivos de cabeçalho (`.h`) que definem as structs e protótipos.
* `src/`: Contém todos os arquivos de implementação (`.c`).
* `tests/`: Contém os programas de "stress" (`test_*.c`) e os programas de "experimento" (`experimento_*.c`).
* `scripts/`: Contém scripts auxiliares de análise e visualização.
* `Makefile`: Orquestra a compilação.

## 3. Detalhamento dos Componentes

### Componente 1: Resource Profiler

A coleta de métricas do processo é feita pela leitura de arquivos em `/proc/<PID>/`.

* **CPU (`cpu_monitor.c`):** Lê `/proc/<PID>/stat`. Captura o 14º (`utime`) e 15º (`stime`) campos. O uso percentual é calculado por **delta** (T2 - T1) dividido pelos *jiffies* do sistema (`sysconf(_SC_CLK_TCK)`).
* **Memória (`memory_monitor.c`):** Lê `/proc/<PID>/status` linha por linha. Faz o parse das linhas `VmSize:` (Virtual) e `VmRSS:` (Física).
* **I/O (`io_monitor.c`):** Lê `/proc/<PID>/io` (requer privilégios). Faz o parse das linhas `rchar:` (bytes lidos) e `wchar:` (bytes escritos). A taxa (MB/s) é calculada por **delta** (T2 - T1).
* **Rede (em `io_monitor.c`):** Implementado pelo Aluno 2. Lê `/proc/<PID>/net/dev`. Pula os cabeçalhos e soma as colunas `rx_bytes` e `tx_bytes` de todas as interfaces (exceto `lo`). A taxa (MB/s) é calculada por **delta** (T2 - T1).

### Componente 2: Namespace Analyzer (`namespace_analyzer.c`)

Este módulo analisa o isolamento, conforme a tarefa do Aluno 3.

* **Lógica:** Ele lê o diretório `/proc/<PID>/ns`.
* Para cada link simbólico (ex: `net`, `uts`, `pid`), ele usa a syscall `readlink()` para descobrir o destino (ex: `net:[4026531838]`).
* A **comparação** entre dois PIDs é feita comparando as strings de destino desses links. Se forem iguais, os PIDs compartilham o mesmo namespace.

### Componente 3: Control Group Manager (`cgroup_manager.c`)

Este módulo implementa a leitura e manipulação de Cgroups (v1/v2), conforme a tarefa do Aluno 4.

* **Leitura de Métricas:** É um processo de duas etapas:
    1.  Primeiro, ele lê `/proc/<PID>/cgroup` para descobrir a qual sub-caminho o processo pertence.
    2.  Com esse caminho, ele monta o caminho completo no VFS (ex: `/sys/fs/cgroup/memory/.../memory.usage_in_bytes`) e lê o valor.
* **Manipulação de Cgroups:** A manipulação é feita escrevendo em arquivos específicos (requer `sudo`):
    * **Criação (`cgroup_create`):** Executa `mkdir` nos diretórios dos controladores.
    * **Movimentação (`cgroup_move_process`):** Escreve o PID no arquivo `cgroup.procs` do cgroup de destino.
    * **Limites (`set_cpu_limit`, `set_memory_limit`, `set_io_limit`):** Escreve valores nos arquivos de configuração (ex: `memory.limit_in_bytes` ou `cpu.cfs_quota_us`).

## 4. Fluxo de Integração (`main.c`)

O `main.c` atua como o orquestrador (tarefa do Aluno 1).

1.  Recebe o `<PID>` como argumento (e flags opcionais como `--csv`).
2.  Obtém o `HERTZ` do sistema (`sysconf(_SC_CLK_TCK)`).
3.  Entra em um loop infinito (`while(1)`).
4.  **No início do loop (T1):** Coleta métricas que precisam de delta (CPU, I/O, Rede, Cgroup CPU).
5.  **Pausa:** Dorme por 1 segundo (`sleep(1)`).
6.  **Leitura (T2):** Coleta as métricas atuais (CPU, I/O, Rede, Cgroup) e as métricas instantâneas (Memória, Namespace).
7.  **Cálculo:** Calcula os deltas (T2 - T1) para CPU, I/O e Rede para obter as taxas por segundo.
8.  **Exibição:** Imprime todos os dados formatados no `stdout` (ou em CSV, se a flag for passada).
9.  **Atualização:** Os valores de T2 se tornam os novos T1 para a próxima iteração.