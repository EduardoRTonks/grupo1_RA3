/*
 * ==================================================================
 * ARQUIVO DE EXPERIMENTO (ALUNO 4)
 * * Este programa aplica um limite de CPU a um PID existente.
 * Ele usa as funções definidas em cgroup.h/cgroup_manager.c
 * ==================================================================
 */

#include <stdio.h>
#include <stdlib.h> // Para atoi()
#include <unistd.h>   // Para getuid()
#include "cgroup.h" // Inclui nossas funções de Cgroup

// --- Definições do Experimento ---
const char* CGROUP_NAME = "teste-cpu-50"; // Nome do cgroup que vamos criar
const double CPU_LIMIT_CORES = 0.5;      // Limite de 50% de 1 core
// -----------------------------------

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <PID_para_limitar>\n", argv[0]);
        return 1;
    }

    // A criação de Cgroups requer root
    if (getuid() != 0) {
        fprintf(stderr, "ERRO: Este experimento DEVE ser rodado com sudo.\n");
        return 1;
    }

    pid_t pid = atoi(argv[1]);

    printf("Iniciando experimento de Cgroup para PID: %d\n", pid);

    // 1. Criar o cgroup (ex: /sys/fs/cgroup/cpu/teste-cpu-50)
    // [Função do cgroup.h / cgroup_manager.c]
    if (!cgroup_create(CGROUP_NAME)) {
        fprintf(stderr, "Falha ao criar o cgroup '%s'.\n", CGROUP_NAME);
        return 1;
    }

    // 2. Aplicar o limite de 50% de CPU
    // [Função do cgroup.h / cgroup_manager.c]
    if (!cgroup_set_cpu_limit(CGROUP_NAME, CPU_LIMIT_CORES)) {
        fprintf(stderr, "Falha ao aplicar limite de CPU.\n");
        return 1;
    }

    // 3. Mover o PID de teste para dentro do cgroup
    // [Função do cgroup.h / cgroup_manager.c]
    if (!cgroup_move_process(pid, CGROUP_NAME)) {
        fprintf(stderr, "Falha ao mover PID %d para o cgroup.\n", pid);
        return 1;
    }

    printf("\n======================================================\n");
    printf("Sucesso! O PID %d agora está no cgroup '%s'.\n", pid, CGROUP_NAME);
    printf("Limite de CPU de %.2f core(s) aplicado.\n", CPU_LIMIT_CORES);
    printf("Use o ./bin/resource_monitor %d para validar.\n", pid);
    printf("======================================================\n");

    return 0;
}