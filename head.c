#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_NLINE 10

typedef enum {
  SELECTOR_LINE,
  SELECTOR_CHAR,
} Selector;

void do_line_head(FILE* fp, int nline)
{
  int cnt = 0;
  char ch;
  while(cnt < nline) {
    ch = fgetc(fp);
    if(ch == EOF) {
      break;
    }
    if(ch == '\n') {
      cnt++;
    }
    putchar(ch);
  }
}

void do_char_head(FILE* fp, int nchar)
{
  char ch;
  for(int i = 0; i < nchar; i++) {
    ch = fgetc(fp);
    if(ch == EOF) {
      break;
    }
    putchar(ch);
  }
}

void do_head(FILE* fp, int n, Selector selector)
{
  switch(selector) {
    case SELECTOR_LINE:
      do_line_head(fp, n);
      break;
    case SELECTOR_CHAR:
      do_char_head(fp, n);
      break;
    default:
      fprintf(stderr, "Unknown selector found: %c\n", selector);
      exit(1);
  }
}

int main(int argc, char** argv)
{
  FILE* fp;
  int nline = DEFAULT_NLINE;
  int opt;
  Selector selector = SELECTOR_LINE;

  while((opt = getopt(argc, argv, "c:n:")) != -1) {
    switch(opt) {
      case 'c':
        nline = atoi(optarg);
        selector = SELECTOR_CHAR;
        break;
      case 'n':
        nline = atoi(optarg);
        break;
      case '?':
        fputs("Invalid option found.\n", stderr);
        exit(1);
      default:
        fputs("Option parser: unreachable code\n", stderr);
        exit(1);
    }
  }

  if(argv[optind] == NULL) {
    do_head(stdin, nline, selector);
    return 0;
  }
  for(int i = optind; i < argc; i++) {
    fp = fopen(argv[i], "rb");
    if(fp == NULL) {
      perror("fopen");
      exit(1);
    }
    do_head(fp, nline, selector);
    fclose(fp);
  }
  return 0;
}
