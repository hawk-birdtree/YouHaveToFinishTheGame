#!/bin/bash
# Ensure the output directory exists
mkdir -p out

# Compile with all warnings enabled
gcc -Wall -Wextra -Wpedantic -fsanitize=address -g -o out/youHaveToFinishTheGame src/*.c -lraylib -lm -ldl -lpthread -lGL -lX11
