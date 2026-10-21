#!/bin/bash
if [ -z "$1" ]; then
  echo "Error: Please provide an argument."
  exit 1
fi
mkdir -p ./bin
mkdir -p ./asm
mkdir -p ./dsm

file=$1
filename_no_ext=$(basename "$file" .asm)

if [ -e "dsm/$filename_no_ext.txt" ]; then
  rm "dsm/$filename_no_ext.txt" 
fi

if [ -e "asm/$filename_no_ext.txt" ]; then
  rm "asm/$filename_no_ext.txt" 
fi

./ppu_assembler.py -a -i "$file" -o "bin/$filename_no_ext.bin"
./ppu_assembler.py -p -i "$file" -o "asm/$filename_no_ext.asm"
./ppu_assembler.py -d -i "bin/$filename_no_ext.bin" >> "dsm/$filename_no_ext.txt"
