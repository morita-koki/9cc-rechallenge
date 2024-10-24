#include "9cc.h"

extern Token *token;
extern LVar *locals;

/* find variable from locals
 * if found, return Var
 * if not found, return NULL
 */
Var *find_var(Token *tok) {
  for (LVar *lvar = locals; lvar; lvar = lvar->next) {
    if (strlen(lvar->var->name) == tok->len &&
        !memcmp(tok->str, lvar->var->name, tok->len)) {
      return lvar->var;
    }
  }
  return NULL;
}

/* push variable to locals
 * return added variable
 * must check if variable is already defined
 *    before calling this function
 */
Var *push_var(char *name, Type *ty) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->ty = ty;

  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->var = var;
  lvar->next = locals;
  locals = lvar;
  return var;
}

Token *peek(char *s) {
  if (token->kind != TK_RESERVED || strlen(s) != token->len ||
      memcmp(token->str, s, token->len)) {
    return NULL;
  }
  return token;
}

bool consume(char *s) {
  if (!peek(s)) {
    return false;
  }
  token = token->next;
  return true;
}

Token *consume_kind(TokenKind kind) {
  if (token->kind != kind) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

void expect(char *s) {
  if (!peek(s)) {
    error_at(token->str, "expected \"%s\"", s);
  }
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

char *expect_ident() {
  if (token->kind != TK_IDENT) {
    error_at(token->str, "expected an identifier");
  }
  char *ident_name = malloc(token->len + 1);
  memcpy(ident_name, token->str, token->len);
  ident_name[token->len] = '\0';
  token = token->next;
  return ident_name;
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
  program    = function*
  function   = type ident "(" params? ")" "{" stmt* "}"
  params     = param ("," param)*
  param      = "int" ident
  stmt       = expr ";"
              | "{" stmt* "}"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "return" expr ";"
              | declaration
  declaration = "int" ident ("=" expr)? ";"
  expr       = assign
  assign     = equality ("=" assign)?
  equality   = relational ("==" relational | "!=" relational)*
  relational = add ("<" add | "<=" add | ">" add | ">=" add)*
  add        = mul ("+" mul | "-" mul)*
  mul        = unary ("*" unary | "/" unary)*
  unary      = "sizeof" unary
               | ("+" | "-" | "&" | "*")? unary
               | primary
  primary    = num | ident ("(" "")")? | "(" expr ")"
*/

Function *program() {
  Function head;
  head.next = NULL;
  Function *cur = &head;

  while (!at_eof()) {
    cur->next = function();
    cur = cur->next;
  }
  return head.next;
}

// args       = arg ("," arg)*
// arg        = "int" ident
LVar *read_func_args() {
  // func(arg0, arg1, arg2)
  // locals: ...local1 -> local0 -> arg2 -> arg1 -> arg0
  // args: arg0 -> arg1 -> arg2...

  if (consume(")")) {
    return NULL;
  }

  LVar head;
  head.next = NULL;
  LVar *cur = &head;

  while (!consume(")")) {
    cur->next = calloc(1, sizeof(LVar));
    Type *ty = read_type();
    char *name = expect_ident();
    cur->next->var = push_var(name, ty);
    cur = cur->next;
    if (!consume(",")) {
      expect(")");
      break;
    }
  }
  return head.next;
}

Type *read_type() {
  if (!consume("int")) {
    error_at(token->str, "expected type");
  }
  Type *ty = int_type();
  while (consume("*")) {
    ty = pointer_to(ty);
  }
  return ty;
}

// type       = "int" "*"*
// function   = "int" ident "(" args? ")" "{" stmt* "}"
// args       = arg ("," arg)*
// arg        = type ident
Function *function() {
  locals = NULL;
  Function *fn = calloc(1, sizeof(Function));

  read_type();  // function return type, now only int
  fn->name = expect_ident();
  expect("(");
  fn->args = read_func_args();
  expect("{");

  Node head;
  head.next = NULL;
  Node *cur = &head;
  while (!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }
  fn->node = head.next;
  fn->locals = locals;
  return fn;
}

Node *stmt() {
  if (consume_kind(TK_RETURN)) {
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

  if (consume_kind(TK_IF)) {
    // if (cond) { then } else { els }
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume_kind(TK_ELSE)) {
      node->els = stmt();
    }
    return node;
  }

  if (consume_kind(TK_WHILE)) {
    // while (cond) { then }
    Node *node = new_node(ND_WHILE, NULL, NULL);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  }

  if (consume_kind(TK_FOR)) {
    // for (init; cond; inc) { then }
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if (!consume(";")) {
      node->init = new_node(ND_EXPR_STMT, expr(), NULL);
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = new_node(ND_EXPR_STMT, expr(), NULL);
      expect(")");
    }
    node->then = stmt();

    return node;
  }

  if (peek("int")) {  // only check if next token is "int"
    return declaration();
  }

  Node *node = new_node(ND_EXPR_STMT, expr(), NULL);
  expect(";");
  return node;
}

// declaration = "int" ident ("[" num "]")* ("=" expr)? ";"
Node *declaration() {
  Type *ty = read_type();
  char *name = expect_ident();

  while (consume("[")) {
    int array_size = expect_number();
    expect("]");
    ty = array_of(ty, array_size);
  }

  if (consume(";")) {
    Var *var = push_var(name, ty);
    return new_node(ND_NULL, NULL, NULL);
  }

  expect("=");
  Node *node_var = new_node(ND_VAR, NULL, NULL);
  node_var->var = push_var(name, ty);
  Node *node = new_node(ND_ASSIGN, node_var, expr());
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
    visit(node);
    // fprintf(stderr, "node->ty->kind: %d\n", node->ty->kind);
    if (consume("+")) {
      Node *r = mul();
      if (node->ty->kind == TY_ARRAY) {
        node->ty = pointer_to(node->var->ty->ptr_to);
      }
      if (node->ty->kind == TY_PTR) {
        int n = size_of(node->ty->ptr_to);
        r = new_node(ND_MUL, r, new_node_num(n));
      }
      node = new_node(ND_ADD, node, r);
    } else if (consume("-")) {
      Node *r = mul();
      if (node->ty->kind == TY_ARRAY) {
        node->ty = pointer_to(node->var->ty->ptr_to);
      }
      if (node->ty->kind == TY_PTR) {
        int n = size_of(node->ty->ptr_to);
        r = new_node(ND_MUL, r, new_node_num(n));
      }
      node = new_node(ND_SUB, node, r);
    } else
      return node;
  }
  // visit(node);

  // for (;;) {
  //   if (consume("+")) {
  //     Node *r = mul();
  //     if (node->kind == ND_VAR && node->ty->kind == TY_PTR) {
  //       // int n = node->var->ty->ptr_to->kind == TY_INT ? 4 : 8;
  //       int n = size_of(node->ty->ptr_to);
  //       r = new_node(ND_MUL, r, new_node_num(n));
  //     }
  //     node = new_node(ND_ADD, node, r);
  //   } else if (consume("-")) {
  //     Node *r = mul();
  //     if (node->kind == ND_VAR && node->ty->kind == TY_PTR) {
  //       // int n = node->var->ty->ptr_to->kind == TY_INT ? 4 : 8;
  //       int n = size_of(node->ty->ptr_to);
  //       r = new_node(ND_MUL, r, new_node_num(n));
  //     }
  //     node = new_node(ND_SUB, node, r);
  //   } else {
  //     return node;
  //   }
  // }
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
  if (consume("sizeof")) {
    Node *node = unary();
    // nodeのtypeを知るためには、nodeを再起的に評価する必要がある
    // chibiccではvisit, add_typeで実装されている。
    visit(node);
    return new_node_num(size_of(node->ty));
  }
  if (consume("+")) return unary();
  if (consume("-")) return new_node(ND_SUB, new_node_num(0), unary());
  if (consume("&")) return new_node(ND_ADDR, unary(), NULL);
  if (consume("*")) return new_node(ND_DEREF, unary(), NULL);
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
    Node *node = new_node(ND_VAR, NULL, NULL);
    Var *var = find_var(tok);
    if (var) {
      node->var = var;
    } else {
      error("undefined variable. %s", tok->str);
    }
    return node;
  }

  int val = expect_number();
  return new_node_num(val);
}
