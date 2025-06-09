# Directorios de origen y salida
SRC_DIR := ./src
OUT_DIR := ./output

# Archivos fuente
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(SRC_FILES:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)

# Nombre del ejecutable final
TARGET := $(OUT_DIR)/clox

# Compilador y flags
CC := musl-gcc
CFLAGS := -g -O0 -Wall
LDFLAGS := -static

# Reglas
.PHONY: all clean

# Regla principal (por defecto)
all: $(TARGET)

# Regla para crear el ejecutable
$(TARGET): $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

# Regla para compilar los archivos fuente en archivos objeto
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar archivos generados
clean:
	rm -rf $(OUT_DIR)

# Regla para ejecutar el programa
run: $(TARGET)
	$(TARGET) $(ARGS)

