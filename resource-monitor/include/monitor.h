#ifndef MONITOR_H
#define MONITOR_H

#include <sys/types.h>

// Struct para guardar métricas de CPU
typedef struct {
    unsigned long utime;  // 14º campo do /proc/[pid]/stat (Tempo de usuário)
    unsigned long stime;  // 15º campo do /proc/[pid]/stat (Tempo de sistema)
} CpuMetrics;

// Struct para guardar métricas de Memória
typedef struct {
    long vm_size_kb; // VmSize (Memória Virtual)
    long vm_rss_kb;  // VmRSS (Memória Física)
} MemoryMetrics;

// --- ADICIONE ISTO ---
// Struct para guardar métricas de I/O
typedef struct {
    unsigned long long rchar; // Bytes lidos
    unsigned long long wchar; // Bytes escritos
} IoMetrics;

// Struct para guardar métricas de Rede
typedef struct {
    unsigned long long rx_bytes;
    unsigned long long rx_packets;
    unsigned long long tx_bytes;
    unsigned long long tx_packets;
} NetworkMetrics;
// --------------------

/*
 * Coleta métricas de CPU lendo /proc/[pid]/stat
 */
CpuMetrics get_cpu_metrics(pid_t pid);

/*
 * Coleta métricas de Memória lendo /proc/[pid]/status
 */
MemoryMetrics get_memory_metrics(pid_t pid);

// --- ADICIONE ISTO ---
/*
 * Coleta métricas de I/O lendo /proc/[pid]/io
 */
IoMetrics get_io_metrics(pid_t pid);

// --------------------
/*
 * Coleta métricas de Rede lendo /proc/[pid]/net/dev
 */
NetworkMetrics get_network_metrics(pid_t pid);

#endif // MONITOR_H