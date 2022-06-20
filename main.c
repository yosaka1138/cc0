#include "cc0.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
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
