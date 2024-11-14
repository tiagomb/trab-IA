# Variáveis de compilação
CC = gcc
CFLAGS = -g -I./cvc5/include
LDFLAGS = -L./cvc5/lib -lcvc5 -Wl,-rpath,./cvc5/lib
SRC = src/*.c
OUT_DIR = bin
OUT = $(OUT_DIR)/main

# Regra padrão (compilar o programa)
all: $(OUT)

# Certifique-se de que o diretório bin existe
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Compila o código-fonte
$(OUT): $(SRC) $(OUT_DIR)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

# Limpeza dos arquivos gerados
clean:
	rm -f $(OUT)

# Regra para gerar o executável com dependências de inclusão e bibliotecas
.PHONY: all clean $(OUT_DIR)

