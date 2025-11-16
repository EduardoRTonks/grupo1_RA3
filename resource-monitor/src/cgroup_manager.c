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
/*
 * Encontra o caminho do cgroup v2 para um processo.
 * Lê /proc/[pid]/cgroup e extrai o caminho (ex: /user.slice/...)
 */
static int find_cgroup_path(pid_t pid, char *out_path, size_t out_len) {
    char proc_cgroup_path[256];
    snprintf(proc_cgroup_path, sizeof(proc_cgroup_path), "/proc/%d/cgroup", pid);

    FILE *fp = fopen(proc_cgroup_path, "r");
    if (fp == NULL) return 0; // Falha

    char line[512];
    char path[512];
    int found = 0;

    // Em v2, o formato é: "0::[path]"
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "0::%[^\n]", path) == 1) {
            // O caminho base é /sys/fs/cgroup
            snprintf(out_path, out_len, "/sys/fs/cgroup%s", path);
            found = 1;
            break;
        }
    }
    fclose(fp);
    return found;
}

CgroupMetrics get_cgroup_metrics(pid_t pid) {
    CgroupMetrics metrics = {0};
    char path[512]; 
    char cgroup_path[512]; 

    // 1. Encontrar o caminho base do cgroup v2
    if (!find_cgroup_path(pid, cgroup_path, sizeof(cgroup_path))) {
        return metrics; // Retorna 0 se não achar o caminho
    }

    // 2. CPU (Controlador: cpu)
    // Em v2, 'cpuacct.usage' está em 'cpu.stat' (em microssegundos)
    snprintf(path, sizeof(path), "%s/cpu.stat", cgroup_path);
    FILE *fp_cpu = fopen(path, "r");
    if (fp_cpu) {
        char line[256];
        while (fgets(line, sizeof(line), fp_cpu)) {
            if (strncmp(line, "usage_usec", 10) == 0) {
                unsigned long long usage_us = 0;
                sscanf(line, "usage_usec %llu", &usage_us);
                metrics.cpu_usage_ns = usage_us * 1000; // Converte us para ns
            }
        }
        fclose(fp_cpu);
    }

    // 3. Memória (Controlador: memory)
    // Em v2, 'memory.usage_in_bytes' é 'memory.current'
    snprintf(path, sizeof(path), "%s/memory.current", cgroup_path);
    metrics.memory_usage_bytes = read_cgroup_ull(path);

    // 4. PIDs (Controlador: pids)
    // Em v2, o arquivo 'pids.current' tem o mesmo nome
    snprintf(path, sizeof(path), "%s/pids.current", cgroup_path);
    metrics.pids_current = read_cgroup_long(path);

    // 5. BlkIO (Controlador: io)
    // Em v2, 'blkio.io_service_bytes' está em 'io.stat'
    snprintf(path, sizeof(path), "%s/io.stat", cgroup_path);
    FILE *fp_io = fopen(path, "r");
    if (fp_io) {
        char line[256];
        while (fgets(line, sizeof(line), fp_io)) {
            // Formato: 259:0 rbytes=... wbytes=...
            unsigned long long rbytes = 0;
            unsigned long long wbytes = 0;
            if (strstr(line, "rbytes=") && strstr(line, "wbytes=")) {
                char *ptr_r = strstr(line, "rbytes=");
                char *ptr_w = strstr(line, "wbytes=");
                sscanf(ptr_r, "rbytes=%llu", &rbytes);
                sscanf(ptr_w, "wbytes=%llu", &wbytes);
                metrics.blkio_read_bytes += rbytes;
                metrics.blkio_write_bytes += wbytes;
            }
        }
        fclose(fp_io);
    }

    return metrics;
}
// --- Implementação das Funções de Manipulação ---

/*
 * Cria um novo cgroup (um diretório) em todos os controladores principais
 * Ex: cgroup_create("meu-teste")
 * Cria /sys/fs/cgroup/cpu/meu-teste, /sys/fs/cgroup/memory/meu-teste, etc.
 */

/*
 * Cria um novo cgroup (v2) e habilita os controladores.
 */
int cgroup_create(const char *name) {
    if (getuid() != 0) {
        fprintf(stderr, "Erro: Criar cgroups requer privilégios de root.\n");
        return 0;
    }

    char path[512];
    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s", name);

    // 1. Criar o diretório do cgroup
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Erro ao criar diretório: %s\n", path);
        perror("mkdir (cgroup v2)");
        return 0;
    }

    // 2. Habilitar os controladores necessários (cpu, memory, pids, io)
    // Isso é feito escrevendo no 'cgroup.subtree_control' do cgroup PAI (root)
    const char *controllers_to_enable = "+cpu +memory +pids +io";
    if (!write_cgroup_file("/sys/fs/cgroup/cgroup.subtree_control", controllers_to_enable)) {
        // Ignora erros aqui, pois eles podem já estar habilitados
        // perror("Aviso ao tentar habilitar controladores");
    }
    
    printf("Cgroup v2 '%s' criado (ou já existia).\n", name);
    return 1;
}

/*
 * Move um PID para o cgroup 'name' em todos os controladores
 */
int cgroup_move_process(pid_t pid, const char *name) {
    char path[512];
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);

    // Em v2, o caminho é único para todos os controladores
    // /sys/fs/cgroup/[name]/cgroup.procs
    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/cgroup.procs", name);

    if (!write_cgroup_file(path, pid_str)) {
        // O 'experimento_cgroup.c' já imprime uma mensagem de falha,
        // mas podemos adicionar esta para sermos claros.
        fprintf(stderr, "Falha ao mover PID %d para %s\n", pid, path);
        return 0;
    }

    // A função write_cgroup_file já imprime o erro se falhar
    return 1;
}

/*
 * Aplica um limite de Memória (em bytes) - v2
 */
int cgroup_set_memory_limit(const char *name, long long bytes) {
    char path[512];
    char value_str[64];
    
    // Em v2, o arquivo é 'memory.max'
    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/memory.max", name);
    snprintf(value_str, sizeof(value_str), "%lld", bytes);
    
    if (!write_cgroup_file(path, value_str)) {
        fprintf(stderr, "Falha ao aplicar limite de memória (v2) para %s\n", name);
        return 0;
    }
    printf("Limite de memória de %lld bytes aplicado a %s (v2)\n", bytes, name);
    return 1;
}

/*
 * Aplica um limite de CPU (em "cores") - v2
 * Usa um período padrão de 100ms (100000us)
 */
int cgroup_set_cpu_limit(const char *name, double cores) {
    long long period_us = 100000; // 100ms
    long long quota_us = (long long)(cores * period_us);

    if (quota_us <= 0) {
        fprintf(stderr, "Valor de 'cores' inválido: %f\n", cores);
        return 0;
    }

    char path_cpu_max[512];
    char value_cpu_max[64];

    // Em v2, o formato é "quota period" (ex: "50000 100000")
    snprintf(path_cpu_max, sizeof(path_cpu_max), "/sys/fs/cgroup/%s/cpu.max", name);
    snprintf(value_cpu_max, sizeof(value_cpu_max), "%lld %lld", quota_us, period_us);

    if (!write_cgroup_file(path_cpu_max, value_cpu_max)) {
        fprintf(stderr, "Falha ao aplicar limite de CPU (v2) para %s\n", name);
        return 0;
    }

    printf("Limite de CPU de %.2f core(s) (v2) aplicado a %s\n", cores, name);
    return 1;
}