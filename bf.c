#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CODESIZE 3000005
#define MEMORYSIZE 100005
#define LOOPMAX 10000000000
char p[MEMORYSIZE];
char buffer[CODESIZE];
int cache[CODESIZE];
char hello[256] = "+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.------------.<++++++++.--------.+++.------.--------.>+.";
char buf[256];
char* filename;
FILE* fp;
char errorinf[50] = "bf:%s:%d:%d: error: infinite loop\n";
char errorneg[50] = "bf:%s:%d:%d: error: negative address access\n";
char errorout[50] = "bf:%s:%d:%d: error: out of memory\n";

int run(char c[], char* p) {
  char *pstart, *pend, *cstart, *err;
  int num, linex, liney;
  long pos, i;
  long long m;
  linex = liney = 0;
  m = 0;
  pstart = p;
  pend = p + MEMORYSIZE - 5;
  cstart = c;
  while (*c) {
    switch (*c) {
      case '+': (*p)++; break;
      case '-': (*p)--; break;
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
                  err = errorout;
                  goto ERR;
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
                  err = errorneg;
                  goto ERR;
                };
                break;
      case '.': putchar(*p);
                fflush(stdout);
                break;
      case ',': *p = getchar();
                break;
      case '[': if (*p == 0) {
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
      case ']': if (*p != 0) {
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
    m++;
    if (m > LOOPMAX) {
      err = errorinf;
      goto ERR;
    }
  }
  return 0;
ERR:
  linex = 1;
  liney = 1;
  for (i = 0; i < c - cstart; i++) {
    if (cstart[i] == '\n') {
      linex = 1;
      liney++;
    } else {
      linex++;
    }
  }
  fprintf(stderr, err, filename, liney, linex);
  exit(1);
}

int main(int argc, char *argv[]) {
  long long codemax;
  codemax = (CODESIZE - 5) / 256;
  if (argc < 2) {
    run(hello, p);
  } else {
    if ((fp = fopen(argv[1], "r")) == NULL) {
      filename = "argv[1]";
      run(argv[1], p);
      return 0;
    }
    filename = argv[1];
    while (fgets(buf, 255, fp) != NULL) {
      if (--codemax == 0) {
        fprintf(stderr, "bf:error: too long source code\n");
        exit(1);
      }
      strcat(buffer, buf);
    }
    fclose(fp);
    run(buffer, p);
  }
  return 0;
}

