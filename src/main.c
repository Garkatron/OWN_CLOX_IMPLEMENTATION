#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value_hashmap.h"
#include "value.h"


#define HASHMAP_NAME ValueMap
#define KEY_TYPE Value*
#define VALUE_TYPE Value*
#define HASH_FUNC vhashValue
#define EQUALS_FUNC vequal
#include "hashmap_impl.h"

static void repl()
{
   char line[1024];
   for (;;)
   {
      printf("> ");
      if (!fgets(line, sizeof(line), stdin))
      {
         printf("\n");
         break;
      }
      interpret(line);
   }
}

static char *readFile(const char *path)
{
   printf("Reading file: %s\n", path);
   FILE *file = fopen(path, "rb");
   if (file == NULL)
   {
      fprintf(stderr, "Could not open file \"%s\".\n");
      exit(74);
   }
   fseek(file, 0L, SEEK_END);     // Move to the end of the file.
   size_t fileSize = ftell(file); // get size of the file seeing the end of it.
   rewind(file);                  // back to the start of the file + 1 byte;

   char *buffer = (char *)malloc(fileSize + 1);
   if (buffer == NULL)
   {
      fprintf(stderr, "Not enough memory to read \"%s\".\n");
      exit(74);
   }
   size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

   if (bytesRead < fileSize)
   {
      fprintf(stderr, "Could not read file \"%s\".\n", path);
      exit(74);
   }
   buffer[bytesRead] = '\0'; // Add null terminator.

   fclose(file);
   printf("File read successfully\n");
   return buffer;
}

static void runFile(const char *path)
{
   char *source = readFile(path);
   InterpretResult result = interpret(source);
   free(source);

   if (result == INTERPRET_COMPILE_ERROR)
      exit(65);
   if (result == INTERPRET_RUNTIME_ERROR)
      exit(70);
}

int main(int argc, const char *argv[])
{
   printf("Working...\n");
   initVM();

   if (argc == 1)
   {
      vm.replMode = true;
   }
   else if (argc == 2)
   {
      printf("Running file...\n");
      runFile(argv[1]);
   }
   else
   {
      fprintf(stderr, "Usage: clox [path]\n");
      exit(64);
   }

   freeVM();

   return 0;
}
