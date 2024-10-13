#include "9cc.h"

extern Token *token;
extern LVar *locals;
extern Node *code[100];

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_return() {
  if (token->kind != TK_RETURN) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_if() {
  if (token->kind != TK_IF) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_else() {
  if (token->kind != TK_ELSE) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_while() {
  if (token->kind != TK_WHILE) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_for() {
  if (token->kind != TK_FOR) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected \"%s\"", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "expected a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

/* BNF (Backus-Naur Form)
  program    = stmt*
  stmt       = expr ";"
              | "{" stmt* "}"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | ...
  expr       = assign
  assign     = equality ("=" assign)?
  equality   = relational ("==" relational | "!=" relational)*
  relational = add ("<" add | "<=" add | ">" add | ">=" add)*
  add        = mul ("+" mul | "-" mul)*
  mul        = unary ("*" unary | "/" unary)*
  unary      = ("+" | "-")? unary | primary
  primary    = num | ident ("(" "")")? | "(" expr ")"
*/

void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node *stmt() {
  if (consume_return()) {
    Node *node = new_node(ND_RETURN, expr(), NULL);
    expect(";");
    return node;
  }

  if (consume("{")) {
    Node *node = new_node(ND_BLOCK, NULL, NULL);
    int stmt_count = 0;
    while (!consume("}")) {
      node->block = realloc(node->block, sizeof(Node *) * (stmt_count + 1));
      node->block[stmt_count] = stmt();
      stmt_count += 1;
    }
    node->block_count = stmt_count;
    return node;
  }

  if (consume_if()) {
    Node *node = new_node(ND_IF, NULL, NULL);
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    // if (A) B
    //    node->lhs = A, node->rhs = B
    // if (A) B else C
    //    node->lhs = A, node->rhs->lhs = B, node->rhs->rhs = C
    if (consume_else()) {
      Node *else_node = new_node(ND_ELSE, NULL, NULL);
      else_node->lhs = node->rhs;
      else_node->rhs = stmt();
      node->rhs = else_node;
    }
    return node;
  }

  if (consume_while()) {
    Node *node = new_node(ND_WHILE, NULL, NULL);
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    return node;
  }

  if (consume_for()) {
    // for (A; B; C) D
    Node *for_node_left = new_node(ND_FOR_LEFT, NULL, NULL);
    Node *for_node_right = new_node(ND_FOR_RIGHT, NULL, NULL);
    expect("(");
    if (!consume(";")) {
      for_node_left->lhs = expr();  // A
      expect(";");
    }
    if (!consume(";")) {
      for_node_left->rhs = expr();  // B
      expect(";");
    }
    if (!consume(")")) {
      for_node_right->lhs = expr();  // C
      expect(")");
    }
    for_node_right->rhs = stmt();  // D

    Node *node = new_node(ND_FOR, for_node_left, for_node_right);
    return node;
  }

  Node *node = expr();
  expect(";");
  return node;
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = equality();
  if (consume("=")) node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume("+")) return unary();
  if (consume("-")) return new_node(ND_SUB, new_node_num(0), unary());
  return primary();
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    // ANCHOR: function call
    if (consume("(")) {
      Node *node = new_node(ND_FUNCCALL, NULL, NULL);
      node->funcname = malloc(tok->len + 1);
      memcpy(node->funcname, tok->str, tok->len);

      // function arguments
      int arg_count = 0;
      while (!consume(")")) {
        node->args = realloc(node->args, sizeof(Node *) * (arg_count + 1));
        node->args[arg_count] = expr();
        arg_count += 1;
        if (!consume(",")) {
          expect(")");
          break;
        }
      }
      node->arg_count = arg_count;
      return node;
    }

    // ANCHOR: local variable
    Node *node = new_node(ND_LVAR, NULL, NULL);
    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      // if new variable, add to locals
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if (locals) {
        lvar->offset = locals->offset + 8;
      } else {
        lvar->offset = 8;
      }
      locals = lvar;
      node->offset = lvar->offset;
    }
    return node;
  }

  int val = expect_number();
  return new_node_num(val);
}
