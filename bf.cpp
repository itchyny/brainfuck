#include <cstdio>
#include <format>
#include <fstream>
#include <iostream>
#include <istream>
#include <stdexcept>
#include <string>
#include <vector>

const std::string name = "bf";

const std::string read_all(std::istream &in);
const std::string read_file(const char* name);
const std::vector<int> parse(const std::string source);
void add(std::vector<int>& codes, const int code, const int diff);
bool is_mults(const std::vector<int>& codes, const int start);
void execute(const std::vector<int> codes);

int main(const int argc, const char* argv[])
{
  try {
    execute(parse(argc <= 1 ? read_all(std::cin) : read_file(argv[1])));
  } catch (const std::exception& err) {
    std::cerr << name << ": " << err.what() << std::endl;
    return 1;
  }
  return 0;
}

const std::string read_all(std::istream &in)
{
  std::string str(
      (std::istreambuf_iterator<char>(in)),
      std::istreambuf_iterator<char>());
  return str;
}

const std::string read_file(const char* name)
{
  std::ifstream file(name, std::ios::in | std::ios::binary);
  if (!file)
    throw std::runtime_error("failed to open " +
        std::string(name) + ": " +
        std::string(std::strerror(errno)));
  return read_all(file);
}

const int bf_incr = 0;  // + -
const int bf_next = 1;  // > <
const int bf_putc = 2;  // .
const int bf_getc = 3;  // ,
const int bf_jmpz = 4;  // [
const int bf_jmpnz = 5; // ]
const int bf_setz = 6;  // [-]
const int bf_mult = 7;  // [->+>++<<]
const int offset = 3;
const int mask = (1 << offset) - 1;
const int offset_mult = offset + 8;

const std::string format(const char* fmt, const int i) // TODO: use std::format
{
  std::string str(fmt);
  str.replace(str.find("{}"), 2, std::to_string(i));
  return str;
}

const std::vector<int> parse(const std::string source)
{
  std::vector<int> codes, jmps;
  for (int i = 0; i < source.size(); i++) {
    switch (source[i]) {
      case '+':
        add(codes, bf_incr, 1);
        break;
      case '-':
        add(codes, bf_incr, -1);
        break;
      case '>':
        add(codes, bf_next, 1);
        break;
      case '<':
        add(codes, bf_next, -1);
        break;
      case '.':
        codes.push_back(bf_putc);
        break;
      case ',':
        codes.push_back(bf_getc);
        break;
      case '[':
        jmps.push_back(codes.size());
        codes.push_back(bf_jmpz);
        break;
      case ']':
        if (jmps.size() == 0)
          throw std::runtime_error(format("unmatched ] at byte {}", i + 1));
        int j = jmps.back(), l = codes.size();
        if (j+2 == l && codes[j+1] == (bf_incr | -(1 << offset))) {
          codes[j] = bf_setz;
          codes.resize(j+1);
        } else if (j+2 < l && (l-j)%2 == 1 && is_mults(codes, j)) {
          for (int k = j+2, p = 0; k < l-1; j += 1, k += 2) {
            codes[j] = bf_mult | (p += codes[k] >> offset) << offset_mult
              | int(uint8_t(codes[k+1] >> offset)) << offset;
          }
          codes[j] = bf_setz;
          codes.resize(j+1);
        } else {
          codes[j] |= l << offset;
          codes.push_back(bf_jmpnz | j << offset);
        }
        jmps.pop_back();
        break;
    }
  }
  if (!jmps.empty()) {
    for (int i = source.size() - 1, depth = 0; i >= 0; i--) {
      switch (source[i]) {
        case '[':
          if (--depth < 0)
            throw std::runtime_error(format("unmatched [ at byte {}", i + 1));
          break;
        case ']':
          depth++;
          break;
      }
    }
  }
  return codes;
}

void add(std::vector<int>& codes, const int code, const int diff)
{
  if (codes.empty() || (codes.back() & mask) != code)
    codes.push_back(code | diff << offset);
  else
    codes.back() += diff << offset;
}

bool is_mults(const std::vector<int>& codes, const int start) {
  int p = 0;
  for (int i = 0, k = start+1; k < codes.size(); i += 1, k += 2) {
    if ((codes[k] & mask) == bf_incr &&
        (i > 0 || (codes[k] >> offset) == -1) &&
        (codes[k+1] & mask) == bf_next) {
      p += codes[k+1] >> offset;
    } else {
      return false;
    }
  }
  return p == 0;
}

void execute(const std::vector<int> codes)
{
  std::vector<uint8_t> memory(1, 0);
  int pointer = 0;
  for (int i = 0; i < codes.size(); i++) {
    int c = codes[i];
    switch (codes[i] & mask) {
      case bf_incr:
        memory[pointer] += c >> offset;
        break;
      case bf_next:
        pointer += c >> offset;
        if (pointer < 0)
          throw std::runtime_error("negative address access");
        if (pointer >= memory.size())
          memory.resize(pointer + 1);
        break;
      case bf_putc:
        if (std::putchar(memory[pointer]) == EOF)
          throw std::runtime_error("putchar failed");
        break;
      case bf_getc:
        int ch;
        if ((ch = std::getchar()) == EOF) {
          if (!feof(stdin))
            throw std::runtime_error("getchar failed");
          return;
        }
        memory[pointer] = ch;
        break;
      case bf_jmpz:
        if (memory[pointer] == 0)
          i = c >> offset;
        break;
      case bf_jmpnz:
        if (memory[pointer] != 0)
          i = c >> offset;
        break;
      case bf_setz:
        memory[pointer] = 0;
        break;
      case bf_mult:
        if (uint8_t v = memory[pointer]; v != 0) {
          int dest = pointer + (c >> offset_mult);
          if (dest < 0)
            throw std::runtime_error("negative address access");
          if (dest >= memory.size())
            memory.resize(dest + 1);
          memory[dest] += v * uint8_t(c >> offset);
        }
        break;
    }
  }
}
