#!/bin/sh

TARGET="test"
TARGET2="test2"
TARGET3="test3"
F1="simpledu_res"
F2="du_res"

test() {
  ./simpledu "$@" "$TARGET" | sort -k2 >"$F1"
  du "$@" "$TARGET" | sort -k2 >"$F2"
  diff -q "$F1" "$F2" || echo "fail ops " "$@" "on target " "$TARGET"
  rm "$F1" "$F2"

  ./simpledu "$@" "$TARGET2" | sort -k2 >"$F1"
  du "$@" "$TARGET2" | sort -k2 >"$F2"
  diff -q "$F1" "$F2" || echo "fail ops " "$@" "on target " "$TARGET2"
  rm "$F1" "$F2"

  ./simpledu "$@" "$TARGET3" | sort -k2 >"$F1"
  du "$@" "$TARGET3" | sort -k2 >"$F2"
  diff -q "$F1" "$F2" || echo "fail ops " "$@" "on target " "$TARGET3"
  rm "$F1" "$F2"
}

if [ "$#" -ne 0 ]; then
  if [ -e "$1" ]; then
    TARGET="$1"
  else
    make && ./simpledu -l "$@" | sort -k2 >a
    du -l "$@" | sort -k2 >b
    diff -q a b && echo "We gud"
    rm a b
    exit 0
  fi
fi
make
echo ""

# 1
test "-la"
test "-lb"
test "-lB512"
test "-lL"
test "-lS"

# 2
test "-lab"
test "-laB512"
test "-laL"
test "-laS"
# test "-lbB512"
test "-lbL"
test "-lbS"
test "-lLB512"
test "-lSB512"
test "-lLS"

# 3
# test "-labB512"
test "-labL"
test "-labS"
test "-laLB512"
test "-laSB512"
# test "-lbLB512"
# test "-lbSB512"
test "-lbLS"
test "-lLSB512"

# 4
# test "-labLB512"
# test "-labSB512"
test "-labLS"
test "-laLSB512"
# test "-lbLSB512"

# 5
# test "-labLSB512"

# block size
test "-aB4096000000000000000"

echo ""
echo "Finished"
echo ""

