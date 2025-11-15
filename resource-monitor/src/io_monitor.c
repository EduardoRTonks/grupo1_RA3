#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IoMetrics get_io_metrics(pid_t pid) {
    char path[256];
    // Cria o caminho para o arquivo /proc/[pid]/io
    snprintf(path, sizeof(path), "/proc/%d/io", pid);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        // Trata o erro (processo inexistente ou sem permissão)
        // Imprime o erro, mas só uma vez se o main estiver em loop
        // perror("Erro ao abrir /proc/[pid]/io"); 
        return (IoMetrics){0, 0}; // Retorna struct zerada
    }

    IoMetrics metrics = {0, 0};
    char line[256];

    // Lê o arquivo linha por linha
    while (fgets(line, sizeof(line), fp)) {
        
        // Procura por rchar (bytes lidos)
        if (strncmp(line, "rchar:", 6) == 0) {
            // Use %llu para unsigned long long
            sscanf(line, "rchar: %llu", &metrics.rchar);
        }
        // Procura por wchar (bytes escritos)
        else if (strncmp(line, "wchar:", 6) == 0) {
            // Use %llu para unsigned long long
            sscanf(line, "wchar: %llu", &metrics.wchar);
        }
    }

    fclose(fp);
    return metrics;
}

NetworkMetrics get_network_metrics(pid_t pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/net/dev", pid);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        // Processo pode não ter namespace de rede ou permissão
        return (NetworkMetrics){0, 0, 0, 0};
    }

    NetworkMetrics total_metrics = {0, 0, 0, 0};
    char line[512];

    // Pular as duas linhas de cabeçalho
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    // Lê o arquivo linha por linha (cada linha é uma interface)
    while (fgets(line, sizeof(line), fp)) {
        char iface_name[64];
        unsigned long long rx_b, rx_p, tx_b, tx_p;
        
        // Formato:
        // <iface>: <rx_bytes> <rx_packets> ... (6 campos) ... <tx_bytes> <tx_packets> ...
        int result = sscanf(line, " %[^:]: %llu %llu %*u %*u %*u %*u %*u %*u %llu %llu",
                       iface_name,
                       &rx_b,  // Receive bytes
                       &rx_p,  // Receive packets
                       &tx_b,  // Transmit bytes
                       &tx_p); // Transmit packets

        // Soma os valores de todas as interfaces, exceto 'lo' (loopback)
        if (result == 5 && strncmp(iface_name, "lo", 2) != 0) {
            total_metrics.rx_bytes += rx_b;
            total_metrics.rx_packets += rx_p;
            total_metrics.tx_bytes += tx_b;
            total_metrics.tx_packets += tx_p;
        }
    }

    fclose(fp);
    return total_metrics;
}