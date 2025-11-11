Aqui está o `README.md` refeito sem qualquer citação.

-----

````markdown
# Sistema de Monitoramento e Análise de Recursos (RA3)

Este projeto é um sistema de profiling e análise de recursos para Linux, focado nos mecanismos de Namespaces e Control Groups (cgroups), que são a base da tecnologia de contêineres.

O sistema é composto por três ferramentas principais:
1.  **Resource Profiler**: Coleta métricas de processos (CPU, Memória, I/O).
2.  **Namespace Analyzer**: Analisa o isolamento de processos via namespaces.
3.  **Control Group Manager**: Lê métricas e aplica limites de recursos (CPU, Memória, I/O) usando cgroups.

---

## 🛠️ Requisitos e Dependências

* Compilador C/C++ (suporte a C++23)
* `make`
* Bibliotecas padrão do Linux (libc)
* Kernel Linux (recomendado 5.x ou superior)
* **Ambiente de Teste:** O projeto foi desenvolvido e testado no [DISTRIBUIÇÃO, ex: Ubuntu 24.04, Kernel X.Y.Z].

---

## ⚙️ Instruções de Compilação

Para compilar todo o projeto, basta executar o comando `make` na raiz do diretório:

```bash
make
````

Isso irá compilar os executáveis principais (ex: `profiler`, `analyzer`, `manager`) e colocá-los no diretório `bin/`.

Para compilar sem warnings (obrigatório):

```bash
make CFLAGS="-Wall -Wextra"
```

Para limpar os arquivos compilados:

```bash
make clean
```

-----

## 🚀 Instruções de Uso (Com Exemplos)

*(Esta seção deve ser preenchida por vocês com exemplos reais)*

### 1\. Resource Profiler (`profiler`)

Monitora um PID específico em intervalos configuráveis.

```bash
# Exemplo: Monitorar o PID 1234 a cada 2 segundos e salvar em JSON
./bin/profiler --pid 1234 --interval 2 --format json --output metricas.json
```

### 2\. Namespace Analyzer (`analyzer`)

Lista ou compara namespaces.

```bash
# Exemplo: Listar todos os namespaces do PID 1234
./bin/analyzer --pid 1234

# Exemplo: Comparar os namespaces dos PIDs 1234 e 5678
./bin/analyzer --compare 1234 5678
```

### 3\. Control Group Manager (`manager`)

Cria, gerencia e aplica limites.

```bash
# Exemplo: Criar um cgroup novo
./bin/manager --create-cgroup meu_grupo

# Exemplo: Aplicar limite de 1 CPU (100000 quota) ao grupo
./bin/manager --set-cpu-quota meu_grupo 100000

# Exemplo: Mover o PID 9876 para o grupo
./bin/manager --move-pid 9876 meu_grupo
```

-----

## 👨‍💻 Autores e Contribuições

Este trabalho foi realizado em dupla, conforme as regras da disciplina. A divisão de tarefas principal está descrita abaixo.

**Atenção**: Conforme as regras da Prova de Autoria, ambos os membros do grupo contribuíram para a revisão e compreendem a totalidade do código-fonte, incluindo os componentes primariamente desenvolvidos pelo colega.

| Aluno | Contribuição Principal |
| :--- | :--- |
| **Eduardo Rodrigues Araujo de Oliveira** | Implementação do *Resource Profiler* (CPU, Memória, I/O).<br>Implementação do *Namespace Analyzer*.<br>Execução e documentação dos Experimentos 1 e 2. |
| **Ricardo Hey** | Implementação do *Control Group Manager* (Leitura e Aplicação de Limites).<br>Criação dos programas de teste (workloads).<br>Integração do projeto (Makefile) e documentação.<br>Execução e documentação dos Experimentos 3, 4 e 5. |

```
```