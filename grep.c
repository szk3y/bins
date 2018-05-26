#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ERR_BUF_SIZE  0x1000
#define GREP_BUF_SIZE 0x1000
#define PASS_FILTER (int)(-1)
#define CUT_FILTER 0

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
  FILE* fp;
  char errbuf[ERR_BUF_SIZE];
  const char* patstr = NULL;
  int err;
  int opt;
  int is_case_insensitive = CUT_FILTER;
  regex_t pattern;

  while((opt = getopt(argc, argv, "ie:")) != -1) {
    switch(opt) {
      case 'i':
        is_case_insensitive = PASS_FILTER;
        break;
      case 'e':
        patstr = optarg;
        break;
      case '?':
        fputs("Invalid option found.\n", stderr);
        exit(1);
      default:
        fputs("Option parser: unreachable code.\n", stderr);
        exit(1);
    }
  }

  if(argv[optind] == NULL && patstr == NULL) {
    fprintf(stderr, "Usage: %s <pattern> [files...]\n", argv[0]);
    exit(1);
  }
  if(patstr == NULL) {
    patstr = argv[optind];
    optind++;
  }

  err = regcomp(&pattern, patstr,
      REG_EXTENDED | REG_NOSUB | REG_NEWLINE | (REG_ICASE & is_case_insensitive));
  if(err != 0) {
    regerror(err, &pattern, errbuf, sizeof(errbuf));
    fputs(errbuf, stderr);
    exit(1);
  }
  if(argc == optind) {
    do_grep(&pattern, stdin);
  } else {
    for(int i = optind; i < argc; i++) {
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
