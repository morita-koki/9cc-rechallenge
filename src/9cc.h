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
  TK_NUM,       // 数字
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
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
  ND_EQ,   // ==
  ND_NE,   // !=
  ND_LT,   // <
  ND_LE,   // <=
  ND_NUM,  // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

/* tokenize functoins */
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool starts_with(char *p, char *q);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

/* parser funcions (AST) */
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/* generator */
void gen(Node *node);

#endif