#ifndef _9CC_H_
#define _9CC_H_

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子 変数名、関数名
  TK_NUM,       // 数字
  TK_RETURN,    // return
  TK_IF,        // if
  TK_ELSE,      // else
  TK_WHILE,     // while
  TK_FOR,       // for
  TK_EOF,       // 入力終わり
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;  // トークンの種類
  Token *next;     // 次のトークン
  int val;         // kindがTK_NUMの場合その数字
  char *str;       // トークン文字列
  int len;
};

typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_ASSIGN,    // =
  ND_LVAR,      // ローカル変数
  ND_RETURN,    // return
  ND_IF,        // if
  ND_ELSE,      // else
  ND_WHILE,     // while
  ND_FOR,       // for
  ND_FUNCCALL,  // 関数呼び出し
  ND_BLOCK,     // {}
  ND_NUM,       // 整数
} NodeKind;

typedef struct Node Node;
typedef struct LVar LVar;
typedef struct Var Var;

struct Node {
  Node *next;
  NodeKind kind;
  Node *lhs;
  Node *rhs;

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
  int len;
  int offset;
};

typedef struct Function Function;

struct Function {
  Function *next;
  char *name;
  Node *node;
  LVar *args;    // arguments
  LVar *locals;  // local variables
  int stack_size;
};

/* tokenize functoins */
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool starts_with(char *p, char *q);
bool is_alpha(char c);
bool is_alnum(char c);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
void tokenize();

/* parser funcions (AST) */
Var *find_var(Token *tok);
Var *push_var(char *name, int len);
LVar *read_func_args();
bool consume(char *op);
Token *consume_kind(TokenKind kind);
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Function *program();
Function *function();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/* generator */
void codegen(Function *prog);
void gen(Node *node);
void gen_lval(Node *node);

#endif