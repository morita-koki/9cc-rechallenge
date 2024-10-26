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

int main(int argc, char** argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
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
