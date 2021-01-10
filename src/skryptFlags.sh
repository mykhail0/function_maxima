#!/bin/bash

# 601
#to_pass=(101 102 103 104 105 106 201 202 203 204 205 206 207 208 209 210 211
# 301 401 402 403 404 405 406 407 701)
#to_fail=(501)
to_pass=(101)

compiler_flags="clang -Wfatal-errors -Wall -Wextra -pedantic -std=c++17 -Og -g
-Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=address -fsanitize=undefined -fstack-protector "
RED='\033[0;31m'
GREEN='\033[0;32m'
NOCOLOR='\033[0m'

function assert_test_passes() {
  code="$1"

  if $compiler_flags -DTEST_NUM=$code test_damiana.cc -o test_damiana_compiled ; then
#  if $compiler_flags -DTEST_NUM=$code test_damiana.cc -o test_damiana_compiled >/dev/null 2>/dev/null; then
    if ./test_damiana_compiled >/dev/null 2>/dev/null; then
      echo -ne "${GREEN}[${code}] success: exit code 0${NOCOLOR}\n"
    else
      echo -ne "${RED}[${code}] failed: nonzero code returned${NOCOLOR}\n"
    fi
  else
    echo -ne "${RED}[${code}] error: compilation failed${NOCOLOR}\n"
  fi
}

function assert_test_fails() {
  code="$1"

  if $compiler_flags -DTEST_NUM=$code test_damiana.cc -o test_damiana_compiled 2>/dev/null; then
    echo -ne "${RED}[${code}] failed: compilation should fail but did not${NOCOLOR}\n"
  else
    echo -ne "${GREEN}[${code}] success: compilation failed${NOCOLOR}\n"
  fi
}

for i in "${to_pass[@]}"; do
  assert_test_passes $i
done
for i in "${to_fail[@]}"; do
  assert_test_fails $i
done

