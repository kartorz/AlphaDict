#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Convert a StarDict dictionary to the AlphaDict text front-end.

StarDict stores a dictionary as three files sharing one base name:
  .ifo  metadata, one "key=value" per line
  .idx  word list: word\\0 offset size, sorted
  .dict the articles

Usage:
  stardict_to_aldtxt.py -i langdao-ec-gb -o langdao-ec-gb.txt
  stardict_to_aldtxt.py -i langdao-ec-gb
"""

import codecs
import struct
import sys

from lib_aldict import *

usage = """Usage: stardict_to_aldtxt.py -i <dict-base> [-o out.txt]

  -i   dictionary base name, e.g. ~/dicts/langdao-ec-gb for
       langdao-ec-gb.ifo/.idx/.dict
  -o   output text file (default: <dict-base>.txt)
"""

opts = get_opt(sys.argv, ["-i", "-o", "-p"])
if "-i" not in opts:
    sys.stderr.write(usage)
    sys.exit(2)

base = opts["-i"]
out = opts.get("-o", base + ".txt")

try:
    f_ifo = codecs.open(base + ".ifo", "r", encoding="utf-8", errors="ignore")
    f_dict = open(base + ".dict", "rb")
    f_idx = open(base + ".idx", "rb")
except IOError as e:
    sys.stderr.write("cannot open %s[.ifo|.idx|.dict]: %s\n" % (base, e))
    sys.exit(1)

ifo = {}
for line in f_ifo:
    pos = line.find("=")
    if pos != -1:
        ifo[line[:pos]] = line[pos + 1:].rstrip("\r\n")
f_ifo.close()


def _two_part(version):
    """StarDict versions may be 2.4.2, the format wants major.minor."""
    parts = [p for p in str(version).split(".") if p]
    while len(parts) < 2:
        parts.append("0")
    return "%s.%s" % (parts[0], parts[1])


header = {
    "dictname": ifo.get("bookname", ""),
    "publisher": ifo.get("author", "stardict"),
    "publishdate": ifo.get("date", "").replace(".", "-"),
    # "2.4.2" would be truncated by build, so keep major.minor here
    "dictversion": _two_part(ifo.get("version", "0.0")),
    "srclan": "any",
    "detlan": "any",
}

# 64-bit offset is signalled explicitly; the size field stays 32-bit.
if ifo.get("idxoffsetbits") == "64":
    off_fmt = ">Q"
else:
    off_fmt = ">L"
off_len = struct.calcsize(off_fmt)
size_fmt = ">L"
size_len = 4

if not aldict_openfile(out):
    sys.exit(1)

aldict_writeheader(header)

data = f_idx.read()
count = 0
while data:
    end = data.find(b"\0")
    if end < 0:
        sys.stderr.write("bad .idx: no NUL terminator\n")
        break
    if end == 0:  # skip padding
        data = data[1:]
        continue
    if len(data) < end + 1 + off_len + size_len:
        sys.stderr.write("bad .idx: truncated record\n")
        break

    word = data[:end].decode("utf-8", "ignore")
    pos = end + 1
    offset = struct.unpack(off_fmt, data[pos:pos + off_len])[0]
    pos += off_len
    size = struct.unpack(size_fmt, data[pos:pos + size_len])[0]
    data = data[pos + size_len:]

    f_dict.seek(offset)
    explanation = f_dict.read(size).decode("utf-8", "ignore")
    aldict_writeword([word], None, [explanation])

    count += 1
    if count % 1000 == 0:
        sys.stderr.write("\r%d entries" % count)

f_idx.close()
f_dict.close()
aldict_close()
sys.stderr.write("\nwrite %s, entries(%d)\n" % (out, count))
