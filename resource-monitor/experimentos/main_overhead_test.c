/*
 * ==================================================================
 * ARQUIVO DE TESTE DO ALUNO 3 (Medição de Overhead de Namespace)
 * * Compilação:
 * gcc -o overhead_test experimentos/main_overhead_test.c
 * * Execução (OBRIGATÓRIO USAR SUDO):
 * sudo ./overhead_test
 * ==================================================================
 */

#define _GNU_SOURCE
#include <sched.h>      // Para unshare() e CLONE_ flags
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // Para fork() e getpid()
#include <sys/wait.h>   // Para waitpid()
#include <time.h>       // Para clock_gettime()

// Quantas vezes vamos repetir o teste para ter uma média estável
#define N_ITERATIONS 1000

// Define qual namespace queremos testar.
// CLONE_NEWUTS (hostname) é leve e não exige setup extra.
// (Você pode trocar por CLONE_NEWPID ou CLONE_NEWNET)
const int NS_FLAG_TO_TEST = CLONE_NEWUTS;

/*
 * Função auxiliar para calcular a diferença de tempo em nanossegundos
 * entre duas medições de clock.
 */
double get_time_diff_ns(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1.0e9 + (end->tv_nsec - start->tv_nsec);
}

/*
 * Teste A: Mede o tempo de um 'fork()' + 'waitpid()' simples.
 * Este é o nosso "baseline" (ponto de comparação).
 */
double measure_baseline() {
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork (baseline) failed");
        exit(1);
    }
    
    if (child_pid == 0) {
        // --- Processo Filho ---
        exit(0); // O filho sai imediatamente
    } else {
        // --- Processo Pai ---
        waitpid(child_pid, NULL, 0); // O pai espera o filho
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    return get_time_diff_ns(&start, &end);
}

/*
 * Teste B: Mede o tempo de 'unshare()' + 'fork()' + 'waitpid()'.
 * Este é o nosso teste de isolamento.
 */
double measure_isolated() {
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // 1. Aplicar o isolamento ao processo atual
    if (unshare(NS_FLAG_TO_TEST) == -1) {
        perror("unshare failed");
        exit(1);
    }

    // 2. Criar o filho (que vai herdar o novo namespace)
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork (isolated) failed");
        exit(1);
    }

    if (child_pid == 0) {
        // --- Processo Filho (Isolado) ---
        exit(0);
    } else {
        // --- Processo Pai ---
        waitpid(child_pid, NULL, 0);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    return get_time_diff_ns(&start, &end);
}


/*
 * ==================================
 * PROGRAMA PRINCIPAL
 * ==================================
 */
int main() {
    double total_time_baseline = 0;
    double total_time_isolated = 0;

    printf("Iniciando medição de overhead de namespace (Flag: %d)...\n", NS_FLAG_TO_TEST);
    printf("Executando %d iterações para média...\n", N_ITERATIONS);
    printf("AGUARDE (isso pode levar alguns segundos)...\n");

    // unshare() requer privilégios de root
    if (getuid() != 0) {
        fprintf(stderr, "\nERRO: Este teste DEVE ser rodado com sudo.\n");
        fprintf(stderr, "      (A chamada 'unshare()' exige privilégios de root)\n");
        return 1;
    }

    for (int i = 0; i < N_ITERATIONS; i++) {
        total_time_baseline += measure_baseline();
        total_time_isolated += measure_isolated();
    }
    
    // --- Cálculo Final ---
    double avg_baseline_ns = total_time_baseline / N_ITERATIONS;
    double avg_isolated_ns = total_time_isolated / N_ITERATIONS;
    double overhead_ns = avg_isolated_ns - avg_baseline_ns;

    // Imprime resultados em microsegundos (us)
    // (1 microsegundo = 1000 nanossegundos)
    printf("\n======================================================\n");
    printf("Resultados Finais (após %d iterações):\n", N_ITERATIONS);
    printf("------------------------------------------------------\n");
    printf("  Média Baseline (fork() only):     %.2f us\n", avg_baseline_ns / 1000.0);
    printf("  Média Isolado (unshare() + fork()): %.2f us\n", avg_isolated_ns / 1000.0);
    printf("------------------------------------------------------\n");
    printf("  Overhead (Custo Extra por chamada): +%.2f us\n", overhead_ns / 1000.0);
    printf("======================================================\n");

    return 0;
}