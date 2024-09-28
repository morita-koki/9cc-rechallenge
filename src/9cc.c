#include "9cc.h"

char* user_input;
Token* token;

int main(int argc, char** argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  user_input = argv[1];
  token = tokenize();   // tokenize
  Node* node = expr();  // parse

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  gen(node);  // generate

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
