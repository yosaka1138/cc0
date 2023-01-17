#include "cc0.h"
#include <stdlib.h>
#include <string.h>

// エラー報告のための関数
// printfと同じ引数をとる
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 具体的な位置を示すエラー報告のための関数
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個のwhitespace
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号の時には，トークンを1つ読み進めて
// trueを返す。それ以外はfalse
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  // トークンを次に進める
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
};

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }
  return NULL;
}

// 次のトークンが期待している記号の時には，トークンを1つ読み進める
// それ以外はエラー
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%s'ではありません", op);
  token = token->next;
}

// 次のトークンが期待している記号の時には，トークンを1つ読み進めてその値を返す
// それ以外はエラー
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数字ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

typedef struct ReservedWord ReservedWord;
struct ReservedWord {
  char *word;
  TokenKind kind;
};

ReservedWord reservedWords[] = {
    {"return", TK_RETURN}, {"if", TK_IF},   {"else", TK_ELSE},
    {"while", TK_WHILE},   {"for", TK_FOR}, {"", TK_EOF},
};

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    // 複数文字の比較演算子
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    bool found = false;
    for (int i = 0; reservedWords[i].kind != TK_EOF; i++) {
      char *w = reservedWords[i].word;
      int len = strlen(w);
      if (startswith(p, w) && !is_alnum(p[len])) {
        cur = new_token(reservedWords[i].kind, cur, p, len);
        p += len;
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    }

    // アルファベット小文字1文字ならTK_IDENT
    if ('a' <= *p && *p <= 'z') {
      char *c = p;
      while (is_alnum(*c)) {
        c++;
      }
      int len = c - p;
      cur = new_token(TK_IDENT, cur, p, len);
      p = c;
      continue;
    }

    // 1文字の演算子
    if (strchr("+-*/()<>=;{},", *p)) {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "トークナイズできません。");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// 2項演算子用のNode
Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// 数値を受け取る用のNode
Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}

Node *expr() { return assign(); }

// stmt = expr ";"
//        | "{" stmt "}"
//        |  "if" "(" expr ")" stmt ("else" stmt)?
//        |  "while" "(" expr ")" stmt
//        |  "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        |  ...
Node *stmt() {
  Node *node;

  if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    // TODO: とりあえず100行Nodeが入る
    node->block = calloc(100, sizeof(Node));
    for (int i = 0; !consume("}"); i++) {
      node->block[i] = stmt();
    }
    return node;
  }

  if (consume_kind(TK_FOR)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    // for文を２つのNodeに分ける
    // for (A; B; C) D;を(A, B) (C, D)に分ける
    Node *left = calloc(1, sizeof(Node));
    left->kind = ND_FOR_LEFT;
    Node *right = calloc(1, sizeof(Node));
    right->kind = ND_FOR_RIGHT;

    // for (A; B; C) D のA,B,Cは省略可能なため
    if (!consume(";")) {
      left->lhs = expr();
      expect(";");
    }
    if (!consume(";")) {
      left->rhs = expr();
      expect(";");
    }
    if (!consume(")")) {
      right->lhs = expr();
      expect(")");
    }
    right->rhs = stmt();
    node->lhs = left;
    node->rhs = right;
    return node;
  }

  if (consume_kind(TK_WHILE)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    return node;
  }

  if (consume_kind(TK_IF)) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    if (consume_kind(TK_ELSE)) {
      Node *els = calloc(1, sizeof(Node));
      els->kind = ND_ELSE;
      els->lhs = node->rhs;
      els->rhs = stmt();
      node->rhs = els;
    }
    return node;
  }

  if (consume_kind(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }
  expect(";");
  return node;
}

// program = func*
void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = func();
  }
  code[i] = NULL;
}

// func = ident "("  ")" "{" stmt "}"
Node *func() {
  Node *node;
  Token *tok = (consume_kind(TK_IDENT));
  if (tok == NULL) {
    error("not function!");
  }
  node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_DEF;
  node->funcname = calloc(100, sizeof(char));
  memcpy(node->funcname, tok->str, tok->len);
  expect("(");
  // TODO: args
  expect(")");
  node->lhs = stmt();
  // node->block = calloc(100, sizeof(Node));
  // for (int i = 0; !consume("}"); i++) {
  //   node->block[i] = stmt();
  // }
  // expect("}");
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();
  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());
  return primary();
}

// primary = num
//        |  ident ( "(" expr* ")")?
//        |  "(" expr ")"
Node *primary() {

  // 次のトークンが(なら( expr ) という形になっているはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  Token *tok = consume_kind(TK_IDENT);

  if (tok) {
    if (consume("(")) {
      // 関数呼び出し
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_FUNC_CALL;
      node->funcname = calloc(100, sizeof(char));
      memcpy(node->funcname, tok->str, tok->len);
      // TODO: とりあえず引数10個まで
      node->block = calloc(10, sizeof(Node));
      for (int i = 0; !consume(")"); i++) {
        node->block[i] = expr();
        if (consume(")")) {
          break;
        }
        // 引数の間には','がある
        expect(",");
      }

      return node;
    }
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if (locals == NULL) {
        lvar->offset = 8;
      } else {
        lvar->offset = locals->offset + 8;
      }
      node->offset = lvar->offset;
      locals = lvar;
    }

    /* node->offset = (tok->str[0] - 'a' + 1) * 8; */
    return node;
  }

  // そうでなければ数値
  return new_num(expect_number());
}
