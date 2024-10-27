#include "9cc.h"

/* Global variables */
char* user_input;
Token* token;
// Program* prog;
LVar* locals;
LVar* globals;
int label_count;
char* argreg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char* argreg4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char* argreg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

char* filename;
char* read_file(char* path);

int main(int argc, char** argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  filename = argv[1];
  user_input = read_file(filename);
  tokenize();
  Program* prog = program();
  add_type(prog);

  for (Function* fn = prog->funcs; fn; fn = fn->next) {
    // set stack size
    int offset = 0;
    for (LVar* lvar = fn->locals; lvar; lvar = lvar->next) {
      size_t size = size_of(lvar->var->ty);
      offset += size;
      lvar->var->offset = offset;
    }
    if (offset % 16 != 0) {
      offset += 16 - offset % 16;
    }
    fn->stack_size = offset;
  }
  codegen(prog);

  return 0;
}

// Returns the contents of a given file.
char* read_file(char* path) {
  // Open and read the file.
  FILE* fp = fopen(path, "r");
  if (!fp) error("cannot open %s: %s", path, strerror(errno));
  int filemax = 10 * 1024 * 1024;
  char* buf = malloc(filemax);
  int size = fread(buf, 1, filemax - 2, fp);
  if (!feof(fp)) error("%s: file too large");
  // Make sure that the string ends with "\n\0".
  if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
  buf[size] = '\0';
  return buf;
}