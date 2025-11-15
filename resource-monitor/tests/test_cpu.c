#include <stdio.h>
#include <unistd.h> // Para getpid()

int main() {
    printf("Iniciando stress de CPU... (PID: %d)\n", getpid());
    printf("Use o 'resource_monitor' neste PID.\n");
    
    // Loop infinito para consumir CPU
    while (1) {
        // Operação matemática simples
        double x = 1.234 * 5.678;
    }
    
    return 0;
}