# Rutas
SRC_DIR := ./src
OUT_DIR := ./output

# Encuentra todos los .c
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')

# Reemplaza ./src/xx/yy.c -> ./output/xx/yy.o
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/%.o, $(SRC_FILES))

# Compilador
CC := musl-gcc
INC_DIRS := $(shell find $(SRC_DIR) -type d)
CFLAGS := -g -O0 -Wall $(addprefix -I, $(INC_DIRS))
LDFLAGS := -static

TARGET := $(OUT_DIR)/clox

.PHONY: all clean run

all: $(TARGET)

# Ejecutable
$(TARGET): $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

# Compila cada .c en su correspondiente .o
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OUT_DIR)

run: $(TARGET)
	$(TARGET) $(ARGS)
