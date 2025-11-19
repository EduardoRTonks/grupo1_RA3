import time
import os
import math
import gc

# Configurações
MEMORIA_MB = 100    # Aloca 100MB de RAM
ARQUIVO_TEMP = "lixo_temporario.dat"
CHUNK_SIZE = 1024 * 1024 # 1 MB por vez

print(f"PID do Caos: {os.getpid()}")
print("Gerando carga CONTÍNUA em CPU, Memória e I/O...")

def estresse_misto(duracao):
    """
    Faz tudo ao mesmo tempo:
    - Queima CPU
    - Escreve no Disco sem parar
    """
    fim = time.time() + duracao
    
    # Abre o arquivo para escrita contínua
    try:
        with open(ARQUIVO_TEMP, "wb") as f:
            while time.time() < fim:
                # 1. CPU: Faz uma conta pesada
                math.sqrt(12345.6789) * math.sqrt(98765.4321)
                
                # 2. I/O: Escreve 1MB no disco
                # Usamos urandom para forçar processamento também
                f.write(os.urandom(CHUNK_SIZE))
                f.flush() # Força o envio para o sistema
                
    except Exception as e:
        print(f"Erro IO: {e}")

try:
    memoria_ocupada = None
    
    while True:
        print(">>> PICO (CPU + I/O + MEM)...")
        
        # 1. MEMÓRIA: Segura 100MB na RAM
        if memoria_ocupada is None:
            memoria_ocupada = bytearray(MEMORIA_MB * 1024 * 1024)
        
        # 2. CPU e I/O: Martela por 3 segundos
        estresse_misto(3)

        print(">>> DESCANSO...")
        
        # Libera a memória
        del memoria_ocupada
        memoria_ocupada = None
        gc.collect()
        
        # Limpa o arquivo do disco
        if os.path.exists(ARQUIVO_TEMP):
            os.remove(ARQUIVO_TEMP)
        
        # Dorme 3 segundos
        time.sleep(3)

except KeyboardInterrupt:
    print("\nEncerrando o caos...")
    if os.path.exists(ARQUIVO_TEMP):
        os.remove(ARQUIVO_TEMP)
