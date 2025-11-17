#!/bin/bash

# --- Função de Pausa ---
# Aguarda o usuário pressionar Enter para continuar
pause() {
    echo
    read -p "Pressione [Enter] para retornar ao menu..."
}

# --- Verificação Inicial de Permissões ---
# A maioria dos scripts precisa de sudo, vamos verificar uma vez.
if [ "$EUID" -ne 0 ]; then
  echo "ATENÇÃO: Este menu gerencia scripts que exigem 'sudo'."
  echo "Ele pedirá a senha quando necessário."
  echo
fi

# --- Loop Principal do Menu ---
while true; do
    clear # Limpa a tela
    echo "============================================="
    echo "     Menu de Execução - Projeto RA3 (S.O.)"
    echo "============================================="
    echo " [Experimentos Obrigatórios]"
    echo "   1. Experimento 1: Overhead de Monitoramento"
    echo "   2. Experimento 2: Overhead de Namespace"
    echo "   3. Experimento 3: Limite de CPU (Cgroup)"
    echo "   4. Experimento 4: Limite de Memória (Cgroup)"
    echo "   5. Experimento 5: Limite de I/O (Cgroup)"
    echo
    echo " [Ferramentas de Análise]"
    echo "   6. Validar Ferramentas (compare_tools.sh)"
    echo "   7. Visualizador Gráfico (visualize.py)"
    echo
    echo "   0. Sair"
    echo "============================================="
    
    read -p "Escolha uma opção (0-7): " choice

    # --- ATENÇÃO: Todos os caminhos agora incluem 'scripts/' ---
    case "$choice" in
        1)
            echo "--- Executando Experimento 1 (Overhead de Monitoramento)... ---"
            ./scripts/run_monitor_overhead_test.sh
            pause
            ;;
        2)
            echo "--- Executando Experimento 2 (Overhead de Namespace)... ---"
            sudo ./scripts/run_overhead_test.sh
            pause
            ;;
        3)
            echo "--- Executando Experimento 3 (Limite de CPU)... ---"
            sudo ./scripts/run_cgroup_experiment.sh
            pause
            ;;
        4)
            echo "--- Executando Experimento 4 (Limite de Memória)... ---"
            sudo ./scripts/run_memory_limit_test.sh
            pause
            ;;
        5)
            echo "--- Executando Experimento 5 (Limite de I/O)... ---"
            sudo ./scripts/run_io_limit_test.sh
            pause
            ;;
        6)
            echo "--- Iniciando Validação (compare_tools.sh)... ---"
            read -p "  -> Digite o PID do processo para comparar: " pid
            if [[ ! -z "$pid" ]]; then
                sudo ./scripts/compare_tools.sh $pid
            else
                echo "PID inválido. Retornando ao menu."
            fi
            pause
            ;;
        7)
            echo "--- Iniciando Visualizador (visualize.py)... ---"
            read -p "  -> Digite o PID do processo para visualizar: " pid
            if [[ ! -z "$pid" ]]; then
                echo "Aperte Ctrl+C na janela do gráfico (ou aqui) para fechar."
                sudo python3 ./scripts/visualize.py $pid
            else
                echo "PID inválido. Retornando ao menu."
            fi
            pause
            ;;
        0)
            echo "Saindo..."
            exit 0
            ;;
        *)
            echo "Opção inválida. Tente novamente."
            sleep 2 # Pausa rápida para o usuário ler a mensagem
            ;;
    esac
done