#include "cc0.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくないです\n");
    return 1;
  }
  // locals = NULL;
  // 入力をトークナイズする
  user_input = argv[1];
  token = tokenize();
  program();

  // アゼンブリの前半を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  cur_func = 0;
  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    cur_func++;
    gen(code[i]);
  }
  return 0;
}
