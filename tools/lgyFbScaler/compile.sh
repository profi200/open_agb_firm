#!/bin/bash

rm ./lgyFbScaler
g++ -std=c++17 -s -flto -O2 -fstrict-aliasing -ffunction-sections -Wall -Wextra -I./lodepng -Wl,--gc-sections ./lodepng/lodepng.cpp ./lgyFbScaler.cpp -o ./lgyFbScaler