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
  TK_IDENT,     // 識別子
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
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_ASSIGN,     // =
  ND_LVAR,       // ローカル変数
  ND_RETURN,     // return
  ND_IF,         // if
  ND_ELSE,       // else
  ND_WHILE,      // while
  ND_FOR,        // for
  ND_FOR_LEFT,   // for
  ND_FOR_RIGHT,  // for
  ND_BLOCK,      // {}
  ND_NUM,        // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  Node **block;
  int block_count;
  int val;
  int offset;  // kindがND_LVARのとき
};

typedef struct LVar LVar;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

/* tokenize functoins */
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool starts_with(char *p, char *q);
bool is_alnum(char c);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
void tokenize();

/* parser funcions (AST) */
LVar *find_lvar(Token *tok);
bool consume(char *op);
Token *consume_ident();
Token *consume_return();
Token *consume_if();
Token *consume_else();
void expect(char *op);
int expect_number();
bool at_eof();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void program();
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
void gen(Node *node);
void gen_lval(Node *node);

#endif