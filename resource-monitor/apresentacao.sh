#!/bin/bash

# Cores
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

function pause(){
    echo -e "${CYAN}>>> Pressione [ENTER] para voltar ao menu...${NC}"
    read
}

while true; do
    clear
    echo -e "${GREEN}=================================================${NC}"
    echo -e "${GREEN}   SISTEMA DE DEFESA RA3 - EDUARDO & RICARDO     ${NC}"
    echo -e "${GREEN}=================================================${NC}"
    echo "1. [Compilação] Limpar e Recompilar (Make)"
    echo "2. [Básico] Monitorar Processo (Temporizado)"
    echo "-------------------------------------------------"
    echo "3. [Controle] Limite de CPU (Exp 3 - 50%)"
    echo "4. [Controle] Limite de MEMÓRIA (Exp 4 - 100MB) [NOVO]"
    echo "5. [Controle] Limite de I/O (Exp 5 - 10MB/s)    [NOVO]"
    echo "-------------------------------------------------"
    echo "6. [Isolamento] Teste de Namespaces (Exp 2)"
    echo "7. [Bônus] Visualização Gráfica (Estresse CPU)"
    echo "8. [Visual] Visualização CAÓTICA (CPU + RAM + I/O)"
    echo "0. Sair"
    echo -e "${GREEN}=================================================${NC}"
    read -p "Escolha uma opção: " op

    case $op in
        1)
            echo -e "\n${CYAN}>>> Executando 'make clean && make'...${NC}"
            make clean && make
            pause
            ;;
        2)
            read -p "Por quantos segundos você quer monitorar? " TEMPO
            echo -e "\n${CYAN}>>> Iniciando processo 'sleep 600' em background...${NC}"
            sleep 600 &
            PID_SLEEP=$!
            echo -e ">>> PID detectado: $PID_SLEEP"
            echo -e ">>> Monitorando por $TEMPO segundos..."
            sleep 1
            timeout $TEMPO ./bin/resource_monitor $PID_SLEEP
            echo -e "\n>>> Tempo esgotado! Encerrando..."
            kill $PID_SLEEP 2>/dev/null
            pause
            ;;
        3)
            echo -e "\n${CYAN}>>> Experimento 3: Limite de CPU (50%)...${NC}"
            read -p "Duração do teste (segundos): " TEMPO
            echo ">>> A senha de SUDO será solicitada agora."
            sudo timeout -s SIGINT ${TEMPO}s ./scripts/run_cgroup_experiment.sh
            pause
            ;;
        4)
            echo -e "\n${CYAN}>>> Experimento 4: Limite de Memória (100MB)...${NC}"
            read -p "Duração do teste (segundos): " TEMPO
            echo ">>> O processo tentará alocar memória infinitamente."
            echo ">>> O Cgroup deve barrar em 100MB."
            sudo timeout -s SIGINT ${TEMPO}s ./scripts/run_memory_limit_test.sh
            pause
            ;;
        5)
            echo -e "\n${CYAN}>>> Experimento 5: Limite de I/O (10MB/s)...${NC}"
            read -p "Duração do teste (segundos): " TEMPO
            echo ">>> Tentando escrever no disco à velocidade máxima..."
            echo ">>> O Cgroup deve limitar a ~10MB/s."
            sudo timeout -s SIGINT ${TEMPO}s ./scripts/run_io_limit_test.sh
            pause
            ;;
        6)
            echo -e "\n${CYAN}>>> Testando Overhead de Namespaces...${NC}"
            sudo ./scripts/run_overhead_test.sh
            pause
            ;;
        7)
            echo -e "\n${CYAN}>>> Iniciando 'test_cpu' (LINHA RETA)...${NC}"
            ./bin/test_cpu &
            PID_GRAPH=$!
            echo -e ">>> PID detectado: $PID_GRAPH"
            echo -e ">>> Abrindo Python..."
            sudo python3 scripts/visualize.py $PID_GRAPH
            kill $PID_GRAPH 2>/dev/null
            pause
            ;;
        8)
            echo -e "\n${CYAN}>>> Iniciando 'CARGA TOTAL' (CPU, RAM, DISCO)...${NC}"
            
            # --- CORREÇÃO AQUI ---
            echo ">>> Digite sua senha de sudo AGORA para autorizar o visualizador:"
            sudo -v  # Atualiza as credenciais do sudo antes de começar a bagunça
            
            # Inicia o caos silenciado (para não poluir o terminal)
            # Ou mantemos ele visível, mas agora o sudo já está autorizado
            python3 scripts/carga_caos.py &
            PID_CHAOS=$!
            echo -e ">>> PID detectado: $PID_CHAOS"
            echo -e ">>> Abrindo Visualizador..."
            
            # Roda o visualizador (vai usar o sudo cacheado)
            sudo python3 scripts/visualize.py $PID_CHAOS
            
            echo -e "\n>>> Matando gerador de carga..."
            kill $PID_CHAOS 2>/dev/null
            pause
            ;;
        0)
            echo "Saindo..."
            pkill -f "test_cpu"
            pkill -f "test_memory"
            pkill -f "test_io"
            pkill -f "sleep 600"
            pkill -f "carga_caos.py"
            exit 0
            ;;
        *)
            echo "Opção inválida."
            sleep 1
            ;;
    esac
done
