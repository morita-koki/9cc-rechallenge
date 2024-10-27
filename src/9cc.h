#ifndef _9CC_H_
#define _9CC_H_

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TY_CHAR,
  TY_INT,
  TY_PTR,
  TY_ARRAY,
} TypeKind;

typedef struct Program Program;
typedef struct Function Function;
typedef struct Token Token;
typedef struct Type Type;
typedef struct Node Node;
typedef struct LVar LVar;
typedef struct Var Var;

struct Type {
  TypeKind kind;
  Type *ptr_to;
  size_t array_size;
};

Type *char_type();
Type *int_type();
Type *pointer_to(Type *base);
Type *array_of(Type *base, size_t size);
size_t size_of(Type *ty);
void visit(Node *node);
void add_type(Program *prog);

typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子 変数名、関数名
  TK_NUM,       // 数字
  TK_RETURN,    // return
  TK_IF,        // if
  TK_ELSE,      // else
  TK_WHILE,     // while
  TK_FOR,       // for
  TK_STR,       // 文字列リテラル
  TK_EOF,       // 入力終わり
} TokenKind;

struct Token {
  TokenKind kind;  // トークンの種類
  Token *next;     // 次のトークン
  int val;         // kindがTK_NUMの場合その数字
  char *str;       // トークン文字列
  int len;

  char *contents;
  int contents_len;
};

typedef enum {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_ASSIGN,     // =
  ND_VAR,        // ローカル変数
  ND_RETURN,     // return
  ND_IF,         // if
  ND_ELSE,       // else
  ND_WHILE,      // while
  ND_FOR,        // for
  ND_FUNCCALL,   // 関数呼び出し
  ND_BLOCK,      // {}
  ND_ADDR,       // アドレス &
  ND_DEREF,      // 間接参照 *
  ND_NUM,        // 整数
  ND_EXPR_STMT,  // 式文
  ND_NULL,       // 空
} NodeKind;

struct Node {
  Node *next;
  NodeKind kind;
  Node *lhs;
  Node *rhs;

  Type *ty;

  // Block
  Node **block;
  int block_count;

  // if-else, while, for statement
  // "if" (cond) { then } "else" { els }
  // "for" (init; cond; inc) { then }
  // "while" (cond) { then }
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  // func call
  char *funcname;
  Node **args;
  int arg_count;

  Var *var;  // kindがND_LVARのとき
  int val;
};

struct LVar {
  LVar *next;
  Var *var;
};

struct Var {
  char *name;
  Type *ty;
  int offset;
  bool is_local;

  char *contents;
  int contents_len;
};

struct Function {
  Function *next;
  char *name;
  Node *node;
  LVar *args;    // arguments
  LVar *locals;  // local variables
  int stack_size;
};

struct Program {
  LVar *globals;
  Function *funcs;
};

/* tokenize functoins */
void error(char *fmt, ...);
// void verror_at(char *loc, char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool starts_with(char *p, char *q);
bool is_alpha(char c);
bool is_alnum(char c);
char *strndup(char *p, int len);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
void tokenize();

/* parser funcions (AST) */
Var *find_var(Token *tok);
Var *push_var(char *name, Type *ty, bool is_local);
LVar *read_func_args();
bool consume(char *op);
Token *consume_kind(TokenKind kind);
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Program *program();
Function *function();
Type *read_type();
Node *stmt();
Node *declaration();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *postfix();
Node *primary();

/* generator */
void codegen(Program *prog);
void gen(Node *node);
void gen_addr(Node *node);
void load(Type *ty);
void store(Type *ty);

#endif