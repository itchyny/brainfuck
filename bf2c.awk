#! /usr/bin/awk -f
# awk -f bf2c.awk hello.bf > a.c && gcc a.c && ./a.out

BEGIN {
  print "#include <stdio.h>";
  print "int p[100];";
  print "int main() {";
  print "  int pc = 0;";
}

{
  #Note: the order in which these are
  #substituted is very important.
  gsub(/\]/, "  }\n");
  gsub(/\[/, "  while(p[pc] != 0) {\n");
  gsub(/\+/, "  ++p[pc];\n");
  gsub(/\-/, "  --p[pc];\n");
  gsub(/>/, "  ++pc;\n");
  gsub(/</, "  --pc;\n");
  gsub(/\./, "  putchar(p[pc]);\n");
  gsub(/\,/, "  p[pc] = getchar();\n");
  print $0
}

END {
  print "  return 0;";
  print "}";
}

