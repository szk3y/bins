#include <stdio.h>
#include <stdlib.h>

void do_cat(FILE* fp)
{
  char ch;
  while(1) {
    ch = fgetc(fp);
    if(ch == EOF) {
      break;
    }
    putchar(ch);
  }
}

int main(int argc, char** argv)
{
  FILE* fp;

  if(argc <= 1) {
    do_cat(stdin);
    return 0;
  }
  for(int i = 1; i < argc; i++) {
    fp = fopen(argv[i], "rb");
    if(fp == NULL) {
      perror("fopen");
      exit(1);
    }
    do_cat(fp);
    fclose(fp);
  }
  return 0;
}
