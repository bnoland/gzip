#!/bin/bash

find src/ include/ test/ -iname *.cpp -o -iname *.hpp | xargs clang-format -i
