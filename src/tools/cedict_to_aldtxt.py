#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Convert a CC-CEDICT file to the AlphaDict text front-end.

CEDICT entry format:
  Traditional Simplified [pin1 yin1] /English 1/equivalent 2/
  e.g.  中國 中国 [Zhong1 guo2] /China/Middle Kingdom/

The traditional form becomes an alias of the simplified head word.

Usage:
  cedict_to_aldtxt.py -i cedict.txt -o cedict-zh.txt
  cedict_to_aldtxt.py -i cedict.txt
"""

import codecs
import sys

from lib_aldict import *

usage = """Usage: cedict_to_aldtxt.py -i <cedict-file> [-o out.txt]

  -i   CC-CEDICT text file
  -o   output text file (default: <input>.txt)
"""

opts = get_opt(sys.argv, ["-i", "-o"])
if "-i" not in opts:
    sys.stderr.write(usage)
    sys.exit(2)

src = opts["-i"]
out = opts.get("-o", src + ".txt")

try:
    f_in = codecs.open(src, "r", encoding="utf-8", errors="ignore")
except IOError as e:
    sys.stderr.write("cannot open %s: %s\n" % (src, e))
    sys.exit(1)

if not aldict_openfile(out):
    f_in.close()
    sys.exit(1)

meta = {}
header_written = False
count = 0
try:
    for line in f_in:
        line = line.rstrip("\r\n")

        if line.startswith("#!"):
            key, _, value = line[2:].strip().partition("=")
            meta[key.strip()] = value.strip()
            continue
        if not line or line.startswith("#"):
            continue

        lb = line.find("[")
        rb = line.find("]", lb + 1)
        first = line.find("/", rb + 1)
        last = line.rfind("/")
        if lb < 0 or rb < 0 or first < 0 or last <= first:
            sys.stderr.write("skip malformed line: %s\n" % line)
            continue

        if not header_written:
            header_written = True
            date = meta.get("date", "")
            # CEDICT dates look like 2014-05-18T08:58:12Z
            if "T" in date:
                date = date[:date.find("T")]
            aldict_writeheader({
                "dictname": "cc-cedict",
                "publisher": meta.get("publisher", "cedict"),
                "publishdate": date,
                "dictversion": "%s.%s" % (meta.get("version", "0"),
                                          meta.get("subversion", "0")),
                "srclan": "chinese",
                "detlan": "english",
            })

        words = line[:lb].split()
        phonetic = line[lb + 1:rb].strip()
        explanation = [x.strip() for x in line[first + 1:last].split("/")]
        explanation = [x for x in explanation if x]

        if words:
            aldict_writeword(words, [[ "拼音", phonetic ]], explanation)
            count += 1
finally:
    f_in.close()
    aldict_close()

sys.stderr.write("write %s, entries(%d)\n" % (out, count))
