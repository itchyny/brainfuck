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
const int bf_move = 7;  // [->+<]
const int offset = 3;
const int mask = (1 << offset) - 1;

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
        int l = codes.size();
        if (jmps.back() == l-2 && codes[l-1] == (bf_incr | -(1 << offset))) {
          codes.resize(jmps.back());
          codes.push_back(bf_setz);
        } else if (jmps.back() == l-5 &&
            codes[l-4] == (bf_incr |-(1 << offset)) && (codes[l-3]&mask) == bf_next &&
            codes[l-2] == (bf_incr |1 << offset) && (codes[l-1]&mask) == bf_next &&
            (codes[l-3] >> offset) + (codes[l-1] >> offset) == 0) {
          codes.resize(jmps.back());
          codes.push_back(bf_move | codes[l-3]&~mask);
        } else {
          codes[jmps.back()] |= l << offset;
          codes.push_back(bf_jmpnz | jmps.back() << offset);
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
        while (pointer >= memory.size())
          memory.push_back(0);
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
      case bf_move:
        int dest = pointer + (c >> offset);
        if (dest < 0)
          throw std::runtime_error("negative address access");
        while (dest >= memory.size())
          memory.push_back(0);
        memory[dest] += memory[pointer];
        memory[pointer] = 0;
        break;
    }
  }
}
