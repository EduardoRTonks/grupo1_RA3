#!/bin/bash

# --- Configuração ---
# O 'set -e' faz o script parar se qualquer comando falhar.
set -e

# Esta variável guardará o PID do nosso processo de teste
TEST_CPU_PID=""

# --- 1. Função de Limpeza ---
# Esta função será chamada automaticamente quando você pressionar Ctrl+C
cleanup() {
    echo
    echo "--- 5. Limpando... ---"
    
    # Verifica se a variável de PID foi definida
    if [ ! -z "$TEST_CPU_PID" ]; then
        echo "Matando processo test_cpu (PID: $TEST_CPU_PID)..."
        # Usa 'kill' para parar o processo de teste em background
        kill $TEST_CPU_PID
    fi
    
    # Informa como remover os cgroups (já que o cgroup_manager.c não tem 'delete')
    echo "sudo rmdir /sys/fs/cgroup/teste-cpu-50"
    echo "Limpeza concluída."
}

# --- 2. Verificação de Permissão ---
echo "--- Verificando permissões (sudo é necessário)... ---"
if [ "$EUID" -ne 0 ]; then
  echo "ERRO: Este script DEVE ser rodado com sudo."
  echo "Exemplo: sudo ./run_experiment.sh"
  exit 1
fi

# --- 3. Compilação ---
echo "--- 3. Compilando todas as ferramentas... ---"
# Compila o monitor principal (resource_monitor)
make all
# Compila o teste de CPU (test_cpu)
make test_cpu
# Compila o programa de experimento (experimento_cgroup)
gcc -o bin/experimento_cgroup tests/experimento_cgroup.c src/cgroup_manager.c -Iinclude -Wall -g
echo "Compilação concluída."
echo

# --- 4. Execução do Experimento ---
echo "--- 4. Iniciando experimento... ---"

# Inicia o 'test_cpu' em background (com '&')
./bin/test_cpu &
# Captura o PID do processo que acabamos de iniciar
TEST_CPU_PID=$!
echo "Processo test_cpu iniciado em background com PID: $TEST_CPU_PID"

# AGORA que temos o PID, ativamos o 'trap'
# Se o usuário pressionar Ctrl+C (SIGINT), a função 'cleanup' será chamada
trap cleanup SIGINT

# Aplica o limite de Cgroup usando o programa de experimento
./bin/experimento_cgroup $TEST_CPU_PID
echo
echo "Aguardando 1s para o Cgroup estabilizar..."
sleep 1

# --- 5. Validação ---
echo "--- 5. Iniciando o monitor. Observe o resultado (CPU deve ser ~50%). ---"
echo "Pressione Ctrl+C para parar o monitor e limpar."
echo

# Inicia o monitor. O script ficará 'parado' nesta linha
# até você pressionar Ctrl+C
./bin/resource_monitor $TEST_CPU_PID

# Quando você pressionar Ctrl+C:
# 1. O 'resource_monitor' será interrompido.
# 2. O 'trap' será acionado, chamando a função 'cleanup'.
# 3. O 'cleanup' matará o 'test_cpu' que estava em background.
