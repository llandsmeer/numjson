#!/usr/bin/env bash

afl-c++ \
    -O3 \
    numjson.cpp \
    -std=c++20 \
    -DSTANDALONE \
    -lpython3.10 \
    $(python3-config --ldflags) \
    $(python3 -m pybind11 --includes) \
    -o numjson-afl

echo run
echo afl-fuzz -i seeds_dir -o /tmp/numjson-afl -- ./numjson-afl
