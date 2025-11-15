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

#endif // CGROUP_H