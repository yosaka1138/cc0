#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークンの種類
typedef enum
{
  TK_RESERVED, // 記号
  TK_NUM,      // 数値
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token
{
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK＿NUMの時，その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

//現在見ているトークン
Token *token;
char *user_input;

// エラー報告のための関数
// printfと同じ引数をとる
void error(char *loc, char *fmt, ...);

// 次のトークンが期待している記号の時には，トークンを1つ読み進めて
// trueを返す。それ以外はfalse
bool consume(char *op);

// 次のトークンが期待している記号の時には，トークンを1つ読み進める
// それ以外はエラー
void expect(char *op);

// 次のトークンが期待している記号の時には，トークンを1つ読み進めてその値を返す
// それ以外はエラー
int expect_number();

bool at_eof();

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

bool startswith(char *p, char *q);

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize();

// Paeser
// 抽象構文木のノードの種類　
typedef enum
{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

struct Node
{
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺 left hand side
  Node *rhs;     // 右辺 right hand side
  int val;       // kindがND_NUMの時だけ使う
};

// 2項演算子用のNode
Node *new_node(NodeKind kind);

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);

// 数値を受け取る用のNode
Node *new_num(int val);

// パーサ
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();
// Code Generator
void gen(Node *node);