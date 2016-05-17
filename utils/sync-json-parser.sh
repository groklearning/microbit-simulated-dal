#!/bin/sh
if [ -z "$1" ]; then
  echo "Usage: $0 path/to/groklearning/app/realtime-server"
  exit 1
fi

if [ ! -d "$PWD/source/" -o $(basename $PWD) != "microbit-simulated-dal" ]; then
  echo "Run from the microbit-simulated-dal directory."
  exit 1
fi

mkdir -p source/json inc/json

FILES="json buffer lexer logging status"
for file in $FILES; do
  if [ ! -e "$1/src/$file.h" ]; then
    echo "Missing $1/src/$file.h"
    exit 1
  fi
done

for file in $FILES; do
  if [ -e "$1/src/$file.c" ]; then
    cp $1/src/$file.c source/json/$file.c
  fi
  cp $1/src/$file.h inc/json/$file.h
done

cp $1/src/xmalloc.h inc/json/xmalloc.h
