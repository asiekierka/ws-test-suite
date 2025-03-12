#!/bin/sh
python3 tools/font2raw.py graphics/font_ascii.png 8 8 a resources/font_ascii.bin
python3 tools/hexdigits2font.py graphics/hex_digits.png resources/hex_00_ff.bin
