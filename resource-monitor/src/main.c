#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Para sleep() e sysconf()
#include "monitor.h"
#include "monitor.h"
#include "namespace.h" 
// #include "namespace.h" // Descomente para integrar o Aluno 3

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <PID>\n", argv[0]);
        return 1;
    }

    pid_t pid = atoi(argv[1]);

    // --- PASSO CRÍTICO: Pegar o "clock tick" do sistema ---
    // Essencial para converter 'jiffies' de CPU em segundos
    long HERTZ = sysconf(_SC_CLK_TCK);
    if (HERTZ <= 0) {
        HERTZ = 100; // Valor padrão caso sysconf falhe
    }
    printf("Monitorando PID: %d (System HERTZ: %ld)\n\n", pid, HERTZ);

    // --- PASSO 2: Obter a primeira leitura (T1) ---
    // Precisamos de um ponto de partida para calcular a "diferença"
    CpuMetrics cpu_t1 = get_cpu_metrics(pid);
    IoMetrics io_t1 = get_io_metrics(pid);
    NetworkMetrics net_t1 = get_network_metrics(pid);

    if (cpu_t1.utime == 0 && io_t1.rchar == 0) {
        fprintf(stderr, "Erro ao ler métricas do PID %d. O processo existe?\n", pid);
        return 1;
    }

    // --- PASSO 3: O Loop de Monitoramento ---
    while (1) {
        // Define o intervalo de atualização (1.0 segundo)
        double INTERVALO_SEGUNDOS = 1.0;
        sleep(INTERVALO_SEGUNDOS);

        // --- Obter leituras atuais (T2) ---
        CpuMetrics cpu_t2 = get_cpu_metrics(pid);
        IoMetrics io_t2 = get_io_metrics(pid);
        // Memória é instantânea, não precisa de T1
        MemoryMetrics mem_t2 = get_memory_metrics(pid);
        NetworkMetrics net_t2 = get_network_metrics(pid);

        // --- PASSO 4: Calcular os "Deltas" (T2 - T1) ---
        // 
        
        // Delta de CPU (em 'jiffies')
        unsigned long cpu_jiffies_total_t1 = cpu_t1.utime + cpu_t1.stime;
        unsigned long cpu_jiffies_total_t2 = cpu_t2.utime + cpu_t2.stime;
        unsigned long delta_jiffies = cpu_jiffies_total_t2 - cpu_jiffies_total_t1;

        // Delta de I/O (em bytes)
        unsigned long long delta_read_bytes = io_t2.rchar - io_t1.rchar;
        unsigned long long delta_write_bytes = io_t2.wchar - io_t1.wchar;

        // Delta de Rede (em bytes)
        unsigned long long delta_rx_bytes = net_t2.rx_bytes - net_t1.rx_bytes;
        unsigned long long delta_tx_bytes = net_t2.tx_bytes - net_t1.tx_bytes;

        // --- PASSO 5: Converter Deltas em Percentuais e Taxas ---

        // 5.1: Cálculo de CPU %
        // Converte jiffies em segundos: (delta_jiffies / HERTZ)
        // Divide pelo tempo do intervalo: (segundos_cpu / INTERVALO_SEGUNDOS)
        // Multiplica por 100 para ter o percentual
        double cpu_percent = ((double)delta_jiffies / HERTZ) / INTERVALO_SEGUNDOS * 100.0;

        // 5.2: Cálculo de I/O (em MB/s)
        double read_MBs = (double)delta_read_bytes / (1024.0 * 1024.0) / INTERVALO_SEGUNDOS;
        double write_MBs = (double)delta_write_bytes / (1024.0 * 1024.0) / INTERVALO_SEGUNDOS;

        // 5.3: Cálculo de Rede (em MB/s)
        double read_MBs_net = (double)delta_rx_bytes / (1024.0 * 1024.0) / INTERVALO_SEGUNDOS;
        double write_MBs_net = (double)delta_tx_bytes / (1024.0 * 1024.0) / INTERVALO_SEGUNDOS;

        // --- Imprimir os resultados ---
        printf("================================\n");
        printf("PID: %d\n", pid);
        printf("CPU: %.2f %%\n", cpu_percent);
        printf("MEM (RSS): %ld KB (%.1f MB)\n", mem_t2.vm_rss_kb, mem_t2.vm_rss_kb / 1024.0);
        printf("MEM (Virt): %ld KB\n", mem_t2.vm_size_kb);
        printf("I/O Leitura: %.2f MB/s\n", read_MBs);
        printf("I/O Escrita: %.2f MB/s\n", write_MBs);
        printf("Rede Recebido (Rx): %.2f MB/s\n", read_MBs_net);
        printf("Rede Enviado (Tx): %.2f MB/s\n", write_MBs_net);
        // ... (impressão de CPU, Mem, I/O) ...

        printf("\n--- Namespaces (PID: %d) ---\n", pid);
        list_process_namespaces(pid);

        printf("================================\n");
        // --- Atualizar T1 para a próxima iteração ---
        // A leitura "agora" (T2) vira a leitura "anterior" (T1) no próximo loop
        cpu_t1 = cpu_t2;
        io_t1 = io_t2;
        net_t1 = net_t2;
    }

    return 0;
}