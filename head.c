#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_NLINE 10

void do_head(FILE* fp, int nline)
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

int main(int argc, char** argv)
{
  FILE* fp;
  int nline = DEFAULT_NLINE;
  int opt;

  while((opt = getopt(argc, argv, "n:")) != -1) {
    switch(opt) {
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
    do_head(stdin, nline);
    return 0;
  }
  for(int i = optind; i < argc; i++) {
    fp = fopen(argv[i], "rb");
    if(fp == NULL) {
      perror("fopen");
      exit(1);
    }
    do_head(fp, nline);
    fclose(fp);
  }
  return 0;
}
