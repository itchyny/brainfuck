#!/bin/bash
set -ueo pipefail

BF="${BF:-bf}"

testing() {
  printf 'Testing %s: ' "$1"
  if out=$("${@:2}"); then
    echo OK
  else
    echo FAIL
    echo "$out"
    exit 1
  fi
}

test_code() {
  code="$1"
  want="$2"
  testing "code ${code//$'\n'/} (want: \"$want\")" \
    diff <($BF <<<"$code" 2>&1) <(printf '%b' "$want")
}

test_exit_code() {
  code="$1"
  want="$2"
  testing "exit code ${code//$'\n'/} (want: $want)" \
    diff <(set +e; $BF <<<"$code" &>/dev/null; echo "$?") <(echo "$want")
}

test_file() {
  file="$1"
  want="$2"
  input="${3:-}"
  testing "file $file (want: \"$want\")" \
    diff <($BF "$file" <<<"$input" 2>&1) <(printf '%b' "$want")
}

test_code '.' '\x00'
test_code '+++++++++.' '\t'
test_code '+++---+++---+++.' '\x03'
test_code '++++++[->++++++++<]>.' '0'
test_code '++++++[-]+++++.' '\x05'
test_code '++++++++++++++[->--------------<]>[-<+>]<.' '<'
test_code '+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.
  ------------.<++++++++.--------.+++.------.--------.>+.' 'Hello, world!'
test_exit_code '.' 0

test_code '<' 'bf: negative address access\n'
test_code '>>><<<<' 'bf: negative address access\n'
test_code '+>+>+[<]' 'bf: negative address access\n'
test_code '+[-<+>]' 'bf: negative address access\n'
test_exit_code '<' 1

test_code '[' 'bf: unmatched [ at byte 1\n'
test_code '+++[' 'bf: unmatched [ at byte 4\n'
test_code '+[[-]+[>[-]' 'bf: unmatched [ at byte 7\n'
test_code ']' 'bf: unmatched ] at byte 1\n'
test_code '+++]' 'bf: unmatched ] at byte 4\n'
test_code '+[-]>+]>[-]' 'bf: unmatched ] at byte 7\n'
test_exit_code '[' 1
test_exit_code ']' 1

test_file /dev/null ''
test_file hello.bf 'Hello, world!'
test_file fib.bf '1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233'
if $BF <<<',' &>/dev/null; then
  test_file cat.bf 'Hello, world!\n' 'Hello, world!'
  test_file cat.bf '１２３４５\n' '１２３４５'
fi
