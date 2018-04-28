#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
  int new_line_flag = 1;
  int opt;

  while((opt = getopt(argc, argv, "n")) != -1) {
    switch(opt) {
      case 'n':
        new_line_flag = 0;
        break;
      case '?':
        fputs("Invalid option found.\n", stderr);
        exit(1);
      default:
        fputs("Unreachable default case\n", stderr);
        exit(1);
    }
  }
  for(int i = optind; i < argc; i++) {
    fputs(argv[i], stdout);
    if(i < argc - 1) {
      putchar(' ');
    }
  }
  if(new_line_flag) {
    putchar('\n');
  }
  return 0;
}
