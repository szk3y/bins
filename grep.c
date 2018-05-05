#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ERR_BUF_SIZE  0x1000
#define GREP_BUF_SIZE 0x1000

void do_grep(regex_t* pattern, FILE* fp)
{
  char buf[GREP_BUF_SIZE];
  while(fgets(buf, sizeof(buf), fp)) { // TODO: long line exception
    if(regexec(pattern, buf, 0, NULL, 0) == 0) {
      fputs(buf, stdout);
    }
  }
}

int main(int argc, char** argv)
{
  int err;
  char errbuf[ERR_BUF_SIZE];
  regex_t pattern;
  FILE* fp;

  if(argc <= 1) {
    fprintf(stderr, "Usage: %s <pattern> [files...]\n", argv[0]);
    exit(1);
  }

  err = regcomp(&pattern, argv[1], REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
  if(err != 0) {
    regerror(err, &pattern, errbuf, sizeof(errbuf));
    fputs(errbuf, stderr);
    exit(1);
  }
  if(argc == 2) {
    do_grep(&pattern, stdin);
  } else {
    for(int i = 2; i < argc; i++) {
      fp = fopen(argv[i], "rb");
      if(!fp) {
        perror("fopen");
        exit(1);
      }
      do_grep(&pattern, fp);
      fclose(fp);
    }
  }
  regfree(&pattern);
  return 0;
}
