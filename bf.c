#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char p[10005];
char hello[256] = "+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.------------.<++++++++.--------.+++.------.--------.>+.";
char buffer[100000];
char buf[256];
int cache[100005];
FILE* fp;

int run(char c[], char* p) {
  char *pstart, *pend, *cstart;
  int num; long pos;
  pstart = p;
  pend = p + 10000;
  cstart = c;
  while (*c) {
    switch (*c) {
      case '+': (*p)++; break;
      case '-': (*p)--; break;
      /*case '+': pos = c - cstart;
                if (cache[pos]) {
                  c += cache[pos] - 1;
                } else {
                  do {
                    c++;
                    cache[pos]++;
                  } while (*c == '+');
                  c--;
                }
                (*p) += cache[pos];
                break;
      case '-': pos = c - cstart;
                if (cache[pos]) {
                  c += cache[pos] - 1;
                } else {
                  do {
                    c++;
                    cache[pos]++;
                  } while (*c == '-');
                  c--;
                }
                (*p) -= cache[pos];
                break;*/
      /*case '>': p++; break;
      case '<': p--; break;*/
      case '>': pos = c - cstart;
                if (cache[pos]) {
                  c += cache[pos] - 1;
                } else {
                  do {
                    c++;
                    cache[pos]++;
                  } while (*c == '>');
                  c--;
                }
                p += cache[pos];
                if (p > pend) {
                  fprintf(stderr, "error: out of memory");
                  exit(1);
                };
                break;
      case '<': pos = c - cstart;
                if (cache[pos]) {
                  c += cache[pos] - 1;
                } else {
                  do {
                    c++;
                    cache[pos]++;
                  } while (*c == '<');
                  c--;
                }
                p -= cache[pos];
                if (p < pstart) {
                  fprintf(stderr, "error: negative address access");
                  exit(1);
                };
                break;
      case '.': putchar(*p); break;
      case ',': *p = getchar(); break;
      case '[':
        if (*p == 0) {
          pos = c - cstart;
          if (cache[pos]) {
            c += cache[pos];
          } else if (c[1] == '-' && c[2] == ']') {
            c++; c++; *p = 0;
            cache[c - cstart] = cache[pos] = 2;
          } else {
            num = 1;
            while (num > 0) {
              c++;
              if (*c == '[') num++;
              else if (*c == ']') num--;
            }
            cache[c - cstart] = cache[pos] = c - cstart - pos;
          }
        }
        break;
      case ']':
        if (*p != 0) {
          pos = c - cstart;
          if (cache[pos]) {
            c -= cache[pos];
          } else {
            num = 1;
            while (num > 0) {
              c--;
              if (*c == ']') num++;
              else if (*c == '[') num--;
            }
            cache[c - cstart] = cache[pos] = - (c - cstart - pos);
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

