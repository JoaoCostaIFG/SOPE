#!/bin/sh

DFLT_TEST_DIR="test"

if [ "$#" -eq 0 ]; then
  make && ./simpledu -l -a -S -L -b "$DFLT_TEST_DIR" | sort -k2 >a
  du -l -a -S -L -b "$DFLT_TEST_DIR" | sort -k2 >b
else
  make && ./simpledu -l "$@" | sort -k2 >a
  du -l "$@" | sort -k2 >b
fi

diff -q a b

rm a b
