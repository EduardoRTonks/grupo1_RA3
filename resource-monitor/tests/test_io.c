#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CHUNK_SIZE (10 * 1024 * 1024) // 10 MB
#define FILE_NAME "temp_io_test.dat"

int main() {
    printf("Iniciando stress de I/O... (PID: %d)\n", getpid());
    printf("Use o 'resource_monitor' neste PID.\n");

    // Cria um buffer de 10MB
    char *buffer = (char*)malloc(CHUNK_SIZE);
    if (buffer == NULL) {
        perror("malloc buffer");
        return 1;
    }
    for(int i = 0; i < CHUNK_SIZE; i++) buffer[i] = (char)i;

    while (1) {
        // --- Fase de Escrita ---
        FILE *fp = fopen(FILE_NAME, "wb");
        if (fp == NULL) {
            perror("fopen write");
            sleep(1); continue;
        }
        printf("Escrevendo 10MB...\n");
        fwrite(buffer, 1, CHUNK_SIZE, fp);
        fclose(fp);
        
        // --- Fase de Leitura ---
        fp = fopen(FILE_NAME, "rb");
        if (fp == NULL) {
            perror("fopen read");
            sleep(1); continue;
        }
        printf("Lendo 10MB...\n");
        fread(buffer, 1, CHUNK_SIZE, fp);
        fclose(fp);
        
        remove(FILE_NAME);
        usleep(100 * 1000); // 100ms
    }

    free(buffer);
    return 0;
}