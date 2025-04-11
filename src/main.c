#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[])
{
   initVM();

   if (argc == 1) {

   } else if (argc == 2) {
      runFile(argv[1]);
   } else {
      fprintf(stderr, "Usage: clox [path]\n");
      exit(64);
   }

   freeVM();
   return 0;
}

static void repl() {
   char line[1024];
   for (;;)
   {
      printf("> ");
      if(!fgets(line, sizeof(line), stdin)) {
         printf("\n");
         break;
      }
      interpret(line);
   }
   
}

static void runFile(const char* path) {
   char* source = readFile(path);
   InterpretResult result = interpret(source);
   free(source);

   if(result == INTERPRET_COMPILE_ERROR) exit(65);
   if(result == INTERPRET_RUNTIME_ERROR) exit(70);
}

static char* readFile(const char* path) {
   FILE* file = fopen(path, "rb");
   if (file == NULL) {
      fprintf(stderr, "Could not open file \"%s\".\n");
      exit(74);
   }
   fseek(file, 0L, SEEK_END); // Move to the end of the file.
   size_t fileSize = ftell(file); // get size of the file seeing the end of it.
   rewind(file); // back to the start of the file + 1 byte;

   char* buffer = (char*)malloc(fileSize + 1);
   if (buffer = NULL) {
      fprintf(stderr, "Not enough memory to read \"%s\".\n");
      exit(74);
   }
   size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
   if (bytesRead < fileSize) {
      fprintf(stderr, "Could not read file \"%s\".\n", path);
      exit(74);
   }
   buffer[bytesRead] = '\0'; // Not null terminartor.

   fclose(file);
   return buffer;
}