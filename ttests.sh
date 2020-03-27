#!/bin/sh

TARGET="test"
TARGET2="test2"
TARGET3="test3"
F1="simpledu_res"
F2="du_res"

test() {
  src/simpledu "$@" "$TARGET" | sort -k2 >"$F1"
  du "$@" "$TARGET" | sort -k2 >"$F2"
  diff -q "$F1" "$F2" && echo "OK" || echo "fail ops" "$@" "on target" "$TARGET"
  rm "$F1" "$F2"

  src/simpledu "$@" "$TARGET2" | sort -k2 >"$F1"
  du "$@" "$TARGET2" | sort -k2 >"$F2"
  diff -q "$F1" "$F2" && echo "OK" || echo "fail ops" "$@" "on target" "$TARGET2"
  rm "$F1" "$F2"

  src/simpledu "$@" "$TARGET3" | sort -k2 >"$F1"
  du "$@" "$TARGET3" | sort -k2 >"$F2"
  diff -q "$F1" "$F2" && echo "OK" || echo "fail ops" "$@" "on target" "$TARGET3"
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

cd src && make && cd .. || echo "Can't make." || exit 1
echo ""

# 1
test "-la"
test "-lb"
test "-lB512"
test "-lS"

# 2
test "-lab"
test "-laB512"
test "-laS"
test "-lbB512"
test "-lbS"
test "-lSB512"

# 3
test "-labB512"
test "-labS"
test "-laSB512"
test "-lbLB512"
test "-lbSB512"

# 4
test "-labLB512"
test "-labSB512"
test "-lbLSB512"

# 5
test "-labLSB512"

# big L
if [ "$2" = "-L" ]; then
  echo "Running -L tests"
  test "-lbLS"
  test "-lLSB512"
  test "-laLB512"
  test "-labL"
  test "-lLS"
  test "-lLB512"
  test "-lbL"
  test "-laL"
  test "-lL"
  test "-labLS"
  test "-laLSB512"
fi

# block size
test "-aB4096000000000000000"

# max depth
test "-la" "--max-depth" "0"
test "-lb" "--max-depth" "1"
test "-lSa" "--max-depth" "2"

cd src && make clean && cd ..
echo ""
echo "Finished"
echo ""
