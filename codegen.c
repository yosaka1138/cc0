#include "cc0.h"
#include <stdio.h>
#include <string.h>

//
void gen_lval(Node *node) {
  if (node->kind == ND_DEREF) {
    // 右辺値として処理する→genでnode->lhsを処理すれば良い
    gen(node->lhs);
    return;
  }
  if (node->kind != ND_LVAR)
    error("not ND_LVAR");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

int genCounter = 0;
char *argRegs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// Code Generator
void gen(Node *node) {
  // printf("  # START GEN\n");
  if (!node)
    return;
  genCounter += 1;
  int id = genCounter;
  int argCount = 0;

  char name[100] = {0};
  switch (node->kind) {

  case ND_ADDR:
    gen_lval(node->lhs);
    return;

  case ND_DEREF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;

  case ND_FUNC_DEF:
    printf("  # ND_FUNC_DEF\n");
    printf("%s:\n", node->funcname);

    // プロローグ
    // 変数分の領域を確保
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    // 引数の値をStackに積む
    // 自動的にrspの値が引かれる
    for (int i = 0; node->args[i]; i++) {
      printf("  push %s\n", argRegs[i]);
      argCount++;
    }
    if (locals[cur_func]) {
      int offset = locals[cur_func][0].offset;
      offset -= argCount * 8;
      printf("  sub rsp, %d\n", offset);
    }

    // 引数を除いた変数の数分rspをずらして、変数領域を確保する
    gen(node->lhs);

    // エピローグ
    // スタックストップに式全体の値が残っているはずなので，
    // それをRAXにロードして関数からの返り値とする
    // FIXME: mov rax, 0が必要か？
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    printf("  # END ND_FUNC_DEF\n");
    return;

  case ND_FUNC_CALL:
    printf("  # ND_FUNC_CALL\n");
    memcpy(name, node->funcname, node->len);
    // int argCount = 0;
    for (int i = 0; node->block[i]; i++) {
      gen(node->block[i]);
      argCount++;
    }
    for (int i = argCount - 1; i >= 0; i--) {
      printf("  pop %s\n", argRegs[i]);
    }
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n");
    printf("  jnz .L.call.%03d\n", id);
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  jmp .L.end.%03d\n", id);
    printf(".L.call.%03d:\n", id);
    printf("  sub rsp, 8\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  add rsp, 8\n");
    printf(".L.end.%03d:\n", id);
    printf("  push rax\n"); // FIXME: あってるか？
    printf("  # END ND_FUNC_CALL\n");
    return;

  case ND_BLOCK:
    printf("  # ND_BLOCK\n");
    for (int i = 0; node->block[i]; i++) {
      gen(node->block[i]);
    }
    printf("  # END ND_BLOCK\n");
    return;

  case ND_FOR:
    printf("  # ND_FOR\n");
    gen(node->lhs->lhs);
    printf(".L.begin%03d:\n", id);
    gen(node->lhs->rhs);
    if (!node->lhs->rhs) {
      printf("  push 1\n");
    }
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .L.end%03d\n", id);
    gen(node->rhs->rhs);
    gen(node->rhs->lhs);
    printf("  jmp .L.begin%03d\n", id);
    printf(".L.end%03d:\n", id);
    printf("  # END ND_FOR\n");
    return;

  case ND_WHILE:
    printf("  # ND_WHILE\n");
    printf(".L.begin%03d:\n", id);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .L.end%03d\n", id);
    gen(node->rhs);
    printf("  jmp .L.begin%03d\n", id);
    printf(".L.end%03d:\n", id);
    printf("  # END ND_WHILE\n");
    return;

  case ND_IF:
    printf("  # ND_IF\n");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .L.else%03d\n", id);
    // if 文の中身を書く
    if (node->rhs->kind == ND_ELSE) {
      gen(node->rhs->lhs);
    } else {
      gen(node->rhs);
    }
    printf("  jmp .L.end%03d\n", id);
    printf(".L.else%03d:\n", id);
    // else文がある場合はelseの中身を書く
    if (node->rhs->kind == ND_ELSE) {
      gen(node->rhs->rhs);
    }
    printf(".L.end%03d:\n", id);
    printf("  # END ND_IF\n");
    return;

  case ND_NUM:
    printf("  # ND_NUM\n");
    printf("  push %d\n", node->val);
    printf("  # END ND_NUM\n");
    return;

  case ND_LVAR:
    printf("  # ND_LVAR\n");
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    printf("  # END ND_LVAR\n");
    return;

  case ND_ASSIGN:
    printf("  # ND_ASSIGN\n");
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    printf("  # END ND_ASSIGN\n");
    return;

  case ND_RETURN:
    printf("  # ND_RETURN\n");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    printf("  # END ND_RETURN\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  switch (node->kind) {
  // +
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  // -
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  // *
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  // /
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  // ==
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("sete al\n");
    printf("  movzb rax, al\n");
    break;
  // !=
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  // <
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  // <=
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");
  printf("  # END GEN\n");
}
