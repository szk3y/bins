#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

#define PARSER_BUF_SIZE 0x100
#define MAX_TOKEN_LEN 0x100
#define MAX_ARGC 0x100

typedef enum {
  TT_LPAREN,
  TT_RPAREN,
  TT_STRING,
  TT_PIPE,
  TT_REDOUT,
  TT_END,
} TokenType;

typedef enum {
  CT_OTHERS,
  CT_LETTER,
  CT_LF,
  CT_PIPE,
  CT_SHARP,
  CT_DOLLAR,
  CT_AND,
  CT_LPAREN,
  CT_RPAREN,
  CT_ASTERISK,
  CT_COMMA,
  CT_SEMICOLON,
  CT_REDIN,
  CT_REDOUT,
  CT_QUESTION,
  CT_LBRACKET,
  CT_BACKSLASH,
  CT_RBRACKET,
  CT_LBRACE,
  CT_RBRACE,
  CT_TILDE,
} CharType;

typedef struct token {
  char str[MAX_TOKEN_LEN];
  TokenType type;
  struct token* next;
} Token;

typedef struct command {
  char* argv[MAX_ARGC];
  struct command* next;
  pid_t pid;
} Command;

CharType ctype[256];

void del_token(Token* head)
{
  Token* token = head;
  Token* prev;
  while(token != NULL) {
    prev = token;
    token = token->next;
    free(prev);
  }
}

void del_command(Command* head)
{
  Command* cmd = head;
  Command* prev;
  while(cmd != NULL) {
    for(int i = 0; cmd->argv[i] != NULL; i++) {
      free(cmd->argv[i]);
    }
    prev = cmd;
    cmd = cmd->next;
    free(prev);
  }
}

void print_command(const Command* cmd)
{
  while(cmd != NULL) {
    for(int i = 0; cmd->argv[i] != NULL; i++) {
      fputs(cmd->argv[i], stdout);
      if(cmd->argv[i+1] == NULL) {
        putchar('\n');
      } else {
        putchar(' ');
      }
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

  token = xcalloc(sizeof(Token));
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
      // TODO: multi-line script
      token->type = TT_END;
      ch = next_ch(fp);
      break;
    case CT_PIPE:
      token->type = TT_PIPE;
      strcpy(token->str, "|");
      ch = next_ch(fp);
      break;
    case CT_REDOUT:
      // TODO: implement '>>', '>&'
      token->type = TT_REDOUT;
      strcpy(token->str, ">");
      ch = next_ch(fp);
      break;
    case CT_OTHERS:
      fprintf(stderr, "'%c' is not implemented.\n", ch);
      exit(1);
    default:
      fputs("Unreachable code. Maybe, you forgot to call init_ctype(),\n",
          stderr);
      fprintf(stderr, "or '%c' is not implemented.\n", ch);
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
  cmd = xcalloc(sizeof(Command));
  cmd_begin = cmd;
  while(token->type != TT_END) {
    switch(token->type) {
      case TT_STRING:
        cmd->argv[i] = xcalloc(sizeof(token->str));
        strncpy(cmd->argv[i], token->str, sizeof(token->str));
        i++;
        break;
      case TT_PIPE:
        cmd->argv[i] = NULL;
        i = 0;
        cmd->next = xcalloc(sizeof(Command));
        cmd = cmd->next;
        break;
      case TT_REDOUT:
        // null terminate argument vector
        cmd->argv[i] = NULL;
        i = 0;

        // create a new command for redirection
        cmd->next = xcalloc(sizeof(Command));
        cmd = cmd->next;
        // write redirection operator to argv[0]
        cmd->argv[0] = xcalloc(sizeof(token->str));
        strncpy(cmd->argv[0], token->str, sizeof(token->str));
        // write redirection operand to argv[1]
        token = token->next;
        assert(token != NULL);
        if(token->type != TT_STRING) {
          fputs("Parse Error: Redirection operand not found\n", stderr);
          exit(1);
        }
        cmd->argv[1] = xcalloc(sizeof(token->str));
        strncpy(cmd->argv[1], token->str, sizeof(token->str));
        i = 2;
        break;
      default:
        break;
    }
    token = token->next;
  }
  cmd->next = NULL;
  del_token(head);
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
  ctype['!'] = CT_LETTER;
  ctype['%'] = CT_LETTER;
  ctype['+'] = CT_LETTER;
  ctype['-'] = CT_LETTER;
  ctype['.'] = CT_LETTER;
  ctype['/'] = CT_LETTER;
  ctype[':'] = CT_LETTER;
  ctype['='] = CT_LETTER;
  ctype['>'] = CT_REDOUT;
  ctype['@'] = CT_LETTER;
  ctype['\n'] = CT_LF;
  ctype['^'] = CT_LETTER;
  ctype['_'] = CT_LETTER;
  ctype['|'] = CT_PIPE;
  // not implemented characters
  //ctype['#'] = CT_SHARP;
  //ctype['$'] = CT_DOLLAR;
  //ctype['&'] = CT_AND;
  //ctype['('] = CT_LPAREN;
  //ctype[')'] = CT_RPAREN;
  //ctype['*'] = CT_ASTERISK;
  //ctype[','] = CT_COMMA;
  //ctype[';'] = CT_SEMICOLON;
  //ctype['<'] = CT_REDIN;
  //ctype['?'] = CT_QUESTION;
  //ctype['['] = CT_LBRACKET;
  //ctype['\\'] = CT_BACKSLASH;
  //ctype[']'] = CT_RBRACKET;
  //ctype['`'] = CT_BACKTICK;
  //ctype['{'] = CT_LBRACE;
  //ctype['}'] = CT_RBRACE;
  //ctype['~'] = CT_TILDE;
}

void redirect_stdout(const char* path)
{
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd < 0) {
    perror("open");
    exit(1);
  }
  close(1);
  dup2(fd, 1);
  close(fd);
}

bool is_redirection(const Command* cmd)
{
  if(cmd == NULL) {
    return false;
  }
  return strcmp(cmd->argv[0], ">") == 0;
}

// Manipulate fd and return true when a given command is redirection.
// When a given command is not redirection, it only returns false.
bool do_redirection(const Command* cmd)
{
  if(cmd == NULL) {
    return false;
  }
  if(strcmp(cmd->argv[0], ">") == 0) {
    redirect_stdout(cmd->argv[1]);
    return true;
  } else {
    return false;
  }
}

/*
 *  When you give a shell program that looks like
 *  proc1 | proc2 | proc3 ...
 *  to this function, it will build a process chain like this:
 *
 *      fork()
 *      /    \
 *  child    parent
 *    |        |
 *  proc1    fork()
 *           /    \
 *      child     parent
 *        |         |
 *      proc2     fork()
 *                /    \
 *            child    parent
 *              |        |
 *            proc3      |
 *                       |
 *                     return
 */
void build_process_chain(Command* head)
{
  int fds[2] = { -1, -1 };
  int prev_fds[2];
  pid_t pid;
  Command* redcmd;

  for(Command* cmd = head; cmd != NULL; cmd = cmd->next) {
    if(is_redirection(cmd)) { // skip redirection command
      continue;
    }
    memcpy(prev_fds, fds, sizeof(prev_fds));
    if(pipe(fds) < 0) {
      perror("pipe");
      exit(1);
    }
    pid = fork();
    if(pid < 0) {
      perror("fork");
      exit(1);
    }
    if(pid == 0) { // child
      if(cmd != head) { // not the first process
        close(0);
        dup2(prev_fds[0], 0);
        close(prev_fds[0]);
        close(prev_fds[1]);
      }
      if(cmd->next != NULL) { // not the last process
        close(1);
        dup2(fds[1], 1);
        close(fds[1]);
        close(fds[0]);
      }
      redcmd = cmd->next;
      while(do_redirection(redcmd)) {
        redcmd = redcmd->next;
      }
      execvp(cmd->argv[0], cmd->argv);
      perror("execvp"); // execX only returns when it fails
      exit(1);
    } else { // parent
      if(prev_fds[0] != -1) {
        close(prev_fds[0]);
      }
      if(prev_fds[1] != -1) {
        close(prev_fds[1]);
      }
      cmd->pid = pid;
      continue;
    }
  }
}

void wait_children(const Command* command)
{
  for(const Command* cmd = command; cmd != NULL; cmd = cmd->next) {
    waitpid(cmd->pid, NULL, 0);
  }
}

int main(int argc, char** argv)
{
  FILE* fp;
  Command* cmd;

  if(argc <= 1) {
    fputs("This function is not implemented.\n", stderr);
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
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
  puts("executing...");
  build_process_chain(cmd);
  wait_children(cmd);
  puts("Done!");
  // TODO: free commands and tokens
  del_command(cmd);
  return 0;
}
