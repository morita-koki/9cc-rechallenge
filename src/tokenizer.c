#include "9cc.h"

extern char *user_input;
extern Token *token;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;  // calc byte
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");  // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool starts_with(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

bool is_aplha(char c) {
  // clang-format off
  return ('a' <= c && c <= 'z') || 
         ('A' <= c && c <= 'Z') || 
         (c == '_');
  // clang-format on
}

bool is_alnum(char c) {
  // clang-format off
  return is_aplha(c) || 
         ('0' <= c && c <= '9');
  // clang-format on
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = strndup(str, len);
  tok->len = len;
  cur->next = tok;
  return tok;
}

char *strndup(char *p, int len) {
  char *s = malloc(len + 1);
  memcpy(s, p, len);
  s[len] = '\0';
  return s;
}

void tokenize() {
  char *p = user_input;
  token = calloc(1, sizeof(Token));
  token->next = NULL;
  Token *cur = token;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    // multi-letter punctuator
    if (starts_with(p, "==") || starts_with(p, "!=") || starts_with(p, "<=") ||
        starts_with(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // single-letter punctuator
    if (strchr("+-*/()<>=;{},&[]", *p)) {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p += 1;
      continue;
    }

    // return
    if (starts_with(p, "return") && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    // if
    if (starts_with(p, "if") && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    // else
    if (starts_with(p, "else") && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    // while
    if (starts_with(p, "while") && !is_alnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    // for
    if (starts_with(p, "for") && !is_alnum(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    // int
    if (starts_with(p, "int") && !is_alnum(p[3])) {
      cur = new_token(TK_RESERVED, cur, p, 3);
      p += 3;
      continue;
    }

    // sizeof
    if (starts_with(p, "sizeof") && !is_alnum(p[6])) {
      cur = new_token(TK_RESERVED, cur, p, 6);
      p += 6;
      continue;
    }

    // identifier (multiple-letter variable name)
    if (is_aplha(*p)) {
      char *q = p;
      while (is_alnum(*p)) p++;
      int len = p - q;
      cur = new_token(TK_IDENT, cur, q, len);
      continue;
    }

    // number
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "invalid token");
  }

  cur = new_token(TK_EOF, cur, p, 0);
  token = token->next;
}