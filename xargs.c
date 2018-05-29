#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DEFAULT_NARGS 10000
#define BUFSIZE 0x1000

char global_buf[BUFSIZE];
bool global_eof_flag = false;

typedef struct arg {
  char* str;
  struct arg* next;
} Arg;

Arg* new_arg()
{
  Arg* ptr = malloc(sizeof(Arg));
  if(ptr == NULL) {
    perror("malloc");
    exit(1);
  }
  ptr->str = NULL;
  ptr->next = NULL;
  return ptr;
}

void del_arg(Arg* head)
{
  Arg* arg = head;
  Arg* victim;
  while(arg != NULL) {
    victim = arg;
    arg = arg->next;
    assert(victim->str);
    free(victim->str);
    free(victim);
  }
}

bool delim_table[256];

void init_delim_table()
{
  memset(delim_table, false, sizeof(delim_table));
  delim_table['\t'] = true;
  delim_table['\n'] = true;
  delim_table[' '] = true;
}

bool is_delim(unsigned char ch)
{
  return delim_table[ch];
}

void set_delim(unsigned char ch)
{
  memset(delim_table, false, sizeof(delim_table));
  delim_table[ch] = true;
}

Arg* next_arg(FILE* fp)
{
  char ch;
  Arg* arg;

  for(size_t i = 0; i < sizeof(global_buf); i++) {
    ch = fgetc(fp);
    if(ch == EOF) {
      global_eof_flag = true;
      global_buf[i] = '\0';
      break;
    }
    if(is_delim(ch)) {
      global_buf[i] = '\0';
      break;
    } else {
      global_buf[i] = ch;
    }
  }
  if(strlen(global_buf) + 1 == sizeof(global_buf)) {
    fputs("Too long argument", stderr);
    exit(1);
  }
  if(strlen(global_buf) == 0) {
    return NULL;
  }
  arg = new_arg();
  arg->str = malloc(strlen(global_buf));
  if(arg->str == NULL) {
    perror("malloc");
    exit(1);
  }
  strcpy(arg->str, global_buf);
  return arg;
}

Arg* parse_input(FILE* fp)
{
  Arg* head = next_arg(fp);
  Arg* arg = head;
  while(!global_eof_flag) {
    arg->next = next_arg(fp);
  }
  return head;
}

void exec_command(char** argv)
{
  pid_t pid = fork();
  if(pid < 0) {
    perror("fork");
    exit(1);
  }
  if(pid == 0) { // child
    execvp(argv[0], argv);
    // execX never returns
  } else { // parent
    waitpid(pid, NULL, 0); // TODO: status
  }
}

void do_xargs(FILE* fp, int nargs, int argc, char** command_line_argv)
{
  int i = argc;
  Arg* head = parse_input(fp);
  // +1 for nullptr
  char** argv = malloc(sizeof(char*) * (argc+nargs+1));
  if(argv == NULL) {
    perror("malloc");
    exit(1);
  }

  for(int k = 0; k < argc; k++) { // set command line arguments
    argv[k] = command_line_argv[k];
  }

  for(Arg* arg = head; arg != NULL; arg = arg->next) {
    if(i == nargs) { // execute command
      argv[i] = NULL;
      exec_command(argv);
      i = argc; // reset index
    } else { // push an argument
      argv[i] = arg->str;
      i++;
    }
  }
  if(i != argc) {
    argv[i] = NULL;
    exec_command(argv);
  }
  del_arg(head);
  free(argv);
}

int main(int argc, char** argv)
{
  int opt;
  int nargs = DEFAULT_NARGS;

  init_delim_table();
  while((opt = getopt(argc, argv, "n:")) != -1) {
    switch(opt) {
      case 'n':
        nargs = atoi(optarg);
        break;
      case '?':
        fputs("Invalid option found.\n", stderr);
        exit(1);
      default:
        fputs("Option parser: default case\n", stderr);
        exit(1);
    }
  }

  if(argv[optind] == NULL) {
    fprintf(stderr, "Usage: <commands..> | %s [options..] <command> [args..]\n", argv[0]);
    exit(1);
  }
  do_xargs(stdin, nargs, argc-optind, argv+argc-optind);

  return 0;
}
