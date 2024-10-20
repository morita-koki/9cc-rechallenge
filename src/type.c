#include "9cc.h"

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

  for (int i = 0; i < node->block_count; i++) {
    visit(node->block[i]);
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
      if (node->rhs->ty->kind == TY_PTR) {  // 右がポインタなら左と入れ替える
        Node *tmp = node->lhs;
        node->lhs = node->rhs;
        node->rhs = tmp;
      }
      if (node->rhs->ty->kind == TY_PTR) {  // 両方がポインタならエラー
        error("invalid pointer arithmetic");
      }
      node->ty = node->lhs->ty;
      return;
    case ND_SUB:
      if (node->rhs->ty->kind == TY_PTR) {  // 右がポインタならエラー
        error("invalid pointer arithmetic");
      }
      node->ty = node->lhs->ty;
      return;
    case ND_ASSIGN:
      node->ty = node->lhs->ty;
      return;
    case ND_ADDR:
      node->ty = pointer_to(node->lhs->ty);
      return;
    case ND_DEREF:
      if (node->lhs->ty->kind != TY_PTR) {
        error("invalid pointer dereference");
      }
      node->ty = node->lhs->ty->ptr_to;
      return;
  }
}