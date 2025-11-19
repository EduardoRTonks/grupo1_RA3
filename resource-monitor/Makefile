# Compilador
CC = gcc

# Flags de Compilação - Obrigatório compilar sem warnings!
# CFLAGS inclui -I para achar os headers em 'include/'
CFLAGS = -Wall -Wextra -Wno-format-truncation -g -Iinclude

# Nome do executável final
TARGET = resource_monitor

# Lista de arquivos-fonte (.c)
# Encontra automaticamente todos os .c dentro de 'src/'
SRCS = $(wildcard src/*.c)

# Lista de arquivos-objeto (.o)
# Substitui 'src/%.c' por 'build/%.o'
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))

# --- Regras ---

# Regra principal: compilar o programa
all: $(TARGET)

# Regra para linkar o executável final
$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/$(TARGET) $(OBJS)
	@echo "Executável [bin/$(TARGET)] criado com sucesso!"

# Regra para compilar arquivos .c em .o
# Cria o diretório 'build' se não existir
build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# --- Alvos de Teste (Aluno 2) ---

# Alvo para compilar o teste de CPU
test_cpu: tests/test_cpu.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/test_cpu tests/test_cpu.c
	@echo "Teste [bin/test_cpu] compilado."

# Alvo para compilar o teste de Memória
test_memory: tests/test_memory.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/test_memory tests/test_memory.c
	@echo "Teste [bin/test_memory] compilado."

# Alvo para compilar o teste de I/O
test_io: tests/test_io.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/test_io tests/test_io.c
	@echo "Teste [bin/test_io] compilado."

# Alvo para compilar todos os testes de uma vez
all_tests: test_cpu test_memory test_io
	@echo "Todos os programas de teste compilados."
# -------------------------------------

# Regra para limpar os arquivos compilados
clean:
	rm -rf build bin
	@echo "Arquivos compilados removidos."

# Regra para forçar a recompilação
re: clean all

.PHONY: all clean re test_cpu test_memory test_io all_tests