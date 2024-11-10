#include "9cc.h"

extern LVar *locals;
extern int label_count;
extern char *argreg4[];
extern char *argreg8[];
extern char *argreg1[];

char *funcname;

void emit_data(Program *prog) {
  printf(".data\n");
  for (LVar *lvar = prog->globals; lvar; lvar = lvar->next) {
    printf("%s:\n", lvar->var->name);

    if (lvar->var->contents) {
      printf("  .string \"%.*s\"\n", lvar->var->contents_len,
             lvar->var->contents);
      // for (int i = 0; i < lvar->var->contents_len; i++)
      //   printf("  .byte %d\n", lvar->var->contents[i]);
      continue;
    } else if (!lvar->var->initializer) {
      printf("  .zero %ld\n", size_of(lvar->var->ty));
      continue;
    }

    for (Initializer *init = lvar->var->initializer; init; init = init->next) {
      if (init->label) {
        printf("  .quad %s\n", init->label);
        continue;
      }

      if (init->size == 1) {
        printf("  .byte %ld\n", init->val);
      } else {
        printf("  .%dbyte %ld\n", init->size, init->val);
      }
    }
  }
}

void emit_text(Program *prog) {
  printf(".text\n");

  for (Function *fn = prog->funcs; fn; fn = fn->next) {
    printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);
    funcname = fn->name;

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);

    // push arguments to the stack
    int i = 0;
    for (LVar *lvar = fn->args; lvar; lvar = lvar->next) {
      int size = size_of(lvar->var->ty);
      switch (size) {
        case 1:
          printf("  mov [rbp-%d], %s\n", lvar->var->offset, argreg1[i]);
          break;
        case 4:
          printf("  mov [rbp-%d], %s\n", lvar->var->offset, argreg4[i]);
          break;
        case 8:
          printf("  mov [rbp-%d], %s\n", lvar->var->offset, argreg8[i]);
          break;
        default:
          error("unknown data size");
          break;
      }
      i++;
    }

    for (Node *node = fn->node; node; node = node->next) {
      gen(node);
    }

    // epilogue
    printf(".Lreturn.%s:\n", fn->name);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}

void codegen(Program *prog) {
  printf(".intel_syntax noprefix\n");
  emit_data(prog);
  emit_text(prog);
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
        printf("  pop %s\n", argreg8[i]);
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
      for (Node *n = node->block; n; n = n->next) {
        gen(n);
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
    case ND_EXPR_STMT:
      gen(node->lhs);
      printf("  add rsp, 8\n");
      // or pop the result
      // printf("  pop rax\n");
      return;
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
      if (node->ty->kind != TY_ARRAY) {
        load(node->ty);
      }
      return;
    case ND_ASSIGN:
      gen_addr(node->lhs);
      gen(node->rhs);
      store(node->ty);
      return;
    case ND_ADDR:
      gen_addr(node->lhs);
      return;
    case ND_DEREF:
      gen(node->lhs);
      if (node->ty->kind != TY_ARRAY) {
        load(node->ty);
      }
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      if (node->ty->kind == TY_PTR || node->ty->kind == TY_ARRAY) {
        int size = size_of(node->ty->ptr_to);
        printf("  imul rdi, %d\n", size);
      }
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      if (node->ty->kind == TY_PTR || node->ty->kind == TY_ARRAY) {
        int size = size_of(node->ty->ptr_to);
        printf("  imul rdi, %d\n", size);
      }
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
    case ND_VAR: {
      if (node->var->is_local) {
        printf("  lea rax, [rbp-%d]\n", node->var->offset);
        printf("  push rax\n");
      } else {
        printf("  lea rax, %s[rip]\n", node->var->name);
        printf("  push rax\n");
        // printf("  push offset %s\n", node->var->name);
      }
      return;
    }
    case ND_DEREF:
      gen(node->lhs);
      return;
  }

  error("not a lvalue");
}

void load(Type *ty) {
  printf("  pop rax\n");

  int size = size_of(ty);
  switch (size) {
    case 1:
      printf("  movsx rax, byte ptr [rax]\n");
      break;
    case 4:
      printf("  movsxd rax, dword ptr [rax]\n");
      break;
    case 8:
      printf("  mov rax, [rax]\n");
      break;
    default:
      error("unknown data size");
      break;
  }
  printf("  push rax\n");
}

void store(Type *ty) {
  printf("  pop rdi\n");
  printf("  pop rax\n");

  int size = size_of(ty);
  switch (size) {
    case 1:
      printf("  mov [rax], dil\n");
      break;
    case 4:
      printf("  mov [rax], edi\n");
      break;
    case 8:
      printf("  mov [rax], rdi\n");
      break;
    default:
      error("unknown data size");
      break;
  }
  printf("  push rdi\n");
}