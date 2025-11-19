import time
import os
import math
import gc

# Configurações
MEMORIA_MB = 100    # Aloca 100MB de RAM
ARQUIVO_TEMP = "lixo_temporario.dat"
TAMANHO_IO_MB = 20  # Escreve 20MB no disco

print(f"PID do Caos: {os.getpid()}")
print("Gerando carga em CPU, Memória e I/O (Ciclo de 3s)...")

def estresse_cpu(duracao):
    """Queima CPU fazendo raiz quadrada"""
    fim = time.time() + duracao
    while time.time() < fim:
        math.sqrt(12345.6789) * math.sqrt(98765.4321)

def estresse_io_escrita():
    """Escreve dados aleatórios no disco (Gera I/O Escrita)"""
    try:
        with open(ARQUIVO_TEMP, "wb") as f:
            # Gera dados aleatórios
            dados = os.urandom(TAMANHO_IO_MB * 1024 * 1024)
            f.write(dados)
            f.flush()
            os.fsync(f.fileno()) # Força a gravação física no disco
    except Exception as e:
        print(f"Erro IO: {e}")

def estresse_io_leitura():
    """Lê o arquivo do disco (Gera I/O Leitura)"""
    try:
        if os.path.exists(ARQUIVO_TEMP):
            with open(ARQUIVO_TEMP, "rb") as f:
                _ = f.read() # Lê tudo
            # Apaga o arquivo para limpar
            os.remove(ARQUIVO_TEMP)
    except:
        pass

try:
    memoria_ocupada = None
    
    while True:
        print(">>> PICO DE CARGA (Subindo tudo!)")
        
        # 1. MEMÓRIA: Aloca uma lista gigante de bytes
        memoria_ocupada = bytearray(MEMORIA_MB * 1024 * 1024)
        
        # 2. I/O: Escreve no disco
        estresse_io_escrita()
        
        # 3. CPU: Mantém processador ocupado por 2 segundos
        # (Enquanto segura a memória alocada)
        estresse_cpu(2)
        
        # 4. I/O: Lê o que escreveu
        estresse_io_leitura()

        print(">>> DESCANSO (Limpando...)")
        
        # Limpa a memória
        del memoria_ocupada
        memoria_ocupada = None
        gc.collect() # Força o Python a devolver a RAM para o sistema
        
        # Dorme (CPU cai, I/O para)
        time.sleep(2)

except KeyboardInterrupt:
    print("\nEncerrando o caos...")
    if os.path.exists(ARQUIVO_TEMP):
        os.remove(ARQUIVO_TEMP)