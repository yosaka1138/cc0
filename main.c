#include "cc0.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくないです\n");
    return 1;
  }

  // 入力をトークナイズする
  user_input = argv[1];
  token = tokenize();
  program();

  // アゼンブリの前半を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // プロローグ
  // 変数26個分の領域を確保
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果としてスタックに一つ値が残っているはず
    // スタックから溢れないようにpopしておく
    printf("  pop rax\n");
  }

  // エピローグ
  // スタックストップに式全体の値が残っているはずなので，
  // それをRAXにロードして関数からの返り値とする
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
