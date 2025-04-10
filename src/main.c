#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char *argv[])
{
   Chunk chunk;
   initChunk(&chunk);

   int constant = addConstant(&chunk, 1000);  

   writeConstant(&chunk, 1.0, 1);
   writeConstant(&chunk, 1000, 2);

   writeChunk(&chunk, OP_RETURN, 3);

   // Imprimir el Chunk
   disassembleChunk(&chunk, "test chunk");

   freeChunk(&chunk);
   return 0;
}
