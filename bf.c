#include <stdio.h>
#include <string.h>
int p[128];
char hello[256] = "+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.------------.<++++++++.--------.+++.------.--------.>+.";
char buffer[10000];
char buf[256];
FILE* fp;

int run(char c[], int* p) {
  int num;
  while (*c) {
    switch(*c) {
      case '+': (*p)++; break;
      case '-': (*p)--; break;
      case '>': p++; break;
      case '<': p--; break;
      case '.': putchar(*p); break;
      case '[':
        if (*p == 0) {
          num = 1;
          while (num > 0) {
            c++;
            if (*c == '[') num++;
            else if (*c == ']') num--;
          }
        }
        break;
      case ']':
        if (*p != 0) {
          num = 1;
          while (num > 0) {
            c--;
            if (*c == ']') num++;
            else if (*c == '[') num--;
          }
        }
        break;
      default: break;
    }
    c++;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    run(hello, p);
  } else {
    if ((fp = fopen(argv[1], "r")) == NULL) {
      run(argv[1], p);
      return 0;
    }
    while (fgets(buf, 255, fp) != NULL) {
      strcat(buffer, buf);
    }
    fclose(fp);
    run(buffer, p);
  }
  return 0;
}

