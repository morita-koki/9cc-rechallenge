#include "9cc.h"

extern Token *token;
extern LVar *locals;
extern LVar *globals;

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
  for (LVar *lvar = globals; lvar; lvar = lvar->next) {
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
Var *push_var(char *name, Type *ty, bool is_local) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->ty = ty;
  var->is_local = is_local;

  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->var = var;

  if (is_local) {
    lvar->next = locals;
    locals = lvar;
  } else {
    lvar->next = globals;
    globals = lvar;
  }
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

bool is_function() {
  Token *tok = token;
  read_type();
  bool isfunc = consume_ident() && consume("(");
  token = tok;
  return isfunc;
}

Initializer *new_init_val(Initializer *cur, int size, int val) {
  Initializer *init = calloc(1, sizeof(Initializer));
  init->size = size;
  init->val = val;
  cur->next = init;
  cur = cur->next;
  return cur;
}

Initializer *new_init_zero(Initializer *cur, int size) {
  for (int i = 0; i < size; i++) {
    cur = new_init_val(cur, 1, 0);
  }
  return cur;
}

Initializer *new_init_label(Initializer *cur, char *label) {
  Initializer *init = calloc(1, sizeof(Initializer));
  init->label = label;
  cur->next = init;
  cur = cur->next;
  return cur;
}

Initializer *gvar_init_string(char *p, int len) {
  Initializer head;
  head.next = NULL;
  Initializer *cur = &head;

  for (int i = 0; i < len; i++) {
    cur = new_init_val(cur, 1, p[i]);
  }
  return head.next;
}

long eval(Node *node) {
  switch (node->kind) {
    case ND_ADD:
      return eval(node->lhs) + eval(node->rhs);
    case ND_SUB:
      return eval(node->lhs) - eval(node->rhs);
    case ND_MUL:
      return eval(node->lhs) * eval(node->rhs);
    case ND_DIV:
      return eval(node->lhs) / eval(node->rhs);
    case ND_EQ:
      return eval(node->lhs) == eval(node->rhs);
    case ND_NE:
      return eval(node->lhs) != eval(node->rhs);
    case ND_LT:
      return eval(node->lhs) < eval(node->rhs);
    case ND_LE:
      return eval(node->lhs) <= eval(node->rhs);
    case ND_NUM:
      return node->val;
  }
  error_at(token->str, "not a constant expression");
}

long const_expr() { return eval(equality()); }

Initializer *gvar_initializer(Initializer *cur, Type *ty) {
  Token *tok = token;

  // array initialization: int x[2] = {1, 2};
  if (consume("{")) {
    if (ty->kind == TY_ARRAY) {
      int i = 0;
      do {
        cur = gvar_initializer(cur, ty->ptr_to);
        i++;
      } while (!initializer_end() && consume(","));
      expect_initializer_end();
      if (i < ty->array_size)
        cur = new_init_zero(cur, size_of(ty->ptr_to) * (ty->array_size - i));
      if (ty->is_incomplete) {
        ty->array_size = i;
        ty->is_incomplete = false;
      }
      return cur;
    }
  }

  // simple initializer: int x = 3;
  Node *expr = equality();  // conditional

  if (expr->kind == ND_ADDR) {
    if (expr->lhs->kind != ND_VAR) {
      error_at(tok->str, "invalid initializer");
    }
    return new_init_label(cur, expr->lhs->var->name);
  }

  if (expr->kind == ND_VAR && expr->var->ty->kind == TY_ARRAY) {
    return new_init_label(cur, expr->var->name);
  }

  return new_init_val(cur, size_of(ty), eval(expr));
}

Type *type_suffix(Type *ty) {
  if (!consume("[")) return ty;

  int sz = 0;
  bool is_incomplete = true;
  if (!consume("]")) {
    sz = const_expr();
    is_incomplete = false;
    expect("]");
  }

  ty = type_suffix(ty);
  ty = array_of(ty, sz);
  ty->is_incomplete = is_incomplete;
  return ty;
}

void global_declaration() {
  Type *ty = read_type();
  char *name = expect_ident();
  ty = type_suffix(ty);

  // while (consume("[")) {
  //   int array_size = expect_number();
  //   expect("]");
  //   ty = array_of(ty, array_size);
  // }

  // check end of declaration
  if (consume(";")) {
    push_var(name, ty, false);
    return;
  }

  // global variable with initializer
  expect("=");
  Var *var = push_var(name, ty, false);
  Initializer head;
  head.next = NULL;
  gvar_initializer(&head, ty);
  var->initializer = head.next;
  expect(";");
}

char *new_label() {
  static int count = 0;
  char buf[20];
  sprintf(buf, ".L.data.%d", count++);
  return strndup(buf, strlen(buf));
}

/* BNF (Backus-Naur Form)
  program    = function*
  function   = type ident "(" params? ")" "{" stmt* "}"
  params     = param ("," param)*
  param      = type ident
  stmt       = expr ";"
              | "{" stmt* "}"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "return" expr ";"
              | declaration
  declaration = type ident ("=" expr)? ";"
  expr       = assign
  assign     = equality ("=" assign)?
  equality   = relational ("==" relational | "!=" relational)*
  relational = add ("<" add | "<=" add | ">" add | ">=" add)*
  add        = mul ("+" mul | "-" mul)*
  mul        = unary ("*" unary | "/" unary)*
  unary      = "sizeof" unary
               | ("+" | "-" | "&" | "*")? unary
               | primary
  primary    = num
               | ident ("(" "")")?
               | ident "[" expr "]"
               | "(" expr ")"
*/

Program *program() {
  Function head;
  head.next = NULL;
  Function *cur = &head;
  globals = NULL;

  while (!at_eof()) {
    if (is_function()) {
      cur->next = function();
      cur = cur->next;
    } else {
      global_declaration();
    }
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->funcs = head.next;
  prog->globals = globals;
  return prog;
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
    cur->next->var = push_var(name, ty, true);
    cur = cur->next;
    if (!consume(",")) {
      expect(")");
      break;
    }
  }
  return head.next;
}

Type *read_type() {
  Type *ty;
  if (!is_typename(token)) error_at(token->str, "expected type");
  if (consume("int"))
    ty = int_type();
  else if (consume("char"))
    ty = char_type();
  else
    ty = struct_decl();

  while (consume("*")) {
    ty = pointer_to(ty);
  }
  return ty;
}

Type *struct_decl() {
  expect("struct");
  expect("{");

  Member head;
  head.next = NULL;
  Member *cur = &head;

  while (!consume("}")) {
    cur->next = struct_member();
    cur = cur->next;
  }

  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_STRUCT;
  ty->members = head.next;

  // Assign offsets within the struct to members.
  int offset = 0;
  for (Member *mem = ty->members; mem; mem = mem->next) {
    mem->offset = offset;
    offset += size_of(mem->ty);
  }

  return ty;
}

Member *struct_member() {
  Member *mem = calloc(1, sizeof(Member));
  mem->ty = read_type();
  mem->name = expect_ident();
  mem->ty = type_suffix(mem->ty);
  expect(";");
  return mem;
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

bool is_typename() { return peek("char") || peek("int") || peek("struct"); }

Node *stmt() {
  if (consume_kind(TK_RETURN)) {
    Node *node = new_node(ND_RETURN, expr(), NULL);
    expect(";");
    return node;
  }

  if (consume("{")) {
    Node *node = new_node(ND_BLOCK, NULL, NULL);
    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    node->block = head.next;
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

  if (is_typename()) {  // only check if next token is "int"
    return declaration();
  }

  Node *node = new_node(ND_EXPR_STMT, expr(), NULL);
  expect(";");
  return node;
}

bool initializer_end() {
  Token *tok = token;
  bool ret = consume("}") || consume(",") && consume("}");
  token = tok;
  return ret;
}

void expect_initializer_end() {
  Token *tok = token;
  if (consume(",") && consume("}")) return;
  token = tok;
  expect("}");
}

typedef struct Designator Designator;
struct Designator {
  Designator *next;
  int index;
};

Node *new_desg_node2(Var *var, Designator *desg) {
  if (!desg) {
    Node *var_node = new_node(ND_VAR, NULL, NULL);
    var_node->var = var;
    return var_node;
  }
  Node *node = new_desg_node2(var, desg->next);
  node = new_node(ND_ADD, node, new_node_num(desg->index));
  return new_node(ND_DEREF, node, NULL);
}

Node *new_desg_node(Var *var, Designator *desg, Node *rhs) {
  Node *lhs = new_desg_node2(var, desg);
  Node *node = new_node(ND_ASSIGN, lhs, rhs);
  return new_node(ND_EXPR_STMT, node, NULL);
}

Node *lvar_init_zero(Node *cur, Var *var, Type *ty, Designator *des) {
  if (ty->kind == TY_ARRAY) {
    for (int i = 0; i < ty->array_size; i++) {
      Designator desg = {des, i};
      cur = lvar_init_zero(cur, var, ty->ptr_to, &desg);
    }
    return cur;
  }
  cur->next = new_desg_node(var, des, new_node_num(0));
  cur = cur->next;
  return cur;
}

Node *lvar_initializer(Node *cur, Var *var, Type *ty, Designator *des) {
  // handle char a[] = "abc" || char a[4] = "abc"
  if (ty->kind == TY_ARRAY && ty->ptr_to->kind == TY_CHAR &&
      token->kind == TK_STR) {
    Token *tok = token;
    token = token->next;

    if (ty->array_size == -1) ty->array_size = tok->contents_len + 1;

    int len = (ty->array_size < tok->contents_len) ? ty->array_size
                                                   : tok->contents_len;
    int i;
    for (i = 0; i < len; i++) {
      Designator desg = {des, i};
      Node *rhs = new_node_num(tok->contents[i]);
      cur->next = new_desg_node(var, &desg, rhs);
      cur = cur->next;
    }

    for (; i < ty->array_size; i++) {
      Designator desg = {des, i};
      cur = lvar_init_zero(cur, var, ty->ptr_to, &desg);
    }
    return cur;
  }

  if (!consume("{")) {
    cur->next = new_desg_node(var, des, assign());
    cur = cur->next;
    return cur;
  }

  if (ty->kind == TY_ARRAY) {
    // handle "int a[] = {1,2,3}"
    if (ty->array_size == -1) {
      // int a[] = {1,2,3}
      // should be convert to below
      // a[0] = 1
      // a[1] = 2
      // a[2] = 3
      int i = 0;
      do {
        Designator desg = {des, i++};
        cur = lvar_initializer(cur, var, ty->ptr_to, &desg);
      } while (!initializer_end() && consume(","));

      ty->array_size = i;
      expect_initializer_end();
      return cur;
    }

    // should be array-like initialization
    // int a[3] = {1,2,hoge()}
    // a->type is array of int
    // should be convert to below
    // a[0] = 1
    // a[1] = 2
    // a[2] = hoge()

    // deal with
    // int a[2][3] = {{1,2,3}, {4,5,6}}
    // converts to
    // a[0][0] = 1: *((*a + 0) + 0) = 1
    // a[0][1] = 2: *((*a + 0) + 1) = 2
    // a[0][2] = 3: *((*a + 0) + 2) = 3
    // a[1][0] = 4: *((*a + 1) + 0) = 4
    // a[1][1] = 5: *((*a + 1) + 1) = 5
    // a[1][2] = 6: *((*a + 1) + 2) = 6

    int i = 0;
    int array_size = ty->array_size;
    do {
      if (i > array_size) {
        error_at(token->str, "excess elements in array initializer");
      }

      Designator desg = {des, i++};
      cur = lvar_initializer(cur, var, ty->ptr_to, &desg);

    } while (!initializer_end() && consume(","));
    expect_initializer_end();

    // fill the rest of array with 0
    while (++i < array_size) {
      Designator desg = {des, i};
      cur = lvar_init_zero(cur, var, ty->ptr_to, &desg);
    }
    return cur;
  }
  error_at(token->str, "invalid initializer");
}

// declaration = type ident ("[" num? "]")* ("=" lvar-initializer)? ";"
Node *declaration() {
  Type *ty = read_type();
  char *name = expect_ident();

  while (consume("[")) {
    if (consume("]")) {
      ty = array_of(ty, -1);
    } else {
      int array_size = expect_number();
      expect("]");
      ty = array_of(ty, array_size);
    }
  }

  if (consume(";")) {
    Var *var = push_var(name, ty, true);
    return new_node(ND_NULL, NULL, NULL);
  }  // end of declaration without initialization

  // declaration with initialization
  expect("=");
  Var *var = push_var(name, ty, true);
  // Node *node = new_node(ND_VAR, NULL, NULL);
  // node->var = var;

  Node head;
  head.next = NULL;
  lvar_initializer(&head, var, var->ty, NULL);
  Node *node = new_node(ND_BLOCK, NULL, NULL);
  node->block = head.next;

  // Node *node_var = new_node(ND_VAR, NULL, NULL);
  // node_var->var = push_var(name, ty, true);
  // Node *node = new_node(ND_ASSIGN, node_var, expr());
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
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else
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
  return postfix();
}

// postfix = primary ("[" expr "]" | "." ident)*
Node *postfix() {
  Node *node = primary();

  for (;;) {
    if (consume("[")) {
      Node *exp = new_node(ND_ADD, node, expr());
      expect("]");
      node = new_node(ND_DEREF, exp, NULL);
      continue;
    }

    if (consume(".")) {
      node = new_node(ND_MEMBER, node, NULL);
      node->member_name = expect_ident();
      continue;
    }
    return node;
  }
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

  if (token->kind == TK_STR) {
    Token *tok = token;
    token = token->next;
    Type *ty = array_of(char_type(), tok->contents_len + 1);
    Var *var = push_var(new_label(), ty, false);
    // exit(0);
    var->contents = tok->contents;
    var->contents_len = tok->contents_len;
    Node *node = new_node(ND_VAR, NULL, NULL);
    node->var = var;
    return node;
  }

  int val = expect_number();
  return new_node_num(val);
}
