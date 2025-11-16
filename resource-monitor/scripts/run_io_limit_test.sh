#!/bin/bash
# Script para executar o Experimento 5: Limitação de I/O

set -e
TEST_IO_PID=""

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# ! IMPORTANTE: Encontre o ID do seu disco principal
# ! Rode 'lsblk' e veja o número "MAJ:MIN" do seu disco
# ! (ex: 8:0, 259:0, etc)
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
DEVICE_ID="8:48"
# Substitua "8:0" pelo ID correto se o script falhar!
# -----------------------------------------------------------

cleanup() {
    echo
    echo "--- Limpando... ---"
    if [ ! -z "$TEST_IO_PID" ]; then
        echo "Matando processo test_io (PID: $TEST_IO_PID)..."
        kill $TEST_IO_PID
    fi
    echo "Limpando cgroup..."
    sudo rmdir /sys/fs/cgroup/teste-io-10m 2>/dev/null || true
    echo "Limpeza concluída."
}

echo "--- Verificando permissões (sudo é necessário)... ---"
if [ "$EUID" -ne 0 ]; then
  echo "ERRO: Este script DEVE ser rodado com sudo."
  exit 1
fi

echo "--- Compilando todas as ferramentas... ---"
make all
make test_io
gcc -o bin/experimento_io tests/experimento_io.c src/cgroup_manager.c -Iinclude -Wall -g

echo "--- Iniciando experimento (Limitação de I/O)... ---"
# Inicia o teste de I/O em background
./bin/test_io &
TEST_IO_PID=$!
echo "Processo test_io iniciado em background com PID: $TEST_IO_PID"

trap cleanup SIGINT

# Aplica o limite de Cgroup, passando o PID e o ID do dispositivo
./bin/experimento_io $TEST_IO_PID $DEVICE_ID
echo
echo "Aguardando 1s para o Cgroup estabilizar..."
sleep 1

echo "--- Iniciando o monitor. Observe (I/O deve limitar em ~10MB/s). ---"
echo "Pressione Ctrl+C para parar o monitor e limpar."
./bin/resource_monitor $TEST_IO_PID