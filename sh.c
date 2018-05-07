#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARSER_BUF_SIZE 0x100
#define MAX_TOKEN_LEN 0x100

typedef enum {
  TK_LPAREN,
  TK_RPAREN,
  TK_STRING,
  TK_PIPE,
  TK_END,
} TokenKind;

typedef enum {
  CK_LPAREN,
  CK_RPAREN,
  CK_LETTER,
  CK_PIPE,
  CK_SEMICOLON,
  CK_OTHERS,
} CharKind;

typedef struct token {
  char str[MAX_TOKEN_LEN];
  TokenKind kind;
  struct token* next;
} Token;

CharKind ckind[256];

int next_ch(FILE* fp)
{
  static int ch = 0;
  if(ch == EOF) {
    return ch;
  }
  ch = fgetc(fp);
  return ch;
}

Token* next_token(FILE* fp)
{
  Token* token;
  static int ch = ' ';

  token = malloc(sizeof(Token));
  if(!token) {
    perror("malloc");
    exit(1);
  }

  while(isspace(ch)) {
    ch = next_ch(fp);
  }
  if(ch == EOF) {
    token->kind = TK_END;
    return token;
  }
  switch(ckind[ch]) {
    case CK_LETTER:
      for(int i = 0; ckind[ch] == CK_LETTER; i++) {
        token->str[i] = (char)ch;
        ch = next_ch(fp);
      }
      break;
    case CK_LPAREN:
    case CK_RPAREN:
    case CK_PIPE:
    case CK_OTHERS:
      fprintf(stderr, "CK_OTHERS found. '%c' is not implemented.\n", ch);
      exit(1);
    default:
      fputs("Unreachable code. Maybe, you forgot to call init_ckind()\n",
          stderr);
      exit(1);
  }
  return token;
}

void init_ckind()
{
  for(size_t i = 0; i < sizeof(ckind); i++) {
    ckind[i] = CK_OTHERS;
  }
  for(int i = '0'; i <= '9'; i++) {
    ckind[i] = CK_LETTER;
  }
  for(int i = 'A'; i <= 'Z'; i++) {
    ckind[i] = CK_LETTER;
  }
  for(int i = 'a'; i <= 'z'; i++) {
    ckind[i] = CK_LETTER;
  }
  ckind['_'] = CK_LETTER;
  // not implemented characters
  //ckind['|'] = CK_PIPE;
  //ckind['('] = CK_LPAREN;
  //ckind[')'] = CK_RPAREN;
  //ckind[';'] = CK_SEMICOLON;
}

int main(int argc, char** argv)
{
  FILE* fp;

  if(argc <= 1) {
    fputs("This function is not implemented.\n", stderr);
    fprintf(stderr, "Usage: %s '<write program here>'\n", argv[0]);
    exit(1);
  }

  init_ckind();
  fp = fopen(argv[1], "rb");
  if(fp == NULL) {
    perror("fopen");
    exit(1);
  }
  fclose(fp);
  return 0;
}
