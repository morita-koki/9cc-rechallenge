#include "9cc.h"

Type *char_type() {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_CHAR;
  return ty;
}

Type *int_type() {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_INT;
  return ty;
}

Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->ptr_to = base;
  return ty;
}

Type *array_of(Type *base, size_t size) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->ptr_to = base;
  ty->array_size = size;
  return ty;
}

size_t size_of(Type *ty) {
  switch (ty->kind) {
    case TY_CHAR:
      return 1;
    case TY_INT:
      return 4;
    case TY_PTR:
      return 8;
    case TY_ARRAY:
      return size_of(ty->ptr_to) * ty->array_size;
    default:
      assert(ty->kind == TY_STRUCT);
      Member *mem = ty->members;
      while (mem->next) mem = mem->next;
      return mem->offset + size_of(mem->ty);
  }

  error("unknown type");
  return 0;
}

Member *find_member(Type *ty, char *name) {
  assert(ty->kind == TY_STRUCT);
  for (Member *mem = ty->members; mem; mem = mem->next)
    if (!strcmp(mem->name, name)) return mem;
  return NULL;
}

void visit(Node *node) {
  if (!node) {
    return;
  }
  visit(node->lhs);
  visit(node->rhs);
  visit(node->cond);
  visit(node->then);
  visit(node->els);
  visit(node->init);
  visit(node->inc);

  for (Node *n = node->block; n; n = n->next) {
    visit(n);
  }
  for (int i = 0; i < node->arg_count; i++) {
    visit(node->args[i]);
  }

  switch (node->kind) {
    case ND_MUL:
    case ND_DIV:
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_FUNCCALL:
    case ND_NUM:
      node->ty = int_type();
      return;
    case ND_VAR:
      node->ty = node->var->ty;
      return;

    case ND_ADD:
      if (node->rhs->ty->ptr_to) {  // 右がポインタなら左と入れ替える
        Node *tmp = node->lhs;
        node->lhs = node->rhs;
        node->rhs = tmp;
      }
      if (node->rhs->ty->ptr_to) {  // 両方がポインタならエラー
        error("invalid pointer arithmetic");
      }
      node->ty = node->lhs->ty;
      return;
    case ND_SUB:
      if (node->rhs->ty->ptr_to) {  // 右がポインタならエラー
        error("invalid pointer arithmetic");
      }
      node->ty = node->lhs->ty;
      return;
    case ND_ASSIGN:
      node->ty = node->lhs->ty;
      return;
    case ND_ADDR:
      if (node->lhs->ty->kind == TY_ARRAY) {
        node->ty = pointer_to(node->lhs->ty->ptr_to);
      } else {
        node->ty = pointer_to(node->lhs->ty);
      }
      return;
    case ND_MEMBER: {
      if (node->lhs->ty->kind != TY_STRUCT) error("not a struct");
      node->member = find_member(node->lhs->ty, node->member_name);
      if (!node->member) error("specified member does not exist");
      node->ty = node->member->ty;
      return;
    }
    case ND_DEREF:
      if (!node->lhs->ty->ptr_to) {
        error("invalid pointer dereference");
      }
      node->ty = node->lhs->ty->ptr_to;
      return;
  }
}

void add_type(Program *prog) {
  for (Function *fn = prog->funcs; fn; fn = fn->next)
    for (Node *node = fn->node; node; node = node->next) {
      visit(node);
    }
}