#!/bin/bash
mkdir -p ./bin
mkdir -p ./dsm
for file in src/*
do
  ./assembler.sh $file
done