#include "cgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // Para mkdir
#include <errno.h>    // Para errno
#include <unistd.h>   // Para getuid

// Escreve um valor (string) em um arquivo de cgroup
static int write_cgroup_file(const char *path, const char *value) {
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        perror("Erro ao abrir arquivo cgroup para escrita");
        return 0;
    }
    if (fprintf(fp, "%s", value) < 0) {
        perror("Erro ao escrever no arquivo cgroup");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

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

// --- Implementação das Funções de Manipulação ---

/*
 * Cria um novo cgroup (um diretório) em todos os controladores principais
 * Ex: cgroup_create("meu-teste")
 * Cria /sys/fs/cgroup/cpu/meu-teste, /sys/fs/cgroup/memory/meu-teste, etc.
 */
int cgroup_create(const char *name) {
    if (getuid() != 0) {
        fprintf(stderr, "Erro: Criar cgroups requer privilégios de root.\n");
        return 0;
    }

    // Lista de controladores obrigatórios
    const char *controllers[] = {"cpu,cpuacct", "memory", "pids", "blkio"};
    int num_controllers = 4;
    int success = 1;

    char path[512];
    for (int i = 0; i < num_controllers; i++) {
        snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/%s", controllers[i], name);
        
        // mkdir(path, 0755) -> 0755 são as permissões
        if (mkdir(path, 0755) != 0 && errno != EEXIST) {
            fprintf(stderr, "Erro ao criar diretório: %s\n", path);
            perror("mkdir");
            success = 0;
        }
    }
    
    if(success) printf("Cgroup '%s' criado (ou já existia) em %d controladores.\n", name, num_controllers);
    return success;
}

/*
 * Move um PID para o cgroup 'name' em todos os controladores
 */
int cgroup_move_process(pid_t pid, const char *name) {
    // Lista de controladores obrigatórios
    const char *controllers[] = {"cpu,cpuacct", "memory", "pids", "blkio"};
    int num_controllers = 4;
    int success = 1;

    char path[512];
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);

    for (int i = 0; i < num_controllers; i++) {
        snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/%s/cgroup.procs", controllers[i], name);
        
        if (!write_cgroup_file(path, pid_str)) {
            fprintf(stderr, "Falha ao mover PID %d para o cgroup %s (controlador: %s)\n", pid, name, controllers[i]);
            success = 0;
        }
    }
    
    if(success) printf("PID %d movido para o cgroup '%s'.\n", pid, name);
    return success;
}

/*
 * Aplica um limite de Memória (em bytes)
 */
int cgroup_set_memory_limit(const char *name, long long bytes) {
    char path[512];
    char value_str[64];
    
    snprintf(path, sizeof(path), "/sys/fs/cgroup/memory/%s/memory.limit_in_bytes", name);
    snprintf(value_str, sizeof(value_str), "%lld", bytes);
    
    if (!write_cgroup_file(path, value_str)) {
        fprintf(stderr, "Falha ao aplicar limite de memória para %s\n", name);
        return 0;
    }
    printf("Limite de memória de %lld bytes aplicado a %s\n", bytes, name);
    return 1;
}

/*
 * Aplica um limite de CPU (em "cores")
 * Usa um período padrão de 100ms (100000us)
 */
int cgroup_set_cpu_limit(const char *name, double cores) {
    long long period_us = 100000; // 100ms
    long long quota_us = (long long)(cores * period_us);

    if (quota_us <= 0) {
        fprintf(stderr, "Valor de 'cores' inválido: %f\n", cores);
        return 0;
    }

    char path_period[512], path_quota[512];
    char value_period[64], value_quota[64];

    // Define o período (100ms)
    snprintf(path_period, sizeof(path_period), "/sys/fs/cgroup/cpu,cpuacct/%s/cpu.cfs_period_us", name);
    snprintf(value_period, sizeof(value_period), "%lld", period_us);
    if (!write_cgroup_file(path_period, value_period)) {
        fprintf(stderr, "Falha ao aplicar período de CPU para %s\n", name);
        return 0;
    }

    // Define a quota (ex: 50000us para 0.5 cores)
    snprintf(path_quota, sizeof(path_quota), "/sys/fs/cgroup/cpu,cpuacct/%s/cpu.cfs_quota_us", name);
    snprintf(value_quota, sizeof(value_quota), "%lld", quota_us);
    if (!write_cgroup_file(path_quota, value_quota)) {
        fprintf(stderr, "Falha ao aplicar quota de CPU para %s\n", name);
        return 0;
    }

    printf("Limite de CPU de %.2f core(s) (quota: %lldus, período: %lldus) aplicado a %s\n", cores, quota_us, period_us, name);
    return 1;
}