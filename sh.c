#include <stdio.h>
#include <stdlib.h>

#define PARSER_BUF_SIZE 0x100

typedef enum {
  PROGRAM,
  PIPE,
} Type;

typedef struct token {
  char* str;
  Type type;
  struct token* next;
} Token;

Token* parse(const char* program)
{
  int index = 0;
  char ch;
  char buf[PARSER_BUF_SIZE];
  Token* first;
  Token* token;

  first = malloc(sizeof(Token));
  if(!first) {
    perror("malloc");
    exit(1);
  }
  token = first;
  token->next = NULL;

  for(const char* ptr = program; *ptr != 0; ptr++) {
    ch = *ptr;
    switch(ch) {
      case '|':
        // TODO: flush buf
        token->next = malloc(sizeof(Token));
        if(!token->next) {
          perror("malloc");
          exit(1);
        }
        token = token->next;
        token->str = "|";
        token->type = PIPE;
        token->next = NULL;
        break;
      default:
        buf[index++] = ch;
        break;
    }
  }
  return first;
}

int main(int argc, char** argv)
{
  if(argc <= 1) {
    fputs("This function is not implemented.\n", stderr);
    fprintf(stderr, "Usage: %s '<write program here>'\n", argv[0]);
    exit(1);
  } else {
    parse(argv[1]);
  }
  return 0;
}
