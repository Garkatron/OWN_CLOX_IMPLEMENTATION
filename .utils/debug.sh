#!/bin/bash

# Set source and output directories
SRC_DIR="/home/deus/Documents/Repos/Personal/OWN_CLOX_IMPLEMENTATION/src"
OUT_DIR="/home/deus/Documents/Repos/Personal/OWN_CLOX_IMPLEMENTATION/output"

# Create or clean the output directory
mkdir -p "$OUT_DIR"
rm -f "$OUT_DIR"/*.o "$OUT_DIR"/clox

# Compile all C source files into object files
object_files=()
for src_file in "$SRC_DIR"/*.c; do
    obj_file="$OUT_DIR/$(basename "${src_file%.c}.o")"
    # Compile with debug information and no optimization
    musl-gcc -g -O0 -c "$src_file" -o "$obj_file"

    object_files+=("$obj_file")
done

# Link object files into a single executable (static linking)
musl-gcc -g -O0 -static "${object_files[@]}" -o "$OUT_DIR/clox"

# Run the executable in gdb in interactive mode
gdb -ex "set args /home/deus/Documents/Repos/Personal/OWN_CLOX_IMPLEMENTATION/test/01.lox" \
    -ex "delete breakpoints" \
    -ex "run" \
    "$OUT_DIR/clox"
