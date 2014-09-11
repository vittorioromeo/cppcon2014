#!/bin/bash

# Simple "compile&run" script that links SFML

clang++ -std=c++1y \
		-lsfml-system -lsfml-window -lsfml-graphics \
		"${@:2}" ./$1 -o /tmp/$1.temp && /tmp/$1.temp