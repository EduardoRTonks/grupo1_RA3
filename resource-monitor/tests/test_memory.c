#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> // Para memset

#define MB_TO_ALLOCATE 10

int main() {
    printf("Iniciando stress de Memória... (PID: %d)\n", getpid());
    printf("Use o 'resource_monitor' neste PID.\n");
    
    long total_allocated = 0;

    while (1) {
        // Aloca 10 MB
        size_t bytes = MB_TO_ALLOCATE * 1024 * 1024;
        char *mem = (char*)malloc(bytes);
        
        if (mem == NULL) {
            printf("Falha ao alocar mais memória. Total alocado: %ld MB\n", total_allocated);
            perror("malloc");
            sleep(5);
            continue;
        }
        
        // Toca na memória para garantir que ela seja contada no RSS
        memset(mem, 0xAA, bytes); 
        
        total_allocated += MB_TO_ALLOCATE;
        printf("Total alocado: %ld MB\n", total_allocated);
        
        sleep(1); // Aloca a cada 1 segundo
    }
    
    return 0;
}