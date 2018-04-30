#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void do_ls(const char* dirname, int all_flag)
{
  DIR* dir;
  struct dirent* dirent;
  dir = opendir(dirname);
  while((dirent = readdir(dir)) != NULL) {
    // ignore files that begin with '.'
    if(!all_flag && dirent->d_name[0] == '.') {
      continue;
    }
    puts(dirent->d_name);
  }
  closedir(dir);
}

int main(int argc, char** argv)
{
  int opt;
  int all_flag = 0;
  while((opt = getopt(argc, argv, "a")) != -1) {
    switch(opt) {
      case 'a':
        all_flag = 1;
        break;
      case '?':
        fputs("Invalid option found.\n", stderr);
        exit(1);
      default:
        fputs("option parser: Unreachable code\n", stderr);
        exit(1);
    }
  }
  if(argv[optind] == NULL) {
    do_ls(".", all_flag);
  } else {
    for(int i = optind; i < argc; i++) {
      do_ls(argv[i], all_flag);
    }
  }
  return 0;
}
