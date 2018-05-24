#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARSER_BUF_SIZE 0x100
#define MAX_TOKEN_LEN 0x100
#define MAX_ARGC 0x100

typedef enum {
  TT_LPAREN,
  TT_RPAREN,
  TT_STRING,
  TT_PIPE,
  TT_END,
} TokenType;

typedef enum {
  CT_LPAREN,
  CT_RPAREN,
  CT_LETTER,
  CT_PIPE,
  CT_SEMICOLON,
  CT_LF,
  CT_OTHERS,
} CharType;

typedef struct token {
  char str[MAX_TOKEN_LEN];
  TokenType type;
  struct token* next;
} Token;

typedef struct command {
  char* argv[MAX_ARGC];
  struct command* next;
} Command;

CharType ctype[256];

void print_command(const Command* cmd)
{
  while(cmd != NULL) {
    for(int i = 0; cmd->argv[i] != NULL; i++) {
      puts(cmd->argv[i]);
    }
    cmd = cmd->next;
  }
}

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
  token->next = NULL;

  while(isspace(ch)) {
    ch = next_ch(fp);
  }
  if(ch == EOF) {
    token->type = TT_END;
    return token;
  }
  switch(ctype[ch]) {
    case CT_LETTER:
      token->type = TT_STRING;
      for(int i = 0; ctype[ch] == CT_LETTER; i++) {
        token->str[i] = (char)ch;
        ch = next_ch(fp);
      }
      break;
    case CT_LF:
      token->type = TT_END;
      break;
    case CT_PIPE:
      token->type = TT_PIPE;
      strcpy(token->str, "|");
      ch = next_ch(fp);
      break;
    case CT_LPAREN:
    case CT_RPAREN:
    case CT_SEMICOLON:
    case CT_OTHERS:
      fprintf(stderr, "'%c' is not implemented.\n", ch);
      exit(1);
    default:
      fputs("Unreachable code. Maybe, you forgot to call init_ctype()\n",
          stderr);
      exit(1);
  }
  return token;
}

Command* parse(FILE* fp)
{
  Token* head;
  Token* token;
  Command* cmd_begin;
  Command* cmd;
  int i = 0;

  head = next_token(fp);
  token = head;
  while(token->type != TT_END) {
    token->next = next_token(fp);
    token = token->next;
  }
  token->next = NULL;

  token = head;
  cmd = malloc(sizeof(Command));
  if(cmd == NULL) {
    perror("malloc");
    exit(1);
  }
  cmd_begin = cmd;
  while(token->type != TT_END) {
    switch(token->type) {
      case TT_STRING:
        cmd->argv[i] = malloc(sizeof(token->str));
        if(cmd->argv[i] == NULL) {
          perror("malloc");
          exit(1);
        }
        strncpy(cmd->argv[i], token->str, sizeof(token->str));
        i++;
        break;
      case TT_PIPE:
        cmd->argv[i] = NULL;
        i = 0;
        cmd->next = malloc(sizeof(Command));
        if(cmd->next == NULL) {
          perror("malloc");
          exit(1);
        }
        cmd = cmd->next;
        break;
      default:
        break;
    }
    token = token->next;
  }
  cmd->next = NULL;
  return cmd_begin;
}

void init_ctype()
{
  for(size_t i = 0; i < sizeof(ctype) / sizeof(*ctype); i++) {
    ctype[i] = CT_OTHERS;
  }
  for(int i = '0'; i <= '9'; i++) {
    ctype[i] = CT_LETTER;
  }
  for(int i = 'A'; i <= 'Z'; i++) {
    ctype[i] = CT_LETTER;
  }
  for(int i = 'a'; i <= 'z'; i++) {
    ctype[i] = CT_LETTER;
  }
  ctype['_'] = CT_LETTER;
  ctype['\n'] = CT_LF;
  ctype['|'] = CT_PIPE;
  // not implemented characters
  //ctype['('] = CT_LPAREN;
  //ctype[')'] = CT_RPAREN;
  //ctype[';'] = CT_SEMICOLON;
}

int main(int argc, char** argv)
{
  FILE* fp;
  Command* cmd;

  if(argc <= 1) {
    fputs("This function is not implemented.\n", stderr);
    fprintf(stderr, "Usage: %s '<write program here>'\n", argv[0]);
    exit(1);
  }

  init_ctype();
  fp = fopen(argv[1], "rb");
  if(fp == NULL) {
    perror("fopen");
    exit(1);
  }
  cmd = parse(fp);
  fclose(fp);
  print_command(cmd);
  return 0;
}
