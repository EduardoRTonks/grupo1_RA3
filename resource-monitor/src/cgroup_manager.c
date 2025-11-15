#include "cgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Funções Auxiliares Internas ---

// Função genérica para ler um valor 'unsigned long long' de um arquivo cgroup
static unsigned long long read_cgroup_ull(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return 0;
    unsigned long long val = 0;
    fscanf(fp, "%llu", &val);
    fclose(fp);
    return val;
}

// Função genérica para ler um valor 'long' de um arquivo cgroup
static long read_cgroup_long(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return 0;
    long val = 0;
    fscanf(fp, "%ld", &val);
    fclose(fp);
    return val;
}

/*
 * Encontra o caminho do cgroup para um controlador específico.
 * Lê /proc/[pid]/cgroup, procura pelo 'controller' (ex: "memory"),
 * e monta o caminho completo em 'out_path'.
 * Retorna 1 em sucesso, 0 em falha.
 */
static int find_cgroup_path(pid_t pid, const char *controller, char *out_path, size_t out_len) {
    char proc_cgroup_path[256];
    snprintf(proc_cgroup_path, sizeof(proc_cgroup_path), "/proc/%d/cgroup", pid);

    FILE *fp = fopen(proc_cgroup_path, "r");
    if (fp == NULL) return 0; // Falha

    char line[512];
    char controllers[256], path[256];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Formato: 5:cpu,cpuacct:/user.slice
        if (sscanf(line, "%*d:%[^:]:%[^\n]", controllers, path) == 2) {
            
            char controllers_copy[256];
            strncpy(controllers_copy, controllers, sizeof(controllers_copy));
            controllers_copy[sizeof(controllers_copy) - 1] = '\0';

            char *token = strtok(controllers_copy, ",");
            while (token != NULL) {
                if (strcmp(token, controller) == 0) {
                    snprintf(out_path, out_len, "/sys/fs/cgroup/%s%s", controllers, path);
                    found = 1;
                    break;
                }
                token = strtok(NULL, ",");
            }
        }
        if (found) break;
    }
    fclose(fp);
    return found;
}

// --- Implementação da Função de Leitura ---

CgroupMetrics get_cgroup_metrics(pid_t pid) {
    CgroupMetrics metrics = {0};
    char path[512]; 
    char cgroup_path[512]; 

    // 1. CPU (Controlador: cpuacct)
    if (find_cgroup_path(pid, "cpuacct", cgroup_path, sizeof(cgroup_path))) {
        snprintf(path, sizeof(path), "%s/cpuacct.usage", cgroup_path);
        metrics.cpu_usage_ns = read_cgroup_ull(path);
    }

    // 2. Memória (Controlador: memory)
    if (find_cgroup_path(pid, "memory", cgroup_path, sizeof(cgroup_path))) {
        snprintf(path, sizeof(path), "%s/memory.usage_in_bytes", cgroup_path);
        metrics.memory_usage_bytes = read_cgroup_ull(path);
    }

    // 3. PIDs (Controlador: pids)
    if (find_cgroup_path(pid, "pids", cgroup_path, sizeof(cgroup_path))) {
        snprintf(path, sizeof(path), "%s/pids.current", cgroup_path);
        metrics.pids_current = read_cgroup_long(path);
    }

    // 4. BlkIO (Controlador: blkio)
    if (find_cgroup_path(pid, "blkio", cgroup_path, sizeof(cgroup_path))) {
        snprintf(path, sizeof(path), "%s/blkio.io_service_bytes", cgroup_path);
        FILE *fp_blkio = fopen(path, "r");
        if (fp_blkio) {
            char line[256];
            while (fgets(line, sizeof(line), fp_blkio)) {
                unsigned long long bytes;
                if (sscanf(line, " %*s Read %llu", &bytes) == 1) {
                    metrics.blkio_read_bytes += bytes;
                } else if (sscanf(line, " %*s Write %llu", &bytes) == 1) {
                    metrics.blkio_write_bytes += bytes;
                }
            }
            fclose(fp_blkio);
        }
    }

    return metrics;
}