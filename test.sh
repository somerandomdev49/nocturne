#!/bin/sh

mkdir -p bin

./noct example.noct bin/$1.o \
&& clang bin/$1.o -o bin/$1  \
&& bin/$1
