#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void do_ls(const char* dirname)
{
  DIR* dir;
  struct dirent* dirent;
  dir = opendir(dirname);
  while((dirent = readdir(dir)) != NULL) {
    puts(dirent->d_name);
  }
  closedir(dir);
}

int main(int argc, char** argv)
{
  if(argc == 1) {
    do_ls(".");
  } else {
    for(int i = 0; i < argc; i++) {
      do_ls(argv[i]);
    }
  }
}
