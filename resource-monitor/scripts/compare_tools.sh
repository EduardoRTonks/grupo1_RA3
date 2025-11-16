#!/bin/bash

# scripts/compare_tools.sh
# Script do Aluno 2 para validar a precisão das medições,
# comparando com as ferramentas padrão do sistema[cite: 283].

if [ "$EUID" -ne 0 ]; then
  echo "ERRO: Este script precisa rodar com sudo para acessar iotop e systemd-cgtop."
  exit 1
fi

if [ -z "$1" ]; then
  echo "Uso: sudo ./scripts/compare_tools.sh <PID>"
  exit 1
fi

PID_TO_MONITOR=$1

echo "========================================================"
echo "          VALIDAÇÃO DO RESOURCE MONITOR (Aluno 2)"
echo "========================================================"
echo "Vamos comparar 3 ferramentas lado a lado."
echo "PID Alvo: $PID_TO_MONITOR"
echo

# 1. Rodar nosso monitor em background
echo "--- 1. Iniciando nosso resource_monitor (em background)..."
./bin/resource_monitor $PID_TO_MONITOR > ./monitor_output.log & 
MONITOR_PID=$!
echo "Nosso monitor está rodando com PID $MONITOR_PID. Logs em monitor_output.log"
sleep 1

# 2. Rodar o systemd-cgtop para Cgroups e CPU/Mem
echo
echo "--- 2. Iniciando systemd-cgtop (para Cgroup, CPU, Memória)..."
echo "Pressione 'q' para sair desta ferramenta e ir para a próxima."
sleep 2
systemd-cgtop
echo "systemd-cgtop finalizado."

# 3. Rodar o iotop para I/O
echo
echo "--- 3. Iniciando iotop (para I/O)..."
echo "Pressione 'q' para sair."
sleep 1
iotop -p $PID_TO_MONITOR
echo "iotop finalizado."

# 4. Limpeza
echo
echo "--- 4. Finalizando nosso resource_monitor (PID $MONITOR_PID)..."
kill $MONITOR_PID
wait $MONITOR_PID 2>/dev/null
echo "Limpeza concluída."
echo
echo "Veja o arquivo 'monitor_output.log' para comparar os dados."
echo "========================================================"