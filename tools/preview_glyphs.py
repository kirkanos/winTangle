#!/usr/bin/env python3
"""Rasterises the output of dump_glyphs into a contact sheet.

    ./build/dump_glyphs | python3 tools/preview_glyphs.py glyphs.png

Deliberately dependency free -- it writes the PNG itself -- so reviewing the
icons needs nothing installed.
"""

import struct
import sys
import zlib

CELL, ICON, PAD, COLUMNS = 72, 48, 12, 8
BACKGROUND = (30, 30, 34)
ACCENT = (0x3A, 0x7B, 0xD5)
FRAME = (150, 150, 155)


def main() -> int:
    out = sys.argv[1] if len(sys.argv) > 1 else "glyphs.png"
    rows = [line for line in sys.stdin.read().strip().split("\n") if line]

    height_cells = (len(rows) + COLUMNS - 1) // COLUMNS
    width, height = COLUMNS * CELL, height_cells * CELL
    image = [[BACKGROUND for _ in range(width)] for _ in range(height)]

    def fill(ox, oy, x, y, w, h, colour):
        left, top = ox + round(x * ICON), oy + round(y * ICON)
        right = ox + max(round((x + w) * ICON), round(x * ICON) + 1)
        bottom = oy + max(round((y + h) * ICON), round(y * ICON) + 1)
        for py in range(top, bottom):
            for px in range(left, right):
                if 0 <= py < height and 0 <= px < width:
                    image[py][px] = colour

    def stroke(ox, oy, x, y, w, h, colour, thickness=2):
        t = thickness / ICON
        fill(ox, oy, x, y, w, t, colour)
        fill(ox, oy, x, y + h - t, w, t, colour)
        fill(ox, oy, x, y, t, h, colour)
        fill(ox, oy, x + w - t, y, t, h, colour)

    for index, row in enumerate(rows):
        parts = row.split("|")
        ox, oy = (index % COLUMNS) * CELL + PAD, (index // COLUMNS) * CELL + PAD
        if parts[1] == "1":
            stroke(ox, oy, 0, 0, 1, 1, FRAME)
        # Outlines first, so a filled shape on top stays readable.
        for shape in parts[2:]:
            kind, *values = shape.split()
            if kind == "O":
                stroke(ox, oy, *map(float, values), FRAME)
        for shape in parts[2:]:
            kind, *values = shape.split()
            if kind == "F":
                fill(ox, oy, *map(float, values), ACCENT)

    raw = b"".join(b"\x00" + b"".join(bytes(px) for px in line) for line in image)

    def chunk(tag, data):
        body = tag + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))

    png = (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(raw))
        + chunk(b"IEND", b"")
    )
    with open(out, "wb") as handle:
        handle.write(png)

    print(f"{out}: {len(rows)} glyphs, {COLUMNS} per row")
    return 0


if __name__ == "__main__":
    sys.exit(main())
