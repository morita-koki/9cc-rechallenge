#include "9cc.h"

extern LVar *locals;
extern int label_count;

void gen(Node *node) {
  label_count++;
  int id = label_count;

  switch (node->kind) {
    case ND_FOR:
      //   Aをコンパイルしたコード
      // .LbeginXXX:
      //   Bをコンパイルしたコード
      //   pop rax
      //   cmp rax, 0
      //   je  .LendXXX
      //   Dをコンパイルしたコード
      //   Cをコンパイルしたコード
      //   jmp .LbeginXXX
      // .LendXXX:
      gen(node->lhs->lhs);  // A
      printf(".Lbegin%03d:\n", id);
      gen(node->lhs->rhs);  // B
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%03d\n", id);
      gen(node->rhs->rhs);  // D
      gen(node->rhs->lhs);  // C
      printf("  jmp .Lbegin%03d\n", id);
      printf(".Lend%03d:\n", id);
      return;
    case ND_WHILE:
      //.LbeginXXX:
      // Aをコンパイルしたコード
      // pop rax
      // cmp rax, 0
      // je  .LendXXX
      // Bをコンパイルしたコード
      // jmp .LbeginXXX
      // .LendXXX:
      printf(".Lbegin%03d:\n", id);
      gen(node->lhs);  // A
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%03d\n", id);
      gen(node->rhs);  // B
      printf("  jmp .Lbegin%03d\n", id);
      printf(".Lend%03d:\n", id);
      return;
    case ND_IF:
      // if (A) B else C
      gen(node->lhs);  // A
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lelse%03d\n", id);
      if (node->rhs->kind == ND_ELSE) {  // B
        gen(node->rhs->lhs);
      } else {
        gen(node->rhs);
      }
      printf("  jmp .Lend%03d\n", id);
      printf(".Lelse%03d:\n", id);
      if (node->rhs->kind == ND_ELSE) {
        gen(node->rhs->rhs);  // C
      }
      printf(".Lend%03d:\n", id);
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    error("代入の左辺値が変数ではありません");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}