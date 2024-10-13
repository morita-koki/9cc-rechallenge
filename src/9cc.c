#include "9cc.h"

char* user_input;
Token* token;
Function* prog;
LVar* locals;
int label_count;
char* argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int main(int argc, char** argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  tokenize();
  prog = program();

  for (Function* fn = prog; fn; fn = fn->next) {
    // set stack size
    int offset = 0;
    for (LVar* lvar = fn->locals; lvar; lvar = lvar->next) {
      offset += 8;
      lvar->var->offset = offset;
    }
    fn->stack_size = offset;
  }
  codegen(prog);

  return 0;
}
