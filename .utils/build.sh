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
    
    # Check if the source file has changed before recompiling
    if [ "$src_file" -nt "$obj_file" ]; then
        musl-gcc -g -O0 -c "$src_file" -o "$obj_file"  # Compilation with debug info
        object_files+=("$obj_file")
    fi
done

# Link object files into a single executable (static linking)
musl-gcc -g -O0 -static "${object_files[@]}" -o "$OUT_DIR/clox"

# Run the executable with the test file
"$OUT_DIR/clox" /home/deus/Documents/Repos/Personal/OWN_CLOX_IMPLEMENTATION/test/01.lox
