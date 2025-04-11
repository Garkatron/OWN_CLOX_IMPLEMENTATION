#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char *argv[])
{
   initVM();

   Chunk chunk;
   initChunk(&chunk);

   // 3 + 2 * 1

   // 3
   writeConstant(&chunk, 3, 0);

   // 2
   writeConstant(&chunk, 2, 0);

   // 1
   writeConstant(&chunk, 1, 0);

   // *
   writeChunk(&chunk, OP_MULTIPLY, 0);

   // +
   writeChunk(&chunk, OP_ADD, 0);

   // return
   writeChunk(&chunk, OP_RETURN, 0);

   // Imprimir el Chunk
   disassembleChunk(&chunk, "test chunk");
   interpret(&chunk);
   freeVM();
   freeChunk(&chunk);
   return 0;
}
