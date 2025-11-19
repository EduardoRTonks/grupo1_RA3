#!/bin/bash
# Script para executar o Experimento 4: Limitação de Memória
# Versão: SILENCIOSA

set -e
TEST_MEM_PID=""

cleanup() {
    echo
    echo "--- Limpando... ---"
    if [ ! -z "$TEST_MEM_PID" ]; then
        echo "Matando processo test_memory (PID: $TEST_MEM_PID)..."
        kill $TEST_MEM_PID
    fi
    echo "Limpando cgroup..."
    sudo rmdir /sys/fs/cgroup/teste-mem-100m 2>/dev/null || true
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
make test_memory > /dev/null 2>&1
gcc -o bin/experimento_memory tests/experimento_memory.c src/cgroup_manager.c -Iinclude -Wall > /dev/null 2>&1

echo "--- Iniciando experimento (Limitação de Memória)... ---"
./bin/test_memory &
TEST_MEM_PID=$!
echo "Processo test_memory iniciado em background com PID: $TEST_MEM_PID"

trap cleanup SIGINT

# Aplica o limite
./bin/experimento_memory $TEST_MEM_PID
echo
echo "Aguardando 1s para o Cgroup estabilizar..."
sleep 1

echo "--- Iniciando o monitor. Observe (Memória deve parar em ~100MB). ---"
echo "Pressione Ctrl+C para parar o monitor e limpar."
./bin/resource_monitor $TEST_MEM_PID
