#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
  MODE_ESCAPE,
  MODE_HEX,
  MODE_OCT,
  MODE_NORMAL,
  MODE_IGNORE,
} Mode;

int is_hex_digit(char ch)
{
  return ('0' <= ch && ch <= '9') ||
         ('A' <= ch && ch <= 'F') ||
         ('a' <= ch && ch <= 'f');
}

unsigned char to_hex(char ch)
{
  if('0' <= ch && ch <= '9') {
    return ch - '0';
  }
  if('A' <= ch && ch <= 'F') {
    return ch - 'A' + 10;
  }
  if('a' <= ch && ch <= 'f') {
    return ch - 'a' + 10;
  }
  fprintf(stderr, "Not a hex char: %c\n", ch);
  exit(1);
}

void echo_escape(const char* str)
{
  char ch;
  unsigned char hex_buf = 0;
  int hex_cnt = 0;
  Mode mode = MODE_NORMAL;

  for(const char* ptr = str; *ptr != 0; ptr++) {
    ch = *ptr;
    switch(mode) {
      case MODE_ESCAPE:
        switch(ch) {
          case '0':
            mode = MODE_OCT;
            break;
          case '\\':
            putchar(ch);
            mode = MODE_NORMAL;
            break;
          case 'a':
            putchar('\a');
            mode = MODE_NORMAL;
            break;
          case 'b':
            putchar('\b');
            mode = MODE_NORMAL;
            break;
          case 'c':
            fclose(stdout);
            mode = MODE_NORMAL;
            break;
          case 'e':
            mode = MODE_IGNORE;
            break;
          case 'f':
            putchar('\f');
            mode = MODE_NORMAL;
            break;
          case 'n':
            putchar('\n');
            mode = MODE_NORMAL;
            break;
          case 'r':
            putchar('\r');
            mode = MODE_NORMAL;
            break;
          case 't':
            putchar('\t');
            mode = MODE_NORMAL;
            break;
          case 'v':
            putchar('\v');
            mode = MODE_NORMAL;
            break;
          case 'x':
            mode = MODE_HEX;
            break;
          default:
            putchar('\\');
            putchar(ch);
            mode = MODE_NORMAL;
            break;
        }
        assert(mode != MODE_ESCAPE);
        break;
      case MODE_HEX:
        if(is_hex_digit(ch)) {
          hex_cnt++;
          hex_buf = hex_buf * 16 + to_hex(ch);
          if(hex_cnt == 2) { // flush
            putchar(hex_buf);
            hex_buf = 0;
            mode = MODE_NORMAL;
          }
        } else if(ch == '\\') {
          putchar(hex_buf);
          hex_buf = 0;
          mode = MODE_ESCAPE;
        } else {
          putchar(hex_buf);
          hex_buf = 0;
          mode = MODE_NORMAL;
        }
        break;
      case MODE_OCT: // TODO: implement this
        fputs("This mode is not implemented. Sorry ;)\n", stderr);
        exit(1);
        break;
      case MODE_NORMAL:
        switch(ch) {
          case '\\':
            mode = MODE_ESCAPE;
            break;
          default:
            putchar(ch);
            break;
        }
        break;
      case MODE_IGNORE: // TODO: implement this
        fputs("This mode is not implemented. Sorry ;)\n", stderr);
        exit(1);
        break;
      default:
        fprintf(stderr, "Unknown mode: %c\n", mode);
        exit(1);
    }
  }

  // flush unused tokens
  switch(mode) {
    case MODE_ESCAPE:
      putchar('\\');
      break;
    case MODE_HEX:
      putchar('\\');
      putchar('x');
      break;
    case MODE_OCT: // TODO
      fputs("This mode is not implemented. Sorry ;)\n", stderr);
      exit(1);
      break;
    case MODE_IGNORE: // TODO
      fputs("This mode is not implemented. Sorry ;)\n", stderr);
      exit(1);
      break;
    case MODE_NORMAL:
      break;
    default:
      fprintf(stderr, "Unknown mode: %c\n", mode);
      exit(1);
  }
}

int main(int argc, char** argv)
{
  int new_line_flag = 1;
  int escape_flag = 0;
  int opt;

  while((opt = getopt(argc, argv, "Een")) != -1) {
    switch(opt) {
      case 'E':
        escape_flag = 0;
        break;
      case 'e':
        escape_flag = 1;
        break;
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
    if(escape_flag) {
      echo_escape(argv[i]);
    } else {
      fputs(argv[i], stdout);
    }
    if(i < argc - 1) {
      putchar(' ');
    }
  }
  if(new_line_flag) {
    putchar('\n');
  }
  return 0;
}
