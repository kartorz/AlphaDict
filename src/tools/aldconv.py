#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Convert third-party dictionaries to the AlphaDict plain-text front-end.

The output is read by "aldctl build" (see doc/front-end_format.txt).
Supported input formats:

  stardict   StarDict .ifo/.idx/.dict triple (give the base name)
  cedict     CC-CEDICT text file
  idp        IDP tab-separated word pairs
  dictsinfo  dicts.info tab-separated rows

The format is guessed from the input when -f is omitted.

Examples:
  aldconv.py -i ~/dicts/langdao-ec-gb
  aldconv.py -i cedict.txt -o cedict-zh.txt
  aldconv.py -f idp -i french.txt -n french
  aldconv.py -f dictsinfo -i april.txt
"""

import argparse
import codecs
import os
import struct
import sys

EXAMPLES = """examples:
  aldconv.py -i ~/dicts/langdao-ec-gb        StarDict base name
  aldconv.py -i cedict.txt -o cedict-zh.txt
  aldconv.py -f idp -i french.txt -n french
  aldconv.py -f dictsinfo -i april.txt

write the result to a .txt file, then:
  aldctl build -o mydict.aldict mydict.txt
"""


def one_line(value):
    """The text format has no escaping, so a value must stay on one line."""
    return " ".join(str(value).replace("\r", " ").split())


class TextWriter:
    def __init__(self, path):
        self.path = path
        self.entries = 0
        self._f = open(path, "w", encoding="utf-8")
        self._f.write("# AlphaDict dictionary text, see doc/front-end_format.txt\n")

    def header(self, fields):
        self._f.write("\n[header]\n")
        for key in ("name", "publisher", "date"):
            value = one_line(fields.get(key, ""))
            if value:
                self._f.write("%s = %s\n" % (key, value))
        self._f.write("version = 1\n")
        for key in ("dictversion", "srclang", "dstlang"):
            value = one_line(fields.get(key, ""))
            if value:
                self._f.write("%s = %s\n" % (key, value))

    def entry(self, word, phonetic=None, aliases=(), explanation=()):
        word = one_line(word)
        if not word:
            sys.stderr.write("skip entry without word\n")
            return
        self.entries += 1
        self._f.write("\n[entry]\nword = %s\n" % word)
        phonetic = one_line(phonetic or "")
        if phonetic:
            self._f.write("phonetic = %s\n" % phonetic)
        for alias in aliases:
            alias = one_line(alias)
            if alias and alias != word:
                self._f.write("alias = %s\n" % alias)

        lines = []
        for item in explanation:
            lines.extend(str(item).splitlines() or [""])
        while lines and not lines[-1].strip():
            lines.pop()
        if lines:
            self._f.write("explanation:\n")
            for line in lines:
                self._f.write(line + "\n")

    def close(self):
        self._f.close()
        sys.stderr.write("write %s, entries(%d)\n" % (self.path, self.entries))


def split_phonetic(text):
    """Split "word [phonetic]" into the word and the bracketed part."""
    start = text.find("[")
    end = text.find("]", start + 1) if start >= 0 else -1
    if 0 <= start < end:
        return text[:start].strip(), text[start + 1:end].strip()
    return text.strip(), ""


def two_part_version(version):
    """StarDict versions may be 2.4.2; the format only keeps major.minor."""
    parts = [p for p in str(version).split(".") if p]
    while len(parts) < 2:
        parts.append("0")
    return "%s.%s" % (parts[0], parts[1])


# ---- input formats -------------------------------------------------------

def from_stardict(w, base, name=None):
    ifo = {}
    try:
        with codecs.open(base + ".ifo", "r", encoding="utf-8", errors="ignore") as f:
            for line in f:
                key, sep, value = line.partition("=")
                if sep:
                    ifo[key.strip()] = value.strip()
    except OSError as e:
        sys.exit("cannot read %s.ifo: %s" % (base, e))

    w.header({
        "name": name or ifo.get("bookname", os.path.basename(base)),
        "publisher": ifo.get("author", "stardict"),
        "date": ifo.get("date", "").replace(".", "-"),
        "dictversion": two_part_version(ifo.get("version", "0.0")),
        "srclang": "any",
        "dstlang": "any",
    })

    # 64-bit offsets are signalled in the .ifo; sizes stay 32-bit.
    off_fmt = ">Q" if ifo.get("idxoffsetbits") == "64" else ">L"
    off_len = struct.calcsize(off_fmt)
    try:
        with open(base + ".idx", "rb") as f_idx, open(base + ".dict", "rb") as f_dict:
            data = f_idx.read()
            pos = 0
            while pos < len(data):
                end = data.find(b"\0", pos)
                if end < 0:
                    sys.stderr.write("bad .idx: no NUL terminator\n")
                    break
                if end == pos:
                    pos += 1
                    continue
                # word\0 offset size; a truncated tail is dropped.
                if len(data) < end + 1 + off_len + 4:
                    sys.stderr.write("bad .idx: truncated record\n")
                    break
                word = data[pos:end].decode("utf-8", "ignore")
                offset = struct.unpack(off_fmt, data[end + 1:end + 1 + off_len])[0]
                size_pos = end + 1 + off_len
                size = struct.unpack(">L", data[size_pos:size_pos + 4])[0]
                pos = size_pos + 4

                f_dict.seek(offset)
                text = f_dict.read(size).decode("utf-8", "ignore")
                w.entry(word, explanation=[text])
    except OSError as e:
        sys.exit("cannot read %s[.idx|.dict]: %s" % (base, e))


def from_cedict(w, path, name=None):
    meta = {}
    written = False
    with codecs.open(path, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            line = line.rstrip("\r\n")
            if line.startswith("#!"):
                key, _, value = line[2:].partition("=")
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

            if not written:
                written = True
                date = meta.get("date", "")
                if "T" in date:
                    date = date[:date.find("T")]
                w.header({
                    "name": name or "cc-cedict",
                    "publisher": meta.get("publisher", "cedict"),
                    "date": date,
                    "dictversion": "%s.%s" % (meta.get("version", "0"),
                                              meta.get("subversion", "0")),
                    "srclang": "chinese",
                    "dstlang": "english",
                })

            words = line[:lb].split()
            phonetic = line[lb + 1:rb].strip()
            explanation = [x.strip() for x in line[first + 1:last].split("/")]
            if words:
                w.entry(words[0], phonetic, words[1:],
                        [x for x in explanation if x])


def from_idp(w, path, name=None):
    w.header({
        "name": name or os.path.splitext(os.path.basename(path))[0],
        "publisher": "IDP",
        "date": "2007-02-01",
        "dictversion": "0.1",
        "srclang": "english",
        "dstlang": "any",
    })
    with codecs.open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.rstrip("\r\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 2:
                continue
            word, meaning = parts[0].strip(), parts[1].strip()
            if word and meaning:
                w.entry(word, explanation=[meaning])


def from_dictsinfo(w, path, name=None):
    w.header({
        "name": name or "dicts.info bilingual",
        "publisher": "www.dicts.info",
        "date": "2014-01-01",
        "dictversion": "0.1",
        "srclang": "any",
        "dstlang": "any",
    })
    with codecs.open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.rstrip("\r\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 2:
                continue

            words = [x for x in (p.strip() for p in parts[0].split(";")) if x]
            if not words:
                continue
            word, phonetic = split_phonetic(words[0])
            aliases = [split_phonetic(x)[0] for x in words[1:]]

            # the translation column may carry the phonetic too: 四月 [sì yuè]
            meaning, extra = split_phonetic(parts[1])
            phonetic = phonetic or extra
            explanation = [meaning] if meaning else []
            if len(parts) > 2 and parts[2].strip():
                explanation.append(parts[2].strip())

            w.entry(word, phonetic, aliases, explanation)


CONVERTERS = {
    "stardict": from_stardict,
    "cedict": from_cedict,
    "idp": from_idp,
    "dictsinfo": from_dictsinfo,
}


def stardict_base(path):
    """Accept foo, foo.ifo or foo.idx; return the base name."""
    for ext in (".ifo", ".idx", ".dict"):
        if path.endswith(ext) and os.path.exists(path[:-len(ext)] + ".ifo"):
            return path[:-len(ext)]
    return path


def detect(path):
    """Guess the format; returns None when nothing matches."""
    if os.path.exists(path + ".ifo"):
        return "stardict"
    try:
        with open(path, "rb") as f:
            sample = f.read(65536).decode("utf-8", "replace")
    except OSError as e:
        sys.exit("cannot read %s: %s" % (path, e))

    columns = 0
    for line in sample.splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        if "\t" in line:
            columns = max(columns, len(line.split("\t")))
        elif "[" in line and "]" in line and line.count("/") >= 2:
            return "cedict"
    if columns >= 3:
        return "dictsinfo"
    if columns == 2:
        return "idp"
    return None


def main(argv=None):
    ap = argparse.ArgumentParser(
        prog="aldconv.py",
        description="convert third-party dictionaries to AlphaDict text",
        epilog=EXAMPLES,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-i", "--input", required=True,
                    help="input file, or the base name of a StarDict set")
    ap.add_argument("-o", "--output", help="output text file (default: <input>.txt)")
    ap.add_argument("-f", "--format", choices=sorted(CONVERTERS),
                    help="input format (default: guessed from the input)")
    ap.add_argument("-n", "--name", help="dictionary name in the header")
    opts = ap.parse_args(argv)

    opts.input = stardict_base(opts.input)
    if not os.path.exists(opts.input) and not os.path.exists(opts.input + ".ifo"):
        sys.exit("cannot read %s" % opts.input)

    fmt = opts.format or detect(opts.input)
    if not fmt:
        sys.exit("cannot guess the format of %s, pass -f" % opts.input)
    if not opts.format:
        sys.stderr.write("%s: detected %s\n" % (opts.input, fmt))

    if opts.output:
        out = opts.output
    elif os.path.splitext(opts.input)[1]:
        # never overwrite the input: cedict.txt -> cedict_aldict.txt
        out = os.path.splitext(opts.input)[0] + "_aldict.txt"
    else:
        out = opts.input + ".txt"
    try:
        writer = TextWriter(out)
    except OSError as e:
        sys.exit("cannot write %s: %s" % (out, e))
    try:
        CONVERTERS[fmt](writer, opts.input, opts.name)
    finally:
        writer.close()


if __name__ == "__main__":
    main()
