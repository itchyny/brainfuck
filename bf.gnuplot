# bf.gnuplot
# Brainfuck in gnuplot (supported by sed, wc)
if (exists('i'))\
  i = i + 1;\
else\
  i = 1;\
  code = "+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.------------.<++++++++.--------.+++.------.--------.>+.";\
  sindex(s,i) = i < 0 ? '' : i > 255 ? sindex(sslice(s,255),i-255) :\
    system('echo "'.s.'" | sed "s/.\{'.i.'\}\(.\).*/\1/"');\
  iindex(s,i) = i < 0 ? '' : i > 255 ? iindex(islice(s,255),i-255) :\
    system('echo "'.s.'" | sed "s/\([^,]*,\)\{'.i.'\}\([^,]*\).*/\2/"');\
  sslice(s,i) = i < 0 ? s : i > 255 ? sslice(sslice(s,255),i-255) : \
    system('echo "'.s.'" | sed "s/\(.\)\{0,'.i.'\}//"');\
  islice(s,i) = i < 0 ? s : i > 255 ? islice(islice(s,255),i-255) : \
    system('echo "'.s.'" | sed "s/\([^,]*,\)\{0,'.i.'\}//"');\
  take(s,i) = i < 0 ? '' : i > 255 ? (take(s,255).take(islice(s,255),i-255)) :\
    system('echo "'.s.'" | sed "s/\(\([^,]*,\)\{'.i.'\}\).*/\1/"');\
  slen(s) = int(system('echo "'.s.'" | sed "s/\(.\)/\1 /g" | wc -w'));\
  ilen(s) = int(system('echo "'.s.'" | sed "s/,*\([^,]*\),*/\1 /g" | wc -w'));\
  string(i) = sprintf('%d', i);\
  replace(s,i,r) = take(s,i).r.','.islice(s,i+1);\
  replicate(s,i) = i < 1 ? '' : i < 2 ? s : replicate(s,i/2).replicate(s,i/2).(i&1?s:'');\
  pointerlen = 10;\
  pointer = replicate('0,', pointerlen);\
  codelen = slen(code);\
  cindex = 0;\
  pindex = 0;\
  outputstr = '';\
  state = 0;\
  counter = 0

if (state == 1)\
  cindex = cindex + 1; codechr = sindex(code, cindex)
if (state == 1 && codechr eq '[')\
  counter = counter + 1; reread
if (state == 1 && codechr eq ']' && counter > 0)\
  counter = counter - 1; reread
if (state == 1 && codechr ne ']')\
  reread
if (state == 1)\
  state = 0; counter = 0;

if (state == 2)\
  cindex = cindex - 1; codechr = sindex(code, cindex)
if (state == 2 && codechr eq ']')\
  counter = counter + 1; reread
if (state == 2 && codechr eq '[' && counter > 0)\
  counter = counter - 1; reread
if (state == 2 && codechr ne '[')\
  reread
if (state == 2)\
  state = 0; counter = 0;

codechr = sindex(code, cindex)
pointerval = int(iindex(pointer, pindex))

if (codechr eq '>')\
  pindex = pindex + 1
if (codechr eq '<')\
  pindex = pindex - 1
if (codechr eq '+')\
  pointer = replace(pointer, pindex, string(pointerval + 1))
if (codechr eq '-')\
  pointer = replace(pointer, pindex, string(pointerval - 1))
if (codechr eq '[' && pointerval == 0)\
  state = 1; reread
if (codechr eq ']' && pointerval != 0)\
  state = 2; reread
if (codechr eq '.')\
  outputstr = outputstr.sprintf('%c', pointerval);

if (pindex >= pointerlen)\
  pointer = pointer . '0,';\
  pointerlen = pointerlen + 1

if (pindex < 0)\
  print 'out of memory! (negative pointer region access)';\
  exit 1

formatptr = replicate('  %3d ',pindex).' [%3d]'.replicate('  %3d ',pointerlen - pindex - 1)
eval("print sprintf('%3d  %s   ".formatptr."  %s',cindex,codechr,".pointer."outputstr)")

cindex = cindex + 1
if (cindex < codelen) reread

print outputstr
