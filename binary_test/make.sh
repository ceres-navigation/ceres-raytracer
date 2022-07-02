#!/bin/bash

g++ -Wall -std=c++17 -I ../lib/ -I/home/crgnam/zstd/lib/ crtb_file.cpp -o run -L/home/crgnam/zstd/lib -l:libzstd.a

