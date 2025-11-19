#!/bin/bash
# Script para executar o Experimento 5: Limitação de I/O
# Versão: SILENCIOSA (Professional Mode)

set -e
TEST_IO_PID=""

# --- DETECÇÃO INTELIGENTE DE DISCO ---
ROOT_DEV=$(findmnt -n -o SOURCE /)
PARENT_NAME=$(lsblk -no PKNAME $ROOT_DEV)

if [ -z "$PARENT_NAME" ]; then
    TARGET_DEV=$ROOT_DEV
else
    TARGET_DEV="/dev/$PARENT_NAME"
fi

DEVICE_ID=$(lsblk -d -n -o MAJ:MIN $TARGET_DEV)

echo "--- Configuração de I/O ---"
echo "Disco Alvo: $TARGET_DEV ($DEVICE_ID)"
echo "---------------------------"

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

echo "--- Compilando ferramentas (Aguarde)... ---"
# O '> /dev/null 2>&1' joga toda a saída (texto e avisos) no lixo
make all > /dev/null 2>&1
make test_io > /dev/null 2>&1
gcc -o bin/experimento_io tests/experimento_io.c src/cgroup_manager.c -Iinclude -Wall > /dev/null 2>&1

echo "--- Iniciando experimento (Limitação de I/O)... ---"
./bin/test_io &
TEST_IO_PID=$!
echo "Processo test_io iniciado em background com PID: $TEST_IO_PID"

trap cleanup SIGINT

# Aplica o limite
./bin/experimento_io $TEST_IO_PID $DEVICE_ID
echo
echo "Aguardando 1s para o Cgroup estabilizar..."
sleep 1

echo "--- Iniciando o monitor. Observe (I/O deve limitar em ~10MB/s). ---"
echo "Pressione Ctrl+C para parar o monitor e limpar."
./bin/resource_monitor $TEST_IO_PID
