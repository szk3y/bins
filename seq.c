#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bool equal_width_flag = false;

void print_usage(char** argv)
{
  fprintf(stderr, "Usage: %s <last>\n", argv[0]);
  fprintf(stderr, "          or\n");
  fprintf(stderr, "       %s <first> <last>\n", argv[0]);
  fprintf(stderr, "          or\n");
  fprintf(stderr, "       %s <first> <step> <last>\n", argv[0]);
}

void do_seq(int first, int step, int last)
{
  // do nothing if range is not closed
  if((last <= first && 0 <= step) || (first <= last && step <= 0)) {
    return;
  }
  for(int i = first; (i <= last && 0 < step) || (last <= i && step < 0); i += step) {
    printf("%d\n", i);
  }
}

int main(int argc, char** argv)
{
  int opt;
  int first, last, step;
  while((opt = getopt(argc, argv, "w")) != -1) {
    switch(opt) {
      case 'w':
        equal_width_flag = true;
        break;
      case '?':
        fputs("Invalid option\n", stderr);
        exit(1);
      default:
        fputs("getopt: defeault case\n", stderr);
        exit(1);
    }
  }

  if(optind == argc) {
    print_usage(argv);
    exit(1);
  } else if(optind + 1 == argc) {
    first = 1;
    step  = 1;
    last  = atoi(argv[optind]);
  } else if(optind + 2 == argc) {
    first = atoi(argv[optind]);
    step  = 1;
    last  = atoi(argv[optind+1]);
  } else if(optind + 3 == argc) {
    first = atoi(argv[optind]);
    step  = atoi(argv[optind+1]);
    last  = atoi(argv[optind+2]);
  } else {
    fputs("Too many command line arguments\n", stderr);
    print_usage(argv);
    exit(1);
  }
  do_seq(first, step, last);

  return 0;
}
