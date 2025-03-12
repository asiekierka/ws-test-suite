#!/usr/bin/python3
#
# Copyright (c) 2025 Adrian Siekierka
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
# RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
# CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

from PIL import Image
import struct, sys

in_filename = sys.argv[1]
out_filename = sys.argv[2]

im = Image.open(in_filename).convert("RGBA")

with open(out_filename, "wb") as fp:
	for ic in range(0, 256):
		ic_low_x = (ic & 0xF) * 4
		ic_high_x = (ic >> 4) * 4
		for iy in range(0, 8):
			v = 0
			for ix in range(0, 4):
				pxl_low = im.getpixel((ic_low_x + 3 - ix, iy))
				pxl_high = im.getpixel((ic_high_x + 3 - ix, iy))
				if pxl_low[0] < 128:
					v = v | (1 << ix)
				if pxl_high[0] < 128:
					v = v | (16 << ix)
			fp.write(struct.pack("<B", v))
