#!/usr/bin/sed
# sed -f bf2c.sed hello.bf > a.c && gcc a.c && ./a.out

1i\
#include <stdio.h>\
int p[100];\
int main() {\
  int pc = 0;


s/\]/  }\
/g
s/\[/  while(p[pc] != 0) {\
/g
s/+/  ++p[pc];\
/g
s/-/  --p[pc];\
/g
s/>/  ++pc;\
/g
s/</  --pc;\
/g
s/\./  putchar(p[pc]);\
/g
s/\,/  p[pc] = getchar();\
/g

a\
  return 0;\
}

