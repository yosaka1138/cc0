#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 数値
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK＿NUMの時，その数値
  char *str;      // トークン文字列
};

//現在見ているトークン
Token *token;
char *user_input;

// エラー報告のための関数
// printfと同じ引数をとる
void error(char *loc, char *fmt, ...) {
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
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  // トークンを次に進める
  token = token->next;
  return true;
}

// 次のトークンが期待している記号の時には，トークンを1つ読み進める
// それ以外はエラー
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error(token->str, "'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが期待している記号の時には，トークンを1つ読み進めてその値を返す
// それ以外はエラー
int expect_number() {
  if (token->kind != TK_NUM)
    error(token->str, "数字ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
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
    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error(p, "トークナイズできません。");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

// Paeser

// 抽象構文木のノードの種類　
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺 left hand side
  Node *rhs;     // 右辺 right hand side
  int val;       // kindがND_NUMの時だけ使う
};

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

// 事前に宣言しておかないとエラーになる
Node *expr();
Node *mul();
Node *primary();

// パーサ
Node *expr() {
  Node *node = mul();
  for (;;) {
    if (consume('+'))
      node = new_binary(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = primary();
  for (;;) {
    if (consume('*'))
      node = new_binary(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_binary(ND_DIV, node, primary());
    else
      return node;
  }
}

Node *primary() {
  // 次のトークンが(なら( expr ) という形になっているはず
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }
  // そうでなければ数値
  return new_num(expect_number());
}

// Code Generator
void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }
  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくないです\n");
    return 1;
  }

  // 入力をトークナイズする
  user_input = argv[1];
  token = tokenize();
  Node *node = expr();

  // アゼンブリの前半を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックストップに式全体の値が残っているはずなので，
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
