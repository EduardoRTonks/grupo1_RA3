#!/bin/bash
# Script para executar o Experimento 3: Limite de CPU
# Versão: SILENCIOSA

set -e
TEST_CPU_PID=""

cleanup() {
    echo
    echo "--- Limpando... ---"
    if [ ! -z "$TEST_CPU_PID" ]; then
        echo "Matando processo test_cpu (PID: $TEST_CPU_PID)..."
        kill $TEST_CPU_PID
    fi
    echo "Limpando cgroup..."
    sudo rmdir /sys/fs/cgroup/teste-cpu-50 2>/dev/null || true
    echo "Limpeza concluída."
}

echo "--- Verificando permissões (sudo é necessário)... ---"
if [ "$EUID" -ne 0 ]; then
  echo "ERRO: Este script DEVE ser rodado com sudo."
  exit 1
fi

echo "--- Compilando ferramentas (Aguarde)... ---"
# Silencia a saída do make e do gcc
make all > /dev/null 2>&1
make test_cpu > /dev/null 2>&1
gcc -o bin/experimento_cgroup tests/experimento_cgroup.c src/cgroup_manager.c -Iinclude -Wall > /dev/null 2>&1

echo "--- Iniciando experimento (Limite de CPU 50%)... ---"
./bin/test_cpu &
TEST_CPU_PID=$!
echo "Processo test_cpu iniciado em background com PID: $TEST_CPU_PID"

trap cleanup SIGINT

# Aplica o limite
./bin/experimento_cgroup $TEST_CPU_PID
echo
echo "Aguardando 1s para o Cgroup estabilizar..."
sleep 1

echo "--- Iniciando o monitor. Observe (CPU deve travar em ~50%). ---"
echo "Pressione Ctrl+C para parar o monitor e limpar."
./bin/resource_monitor $TEST_CPU_PID
