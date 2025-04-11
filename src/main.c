#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char *argv[])
{
   initVM();

   Chunk chunk;
   initChunk(&chunk);

   int constant = addConstant(&chunk, 1000);

   writeConstant(&chunk, 1.0, 1);
   // writeConstant(&chunk, 1000, 2);
   writeChunk(&chunk, OP_NEGATE, 3);
   writeChunk(&chunk, OP_RETURN, 4);

   // Imprimir el Chunk
   disassembleChunk(&chunk, "test chunk");
   interpret(&chunk);
   freeVM();
   freeChunk(&chunk);
   return 0;
}
