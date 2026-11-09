#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Convert a dicts.info bilingual file to the AlphaDict text front-end.

dicts.info rows are tab-separated:
  word1 ; word2<TAB>translation [phonetic]<TAB>category
  April  ; Apr	四月 [sì yuè]	time

The first column may list synonyms separated by ';'.  A bracketed part,
e.g. [sì yuè], is taken as the phonetic of the first word.

Usage:
  dictsinfo_to_aldtxt.py -i foo.txt -o foo-aldict.txt
  dictsinfo_to_aldtxt.py -i foo.txt -p 拼音
"""

import codecs
import sys

from lib_aldict import *

usage = """Usage: dictsinfo_to_aldtxt.py -i <dicts.info-file> [-o out.txt] [-p label]

  -i   dicts.info bilingual dictionary file
  -o   output text file (default: <input>.txt)
  -p   phonetic label written after the phonetic (default: none)
"""

opts = get_opt(sys.argv, ["-i", "-o", "-p"])
if "-i" not in opts:
    sys.stderr.write(usage)
    sys.exit(2)

src = opts["-i"]
out = opts.get("-o", src + ".txt")
label = opts.get("-p", "")

try:
    f_in = codecs.open(src, "r", encoding="utf-8", errors="replace")
except IOError as e:
    sys.stderr.write("cannot open %s: %s\n" % (src, e))
    sys.exit(1)

if not aldict_openfile(out):
    f_in.close()
    sys.exit(1)

aldict_writeheader({
    "dictname": "dicts.info bilingual",
    "publisher": "www.dicts.info",
    "publishdate": "2014-01-01",
    "dictversion": "0.1",
    "srclan": "any",
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

        words = [w.strip() for w in parts[0].split(";") if w.strip()]
        if not words:
            continue

        phonetics = []
        words[0] = extract_phonetic_str(words[0], label, phonetics).strip()
        aliases = []
        for word in words[1:]:
            aliases.append(extract_phonetic_str(word, label, []).strip())

        # the translation column may carry the phonetic too: 四月 [sì yuè]
        translation = extract_phonetic_str(parts[1].strip(), label, phonetics).strip()
        explanation = [translation]
        if len(parts) > 2 and parts[2].strip():
            explanation.append(parts[2].strip())

        aldict_writeword([words[0]] + aliases, phonetics, explanation)
        count += 1
finally:
    f_in.close()
    aldict_close()

sys.stderr.write("write %s, entries(%d)\n" % (out, count))
