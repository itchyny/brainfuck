#include <stdio.h>

/* char* s; */
char *s = "+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.------------.<++++++++.--------.+++.------.--------.>+.";
FILE* fp;
int depth;

void indent(void) {
  int i;
  for (i = 0; i < depth; i++) {
    fprintf(fp, "  ");
  }
}

void output(char* s) {
  indent();
  fprintf(fp, "%s", s);
}

void skip(void) {
  while (*s && *s != '+'
            && *s != '-'
            && *s != '>'
            && *s != '<'
            && *s != '['
            && *s != ']'
            && *s != '.'
            && *s != ',') s++;
}

void trim(char* target, char c) {
  int n;
  n = 0;
  do {
    s++;
    n++;
    skip();
  } while (*s == c);
  if (c == '>') c = '+';
  else if (c == '<') c = '-';
  indent();
  if (n == 1) {
    fprintf(fp, "%s%c%c;\n", target, c, c);
  } else {
    fprintf(fp, "%s %c= %d;\n", target, c, n);
  }
  s--;
}

int main(int argc, char *argv[]) {
  int i;
  /* s = argv[1]; */
  fp = stdout;
  output("#include<stdio.h>\nint p[100];\n\n");
  output("int main() {\n  int pc, i;\n  pc = 0;\n");
  depth = 1;
  while (*s) {
    switch (*s) {
      case '+': trim("p[pc]", '+'); break;
      case '-': trim("p[pc]", '-'); break;
      case '>': trim("pc", '>'); break;
      case '<': trim("pc", '<'); break;
      case '.': output("putchar(p[pc]);\n"); break;
      case ',': output("p[pc] = getchar();\n"); break;
      case '[':
        if (*(s + 1) && *(s + 1) == '-' &&
            *(s + 2) && *(s + 2) == ']') {
          output("p[pc] = 0;\n");
          s += 2;
        } else {
          output("while (p[pc]) {\n");
          depth++;
        }
        break;
      case ']': depth--; output("}\n"); break;
      default : skip();
    }
    s++;
  }
  output("printf(\"\\n\");\n}\n\n");  
  return 0;
}

