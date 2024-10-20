#include "9cc.h"

extern LVar *locals;
extern int label_count;
extern char *argreg[];

char *funcname;

void codegen(Function *prog) {
  printf(".intel_syntax noprefix\n");

  for (Function *func = prog; func; func = func->next) {
    printf(".globl %s\n", func->name);
    printf("%s:\n", func->name);
    funcname = func->name;

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", func->stack_size);

    int i = 0;
    for (LVar *lvar = func->args; lvar; lvar = lvar->next) {
      printf("  mov [rbp-%d], %s\n", lvar->var->offset, argreg[i++]);
    }

    for (Node *node = func->node; node; node = node->next) {
      gen(node);
    }

    // epilogue
    printf(".Lreturn.%s:\n", func->name);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}

void gen(Node *node) {
  switch (node->kind) {
    case ND_NULL:
      return;
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
      if (node->init) {
        gen(node->init);  // A
      }
      printf(".Lbegin%03d:\n", id);
      if (node->cond) {
        gen(node->cond);  // B
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", id);
      }
      gen(node->then);  // D
      if (node->inc) {
        gen(node->inc);  // C
      }
      printf("  jmp .Lbegin%03d\n", id);
      printf(".Lend%03d:\n", id);
      return;
    }
    case ND_WHILE: {
      int id = label_count++;
      printf(".Lbegin%03d:\n", id);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%03d\n", id);
      gen(node->then);
      printf("  jmp .Lbegin%03d\n", id);
      printf(".Lend%03d:\n", id);
      return;
    }
    case ND_IF: {
      int id = label_count++;
      if (node->els) {
        gen(node->cond);  // A
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lelse%03d\n", id);
        gen(node->then);
        printf("  jmp .Lend%03d\n", id);
        printf(".Lelse%03d:\n", id);
        gen(node->els);
        printf(".Lend%03d:\n", id);
      } else {
        gen(node->cond);  // A
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", id);
        gen(node->then);
        printf(".Lend%03d:\n", id);
      }
      return;
    }
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  jmp .Lreturn.%s\n", funcname);
      return;
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_VAR:
      gen_addr(node);
      load();
      return;
    case ND_ASSIGN:
      gen_addr(node->lhs);
      gen(node->rhs);
      store();
      return;
    case ND_ADDR:
      gen_addr(node->lhs);
      return;
    case ND_DEREF:
      gen(node->lhs);
      load();
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

void gen_addr(Node *node) {
  switch (node->kind) {
    case ND_VAR:
      printf("  lea rax, [rbp-%d]\n", node->var->offset);
      printf("  push rax\n");
      return;
    case ND_DEREF:
      gen(node->lhs);
      return;
  }

  error("not a lvalue");
}

void load() {
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

void store() {
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
}