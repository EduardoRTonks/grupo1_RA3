#ifndef CGROUP_H
#define CGROUP_H

#include <sys/types.h>

// Struct para métricas de Cgroup (baseado em cgroup v1)
// Conforme requisitos: CPU, Memory, BIIO, PIDs
typedef struct {
    // Controlador: cpuacct (nanossegundos)
    unsigned long long cpu_usage_ns; // (de cpuacct.usage)
    
    // Controlador: memory (bytes)
    unsigned long long memory_usage_bytes; // (de memory.usage_in_bytes)
    
    // Controlador: pids
    long pids_current; // (de pids.current)
    
    // Controlador: blkio (bytes)
    unsigned long long blkio_read_bytes;  // (de blkio.io_service_bytes)
    unsigned long long blkio_write_bytes; // (de blkio.io_service_bytes)

} CgroupMetrics;

/*
 * Coleta métricas de Cgroups lendo /proc/[pid]/cgroup e /sys/fs/cgroup/...
 * [Requisito: Aluno 4]
 */
CgroupMetrics get_cgroup_metrics(pid_t pid);

/*
 * Cria um novo cgroup experimental (requer permissão)
 */
int cgroup_create(const char *name);

/*
 * Move um processo (PID) para um cgroup (requer permissão)
 */
int cgroup_move_process(pid_t pid, const char *name);

/*
 * Aplica um limite de Memória (em bytes) a um cgroup
 */
int cgroup_set_memory_limit(const char *name, long long bytes);

/*
 * Aplica um limite de CPU (em "cores") a um cgroup
 * Ex: 0.5 = 50% de 1 core. 2.0 = 200% (2 cores)
 */
int cgroup_set_cpu_limit(const char *name, double cores);

#endif // CGROUP_H