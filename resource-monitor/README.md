# 📖 Monitor de Recursos Linux

Ferramenta de linha de comando para monitorar em tempo real o uso de **CPU**, **Memória** e **I/O** de qualquer processo Linux.

---

## 👥 Autores e Contribuição

| Aluno | Componente(s) | Responsabilidades |
| :--- | :--- | :--- |
| Eduardo Rodrigues Oliveira de Araújo | Profiler (CPU/Mem) + Integração | Implementou `cpu_monitor.c` e `memory_monitor.c`. Fez integração no `main.c` e criou o Makefile. |
| Ricardo Hey | Profiler (I/O/Rede) + Testes | Implementou `io_monitor.c`. Criou scripts e testes (`test_*.c`). Validou precisão. |
| Eduardo Rodrigues Oliveira de Araújo | Namespace Analyzer + Experimentos | Criou `namespace_analyzer.c`. Documentou Experimentos 1 e 2. |
| Ricardo Hey | Control Group Manager + Análise | Implementou `cgroup_manager.c` e realizou Experimentos 3, 4 e 5. |

---

## 🚀 Guia Rápido para Teste (para Leigos)

Você vai precisar de **dois terminais**:

- **Terminal 1:** processo "cobaia".  
- **Terminal 2:** o monitor.

---

## 1. Pré-requisitos

```bash
sudo apt update
sudo apt install build-essential python3-pip python3-matplotlib
pip install -r requirements.txt

## 2. Compilação

```bash
cd nome-da-pasta-do-projeto
make clean
make
```

Isto gera o executável `bin/resource_monitor`.

---

## 3. Testando o Monitor

### 🎯 Passo 1 — Terminal 1: Processo Cobaia

```bash
sleep 600
```

---

### 🎯 Passo 2 — Terminal 2: Vá até o projeto

```bash
cd nome-da-pasta-do-projeto
```

---

### 🎯 Passo 3 — Descobrir o PID

```bash
pgrep sleep
```

Exemplo de saída:

```
12345
```

---

### 🎯 Passo 4 — Rodar o Monitor

```bash
./bin/resource_monitor 12345
```

---

### 🎯 Passo 5 — Ver a Saída

```
================================
PID: 12345
CPU: 0.00 %
MEM (RSS): 768 KB
MEM (Virt): 2632 KB
I/O Leitura: 0.00 MB/s
I/O Escrita: 0.00 MB/s
================================
```

Para parar: **Ctrl + C**

---

## 🔬 Executando os Experimentos da Rubrica

Todos os experimentos devem ser rodados com **sudo**.

---

### Experimento 1 — Overhead do Monitor

```bash
sudo ./scripts/run_monitor_overhead_test.sh
```

---

### Experimento 2 — Overhead de Namespace

```bash
sudo ./scripts/run_overhead_test.sh
```

---

### Experimento 3 — Throttling de CPU

```bash
sudo ./scripts/run_cgroup_experiment.sh
```

---

### Experimento 4 — Limitação de Memória

```bash
sudo ./scripts/run_memory_limit_test.sh
```

---

### Experimento 5 — Limitação de I/O

```bash
sudo ./scripts/run_io_limit_test.sh
```

---

## 📈 Resultados dos Experimentos

Resultados resumidos da execução dos scripts:

---

### **Experimento 1**

```
--- 1. Executando Baseline (sem monitor)... ---
Tempo Baseline: 5.47 segundos

--- 2. Executando com Monitor... ---
Tempo Monitorado: 2.97 segundos

--- 3. Resultados (Experimento 1) ---
Overhead do Monitor: -2.50 segundos
(Nota: O overhead negativo sugere que o cache de disco do sistema tornou a segunda execução (monitorada) mais rápida que a primeira (baseline)).
```

---

### **Experimento 2**

```
--- Executando o teste de overhead... ---
Iniciando medição de overhead de namespace (Flag: 67108864)...
Executando 1000 iterações para média...

======================================================
Resultados Finais (após 1000 iterações):
------------------------------------------------------
  Média Baseline (fork() only):     141.65 us
  Média Isolado (unshare() + fork()): 141.59 us
------------------------------------------------------
  Overhead (Custo Extra por chamada): +-0.06 us
======================================================
(Nota: O overhead da criação de namespaces é estatisticamente zero)
```

---

### **Experimento 3**

```
--- 4. Iniciando experimento... ---
Processo test_cpu iniciado em background com PID: 10595
...
======================================================
Sucesso! O PID 10595 agora está no cgroup 'teste-cpu-50'.
Limite de CPU de 0.50 core(s) aplicado.
======================================================
...
--- 5. Iniciando o monitor... ---
Monitorando PID: 10595 (System HERTZ: 100)
================================
PID: 10595
CPU: 50.00 %
...
--- Cgroup Metrics (PID: 10595) ---
Cgroup CPU: 49.95 %
...
================================
(Validação: O Cgroup CPU foi limitado com sucesso a ~50%)
```

---

### **Experimento 4**

```
--- Iniciando experimento (Limitação de Memória)... ---
Processo test_memory iniciado em background com PID: 10625
...
======================================================
Sucesso! O PID 10625 agora está no cgroup 'teste-mem-100m'.
Limite de memória de 104857600 bytes aplicado.
======================================================
...
--- Iniciando o monitor... ---
Monitorando PID: 10625 (System HERTZ: 100)
...
Total alocado: 30 MB
================================
PID: 10625
MEM (RSS): 32256 KB (31.5 MB)
--- Cgroup Metrics (PID: 10625) ---
Cgroup Mem: 20.2 MB
================================
Total alocado: 40 MB
...
(Validação: A memória subiu a cada alocação; experimento interrompido antes do OOM Killer)
```

---

### **Experimento 5**

```
--- Iniciando experimento (Limitação de I/O)... ---
Processo test_io iniciado em background com PID: 10656
...
======================================================
Sucesso! O PID 10656 agora está no cgroup 'teste-io-10m'.
Limite de I/O de 10485760 B/s aplicado.
Use o ./bin/resource_monitor 10656 para validar.
======================================================
...
--- Iniciando o monitor... ---
Monitorando PID: 10656 (System HERTZ: 100)
================================
PID: 10656
...
I/O Escrita: 10.00 MB/s
--- Cgroup Metrics (PID: 10656) ---
Cgroup I/O W: 10.00 MB/s
...
================================
(Validação: I/O Escrita limitada com sucesso)
```

---

## 🖥️ Visualização Gráfica (Python)

Para rodar a visualização gráfica em tempo real:

1.  **Instale as dependências** (se ainda não o fez):
    ```bash
    # Instala o matplotlib
    sudo apt install python3-matplotlib
    ```
    *(Ou, se você instalou o `pip3`, use `sudo pip3 install -r requirements.txt`)*

2.  **Rode um processo** (ex: `test_cpu`) em um terminal:
    ```bash
    ./bin/test_cpu
    # Anote o PID (ex: 8150)
    ```

3.  **Rode o script** em um segundo terminal (sem `sudo`):
    ```bash
    python3 scripts/visualize.py 8150
    ```

4.  O script pedirá sua senha (para o `resource_monitor` [cite: 171-185] rodar) e abrirá uma janela com os gráficos.

