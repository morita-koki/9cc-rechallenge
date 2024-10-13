#include "9cc.h"

extern LVar *locals;
extern int label_count;
extern char *argreg[];

void gen(Node *node) {
  switch (node->kind) {
    case ND_FUNCCALL:
      for (int i = 0; i < node->arg_count; i++) {
        gen(node->args[i]);
      }
      for (int i = node->arg_count - 1; i >= 0; i--) {
        printf("  pop %s\n", argreg[i]);
      }

      // check if RSP is aligned to 16 bytes
      int id = label_count++;
      printf("  mov rax, rsp\n");
      printf("  and rax, 15\n");         // RAX <- RSP % 16
      printf("  jnz .Lcall%03d\n", id);  // if RAX != 0, jump to .LcallXXX
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->funcname);
      printf("  jmp .Lend%03d\n", id);
      printf(".Lcall%03d:\n", id);
      printf("  sub rsp, 8\n");  // align RSP to 16 bytes
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->funcname);
      printf("  add rsp, 8\n");  // restore RSP
      printf(".Lend%03d:\n", id);
      printf("  push rax\n");
      return;
    case ND_BLOCK:
      for (int i = 0; i < node->block_count; i++) {
        gen(node->block[i]);
        printf("  pop rax\n");
      }
      return;
    case ND_FOR: {
      int id = label_count++;
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
      if (node->lhs->lhs) {
        gen(node->lhs->lhs);  // A
      }
      printf(".Lbegin%03d:\n", id);
      if (node->lhs->rhs) {
        gen(node->lhs->rhs);  // B
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", id);
      }
      gen(node->rhs->rhs);  // D
      if (node->rhs->lhs) {
        gen(node->rhs->lhs);  // C
      }
      printf("  jmp .Lbegin%03d\n", id);
      printf(".Lend%03d:\n", id);
      return;
    }
    case ND_WHILE: {
      int id = label_count++;
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
    }
    case ND_IF: {
      int id = label_count++;
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
    }
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