#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Convert an IDP dictionary file to the AlphaDict text front-end.

IDP files are tab-separated pairs, one per line:
  english-word<TAB>other-language-word
  dog	perro

Usage:
  idp_to_aldtxt.py -i idp-french.txt -o idp-french-aldict.txt -n french
  idp_to_aldtxt.py -i idp-french.txt
"""

import codecs
import os
import sys

from lib_aldict import *

usage = """Usage: idp_to_aldtxt.py -i <idp-file> [-o out.txt] [-n name]

  -i   IDP dictionary file
  -o   output text file (default: <input>.txt)
  -n   dictionary name (default: input file base name)
"""

opts = get_opt(sys.argv, ["-i", "-o", "-n"])
if "-i" not in opts:
    sys.stderr.write(usage)
    sys.exit(2)

src = opts["-i"]
out = opts.get("-o", src + ".txt")
name = opts.get("-n", os.path.splitext(os.path.basename(src))[0])

try:
    # IDP files are usually latin-1; utf-8 with replacement keeps them readable
    f_in = codecs.open(src, "r", encoding="utf-8", errors="replace")
except IOError as e:
    sys.stderr.write("cannot open %s: %s\n" % (src, e))
    sys.exit(1)

if not aldict_openfile(out):
    f_in.close()
    sys.exit(1)

aldict_writeheader({
    "dictname": name,
    "publisher": "IDP",
    "publishdate": "2007-02-01",
    "dictversion": "0.1",
    "srclan": "english",
    "detlan": "any",
})

count = 0
try:
    for line in f_in:
        line = line.rstrip("\r\n")
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) < 2:
            continue
        word, meaning = parts[0].strip(), parts[1].strip()
        if word and meaning:
            aldict_writeword([word], None, [meaning])
            count += 1
finally:
    f_in.close()
    aldict_close()

sys.stderr.write("write %s, entries(%d)\n" % (out, count))
