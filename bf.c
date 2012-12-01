#include <stdio.h>

int run(char* c, int* p) {
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
          num = 0;
          while (1) {
            c++;
            if (*c == '[') {
              num++;
            } else if (*c == ']') {
              if (num == 0) {
                break;
              }
              num--;
            }
          }
        }
        break;
      case ']':
        if (*p != 0) {
          num = 0;
          while (1) {
            c--;
            if (*c == ']') {
              num++;
            } else if (*c == '[') {
              if (num == 0) {
                break;
              }
              num--;
            }
          }
        }
        break;
      default: /*do nothing*/
        break;
    }
    c++;
  }
  return 1;
}
int p[128];
char *input = "+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.------------.<++++++++.--------.+++.------.--------.>+.";
int main(void) {
  run(input, p);
}

