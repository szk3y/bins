#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  char ch;
  FILE* fp;

  if(argc <= 1) {
    fp = stdin;
  } else {
    fp = fopen(argv[1], "rb");
    if(!fp) {
      perror("fopen(argv[1], \"rb\")");
      exit(EXIT_FAILURE);
    }
  }

  while(1) {
    ch = fgetc(fp);
    if(ch == EOF) {
      break;
    }
    putchar(ch);
  }

  if(fp != stdin) {
    fclose(fp);
  }
  exit(EXIT_SUCCESS);
}
