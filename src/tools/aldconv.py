#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Convert third-party dictionaries to the AlphaDict plain-text front-end.

The output is read by "aldctl build" (see doc/front-end_format.txt).

Supported input formats, and where to get the large ones:

  stardict   StarDict .ifo/.idx/.dict[.dz], give the base name
             http://download.huzheng.org/  (Chinese/English collections)
             https://freedict.org/downloads/  (*.stardict.tar.xz)
  dictd      dictd .index + .dict[.dz], give the base name
             https://freedict.org/downloads/  (*.dictd.tar.xz)
  wordnet    WordNet database directory, give the directory
             "apt install wordnet" then use /usr/share/wordnet
  cedict     CC-CEDICT, plain or .gz
             https://www.mdbg.net/chinese/dictionary?page=cedict
  jmdict     JMdict_e XML, plain or .gz
             https://www.edrdg.org/pub/Nihongo/JMdict_e.gz
  edict2     EDICT2, plain or .gz (EUC-JP and UTF-8 both work)
             https://www.edrdg.org/pub/Nihongo/edict2u.gz  (UTF-8)
             https://www.edrdg.org/pub/Nihongo/edict2.gz   (EUC-JP)
  idp        IDP tab-separated word pairs
  dictsinfo  dicts.info tab-separated rows

The format is guessed from the input when -f is omitted.  .gz/.bz2/.xz
inputs are decompressed on the fly.
"""

import argparse
import bz2
import codecs
import gzip
import lzma
import os
import re
import shutil
import struct
import sys
import tempfile

EXAMPLES = """examples:
  aldconv.py -i ~/dicts/langdao-ec-gb          StarDict base name
  aldconv.py -i cedict.txt.gz -o cedict.txt    CC-CEDICT, gzip is fine
  aldconv.py -i /usr/share/wordnet             WordNet directory
  aldconv.py -i JMdict_e.gz -o jmdict.txt      JMdict XML
  aldconv.py -i edict2.gz -f edict2            EDICT2 text

downloads (dictionaries with many entries):
  http://download.huzheng.org/                 StarDict 词典合集
  https://freedict.org/downloads/              FreeDict 双语词典
  https://www.edrdg.org/pub/Nihongo/           JMdict_e.gz, edict2.gz
  https://www.mdbg.net/chinese/dictionary?page=cedict
  apt install wordnet                          -> /usr/share/wordnet

write the result to a .txt file, then:
  aldctl build -o mydict.aldict mydict.txt
"""

# Limits of the binary header and entries, see src/dict/aldict/aldict_inner.h
# and src/tools/aldctl.cpp.  aldctl cuts header fields on byte boundaries,
# which can split a UTF-8 character, so they are cut here instead.
HEADER_NAME_MAX = 59       # d_identi is 60 bytes including the NUL
HEADER_PUBLISHER_MAX = 59  # p_identi is 60 bytes including the NUL
HEADER_LANG_MAX = 14       # src_lan/det_lan are 15 bytes including the NUL
WORD_MAX = 255
PHONETIC_MAX = 255

# FreeDict ships files named freedict-eng-deu-0.7.0.<format>; keeping the
# declared languages lets AlphaDict pick the dictionary for the typed script.
ISO639 = {
    "eng": "english", "zho": "chinese", "cmn": "chinese", "jpn": "japanese",
    "kor": "korean", "deu": "german", "ger": "german", "fra": "french",
    "fre": "french", "dan": "danish", "ell": "greek", "gre": "greek",
    "rus": "russian", "spa": "spanish", "ita": "italian", "por": "portuguese",
    "nld": "dutch", "dut": "dutch", "swe": "swedish", "pol": "polish",
    "ces": "czech", "cze": "czech", "fin": "finnish", "hun": "hungarian",
    "ron": "romanian", "rum": "romanian", "ukr": "ukrainian", "lat": "latin",
    "epo": "esperanto", "ara": "arabic", "heb": "hebrew", "tur": "turkish",
    "fas": "persian", "hin": "hindi", "vie": "vietnamese", "tha": "thai",
    "ind": "indonesian", "msa": "malay",
}


def one_line(value):
    """The text format has no escaping, so a value must stay on one line."""
    return " ".join(str(value).replace("\r", " ").split())


def cut_bytes(text, limit):
    """Truncate to limit bytes without splitting a UTF-8 character."""
    raw = text.encode("utf-8")
    if len(raw) <= limit:
        return text
    return raw[:limit].decode("utf-8", "ignore")


TEXT_ENCODINGS = ("utf-8", "euc-jp", "cp932")


def open_binary(path):
    """Open a file for binary reading, transparently decompressing."""
    if path.endswith(".gz"):
        return gzip.open(path, "rb")
    if path.endswith(".bz2"):
        return bz2.open(path, "rb")
    if path.endswith((".xz", ".lzma")):
        return lzma.open(path, "rb")
    return open(path, "rb")


def probe_encoding(path, size=1 << 18):
    """EDICT2 ships as EUC-JP (edict2.gz) and as UTF-8 (edict2u.gz), so the
    encoding has to be measured instead of assumed."""
    with open_binary(path) as f:
        raw = f.read(size)
    for enc in TEXT_ENCODINGS:
        try:
            raw.decode(enc)
            return enc
        except UnicodeDecodeError as e:
            # The sample may end in the middle of a multi-byte character.
            if e.start >= len(raw) - 3:
                return enc
    return "utf-8"


def open_text(path, encoding=None):
    """Open a text file, transparently decompressing .gz/.bz2/.xz.

    Decoding is strict on purpose: replacing undecodable bytes silently turns
    headwords into U+FFFD and produces a dictionary nobody can search."""
    enc = encoding or probe_encoding(path)
    if path.endswith(".gz"):
        return gzip.open(path, "rt", encoding=enc, errors="strict")
    if path.endswith(".bz2"):
        return bz2.open(path, "rt", encoding=enc, errors="strict")
    if path.endswith((".xz", ".lzma")):
        return lzma.open(path, "rt", encoding=enc, errors="strict")
    return open(path, "r", encoding=enc, errors="strict")


def open_entry_data(base):
    """Return (path, temporary) for the entries of a StarDict or dictd set.

    .dict.dz is a gzip file and the index jumps around, so unpack it once to a
    temporary file instead of seeking a gzip stream.
    """
    path = base + ".dict"
    if os.path.exists(path):
        return path, None
    dz = path + ".dz"
    if not os.path.exists(dz):
        return None, None
    tmp = tempfile.NamedTemporaryFile(prefix="aldconv-", suffix=".dict",
                                      delete=False)
    try:
        with gzip.open(dz, "rb") as src:
            shutil.copyfileobj(src, tmp, 1 << 20)
    finally:
        tmp.close()
    return tmp.name, tmp.name


def lang_pair(path):
    """Language names from a FreeDict style file name, ("", "") if unknown."""
    m = re.search(r"-([a-z]{3})-([a-z]{3})(?:-|$)", os.path.basename(path))
    if not m:
        return "", ""
    return ISO639.get(m.group(1), ""), ISO639.get(m.group(2), "")


class TextWriter:
    def __init__(self, path):
        self.path = path
        self.entries = 0
        self._progress = False
        self._f = open(path, "w", encoding="utf-8")
        self._f.write("# AlphaDict dictionary text, see doc/front-end_format.txt\n")

    def header(self, fields):
        name = one_line(fields.get("name", ""))
        publisher = one_line(fields.get("publisher", ""))
        cut_name = cut_bytes(name, HEADER_NAME_MAX)
        cut_pub = cut_bytes(publisher, HEADER_PUBLISHER_MAX)
        if cut_name != name or cut_pub != publisher:
            sys.stderr.write("warning: header field cut to fit the binary header\n")

        self._f.write("\n[header]\n")
        for key, value in (("name", cut_name), ("publisher", cut_pub),
                           ("date", one_line(fields.get("date", "")))):
            if value:
                self._f.write("%s = %s\n" % (key, value))
        self._f.write("version = 1\n")
        for key in ("dictversion", "srclang", "dstlang"):
            value = one_line(fields.get(key, ""))
            if key != "dictversion":
                value = cut_bytes(value, HEADER_LANG_MAX)
            if value:
                self._f.write("%s = %s\n" % (key, value))

    def entry(self, word, phonetic=None, aliases=(), explanation=()):
        word = one_line(word)
        if not word:
            sys.stderr.write("skip entry without word\n")
            return
        if len(word.encode("utf-8")) > WORD_MAX:
            sys.stderr.write("skip word over %d bytes: %s\n" % (WORD_MAX, word))
            return

        self.entries += 1
        if self.entries % 20000 == 0:
            sys.stderr.write("\r%d entries..." % self.entries)
            self._progress = True

        self._f.write("\n[entry]\nword = %s\n" % word)
        phonetic = cut_bytes(one_line(phonetic or ""), PHONETIC_MAX)
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
        if self._progress:
            sys.stderr.write("\r")
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


def file_version(basename):
    """FreeDict names carry the release: freedict-eng-deu-0.7.0.index."""
    m = re.search(r"-(\d+\.\d+(?:\.\d+)?)(?:-|\.|$)", basename)
    return two_part_version(m.group(1)) if m else ""


# ---- input formats -------------------------------------------------------

def from_stardict(w, base, name=None):
    ifo = {}
    try:
        with codecs.open(base + ".ifo", "r", encoding="utf-8",
                         errors="ignore") as f:
            for line in f:
                key, sep, value = line.partition("=")
                if sep:
                    ifo[key.strip()] = value.strip()
    except OSError as e:
        sys.exit("cannot read %s.ifo: %s" % (base, e))

    src, dst = lang_pair(base)
    w.header({
        "name": name or ifo.get("bookname", os.path.basename(base)),
        "publisher": ifo.get("author", "stardict"),
        "date": ifo.get("date", "").replace(".", "-"),
        "dictversion": two_part_version(ifo.get("version", "0.0")),
        "srclang": src or "any",
        "dstlang": dst or "any",
    })

    # 64-bit offsets are signalled in the .ifo; sizes stay 32-bit.
    off_fmt = ">Q" if ifo.get("idxoffsetbits") == "64" else ">L"
    off_len = struct.calcsize(off_fmt)
    dict_path, tmp = open_entry_data(base)
    if not dict_path:
        sys.exit("cannot read %s[.dict|.dict.dz]" % base)
    try:
        with open(base + ".idx", "rb") as f_idx, open(dict_path, "rb") as f_dict:
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
    finally:
        if tmp:
            os.unlink(tmp)


def from_dictd(w, base, name=None):
    """dictd: a text .index of "word<TAB>offset<TAB>size" plus .dict[.dz]."""
    index_path = base + ".index"
    if not os.path.exists(index_path):
        sys.exit("cannot read %s" % index_path)

    src, dst = lang_pair(base)
    w.header({
        "name": name or os.path.basename(base),
        "publisher": "dictd",
        "dictversion": file_version(os.path.basename(base)),
        "srclang": src or "any",
        "dstlang": dst or "any",
    })

    dict_path, tmp = open_entry_data(base)
    if not dict_path:
        sys.exit("cannot read %s[.dict|.dict.dz]" % base)
    try:
        with open(index_path, "r", encoding="utf-8", errors="replace") as f_idx, \
                open(dict_path, "rb") as f_dict:
            for line in f_idx:
                parts = line.rstrip("\r\n").split("\t")
                if len(parts) < 3:
                    continue
                word = parts[0]
                # "@@@" marks a 64-bit index, its offsets are not plain ints.
                if word.startswith("@@"):
                    sys.stderr.write("skip 64-bit index marker\n")
                    continue
                try:
                    offset, size = int(parts[1]), int(parts[2])
                except ValueError:
                    continue
                f_dict.seek(offset)
                text = f_dict.read(size).decode("utf-8", "ignore")
                w.entry(word, explanation=text.splitlines())
    except OSError as e:
        sys.exit("cannot read %s: %s" % (base, e))
    finally:
        if tmp:
            os.unlink(tmp)


def from_wordnet(w, path, name=None):
    """WordNet data files: one entry per synset, the other words as aliases."""
    if not os.path.exists(os.path.join(path, "data.noun")):
        sys.exit("%s has no data.noun, not a WordNet directory" % path)

    w.header({
        "name": name or "wordnet",
        "publisher": "Princeton University WordNet",
        "srclang": "english",
        "dstlang": "english",
    })

    pos_name = {"n": "n.", "v": "v.", "a": "adj.", "s": "adj.", "r": "adv."}
    for fname in ("data.noun", "data.verb", "data.adj", "data.adv"):
        full = os.path.join(path, fname)
        if not os.path.exists(full):
            continue
        with codecs.open(full, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                # The license header has leading spaces and no gloss.
                if line.startswith("  ") or "|" not in line:
                    continue
                body, _, gloss = line.partition(" | ")
                tok = body.split()
                if len(tok) < 5:
                    continue
                ss_type = tok[2]
                try:
                    w_cnt = int(tok[3], 16)
                except ValueError:
                    continue
                words = []
                for item in tok[4:4 + 2 * w_cnt:2]:
                    # A word may carry a (p)/(a) satellite marker.
                    words.append(re.sub(r"\([a-z]+\)$", "", item).replace("_", " "))
                if not words:
                    continue
                gloss = gloss.strip()
                expl = ["%s %s" % (pos_name.get(ss_type, ss_type), gloss)] if gloss else []
                w.entry(words[0], aliases=words[1:], explanation=expl)


def from_cedict(w, path, name=None):
    meta = {}
    written = False
    with open_text(path) as f:
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


def from_edict2(w, path, name=None):
    """EDICT2: "kanji;kanji [reading] /(pos) gloss/gloss/" per line."""
    w.header({
        "name": name or "edict2",
        "publisher": "EDRDG",
        "srclang": "japanese",
        "dstlang": "english",
    })

    with open_text(path) as f:
        for line in f:
            line = line.rstrip("\r\n")
            if not line or line.startswith("#"):
                continue
            head, sep, tail = line.partition(" /")
            if not sep:
                continue

            senses = [x.strip() for x in tail.rstrip("/").split("/") if x.strip()]
            lb = head.rfind(" [")
            rb = head.rfind("]")
            if lb >= 0 and rb > lb:
                words, reading = head[:lb], head[lb + 2:rb]
            else:
                words, reading = head, ""
            words = [x.strip() for x in words.split(";") if x.strip()]
            # EDICT2 starts with a "？？？" placeholder holding the copyright.
            if words and words[0].strip("？?　 "):
                w.entry(words[0], reading, words[1:], senses)


XML_ENTITY_RE = re.compile(r"&([A-Za-z][\w.\-]*);")
XML_BUILTIN = ("amp", "lt", "gt", "quot", "apos")


def strip_jmdict_dtd(stream):
    """JMdict_e declares parts of speech as XML entities (&n;, &vt;, ...) in an
    internal DTD subset.  ElementTree cannot resolve them, so the DOCTYPE
    block is dropped and an unknown entity becomes its own name
    ("&vt;" -> "vt").  The DTD runs until "]>" on a later line."""
    in_doctype = False
    for line in stream:
        if in_doctype:
            if "]>" in line:
                in_doctype = False
            continue
        if "<!DOCTYPE" in line:
            in_doctype = "[" in line and "]>" not in line
            continue
        yield XML_ENTITY_RE.sub(
            lambda m: "&%s;" % m.group(1) if m.group(1) in XML_BUILTIN
            else m.group(1), line)


def from_jmdict(w, path, name=None):
    """JMdict_e XML: kanji + reading + one line per sense."""
    from xml.etree import ElementTree

    w.header({
        "name": name or "jmdict",
        "publisher": "EDRDG",
        "srclang": "japanese",
        "dstlang": "english",
    })

    # iterparse wants a file object or a path, so the cleaned XML goes to a
    # temporary file.  A real JMdict is around 100MB, keeping it in memory
    # would be worse.
    tmp = tempfile.NamedTemporaryFile(prefix="aldconv-", suffix=".xml",
                                      delete=False, mode="w",
                                      encoding="utf-8", newline="")
    try:
        with open_text(path) as f:
            for line in strip_jmdict_dtd(f):
                tmp.write(line)
        tmp.close()

        for _, elem in ElementTree.iterparse(tmp.name, events=("end",)):
            if elem.tag != "entry":
                continue

            keys = [e.text.strip() for e in elem.findall("k_ele/keb")
                    if e.text and e.text.strip()]
            reads = [e.text.strip() for e in elem.findall("r_ele/reb")
                     if e.text and e.text.strip()]

            lines = []
            for sense in elem.findall("sense"):
                pos = [e.text.strip() for e in sense.findall("pos")
                       if e.text and e.text.strip()]
                gloss = [e.text.strip() for e in sense.findall("gloss")
                         if e.text and e.text.strip()]
                if gloss:
                    prefix = "(%s) " % ";".join(pos) if pos else ""
                    lines.append(prefix + "; ".join(gloss))

            word = keys[0] if keys else (reads[0] if reads else "")
            if word:
                phonetic = reads[0] if reads and reads[0] != word else ""
                w.entry(word, phonetic, keys[1:] + reads[:1], lines)
            elem.clear()
    except ElementTree.ParseError as e:
        sys.exit("cannot parse %s: %s" % (path, e))
    finally:
        tmp.close()
        os.unlink(tmp.name)


def from_idp(w, path, name=None):
    w.header({
        "name": name or os.path.splitext(os.path.basename(path))[0],
        "publisher": "IDP",
        "date": "2007-02-01",
        "dictversion": "0.1",
        "srclang": "english",
        "dstlang": "any",
    })
    with open_text(path) as f:
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
    with open_text(path) as f:
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
    "dictd": from_dictd,
    "wordnet": from_wordnet,
    "cedict": from_cedict,
    "jmdict": from_jmdict,
    "edict2": from_edict2,
    "idp": from_idp,
    "dictsinfo": from_dictsinfo,
}

# Formats whose -i argument is a directory or a base name without extension.
BASED = ("stardict", "dictd", "wordnet")

KANA_RE = re.compile("[\u3040-\u30ff]")


def normalize(path):
    """Accept foo.ifo, foo.idx, foo.index, foo.dict, foo.dict.dz or foo."""
    for ext in (".ifo", ".idx", ".index", ".dict", ".dict.dz"):
        if path.endswith(ext):
            base = path[:-len(ext)]
            if ext in (".ifo", ".idx") and not os.path.exists(base + ".ifo"):
                return path
            return base
    return path


def detect(path):
    """Guess the format; returns None when nothing matches."""
    if os.path.isdir(path):
        if os.path.exists(os.path.join(path, "data.noun")):
            return "wordnet"
        return None
    if os.path.exists(path + ".index") and (
            os.path.exists(path + ".dict") or os.path.exists(path + ".dict.dz")):
        return "dictd"
    if os.path.exists(path + ".ifo"):
        return "stardict"

    try:
        with open_text(path) as f:
            sample = f.read(65536)
    except OSError as e:
        sys.exit("cannot read %s: %s" % (path, e))

    if "<JMdict" in sample:
        return "jmdict"

    columns = 0
    for line in sample.splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        if "\t" in line:
            columns = max(columns, len(line.split("\t")))
        elif KANA_RE.search(line) and "[" in line and " /" in line:
            return "edict2"
        elif "[" in line and "]" in line and line.count("/") >= 2:
            return "cedict"
    if columns >= 3:
        return "dictsinfo"
    if columns == 2:
        return "idp"
    return None


def default_output(src):
    if os.path.isdir(src):
        return os.path.basename(os.path.normpath(src)) + "_aldict.txt"
    root = src
    for ext in (".gz", ".bz2", ".xz", ".lzma"):
        if root.endswith(ext):
            root = root[:-len(ext)]
            break
    return (os.path.splitext(root)[0] or root) + "_aldict.txt"


def main(argv=None):
    ap = argparse.ArgumentParser(
        prog="aldconv.py",
        description="convert third-party dictionaries to AlphaDict text",
        epilog=EXAMPLES,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-i", "--input", required=True,
                    help="input file, directory, or the base name of a "
                         "StarDict/dictd set")
    ap.add_argument("-o", "--output",
                    help="output text file (default: <input>_aldict.txt)")
    ap.add_argument("-f", "--format", choices=sorted(CONVERTERS),
                    help="input format (default: guessed from the input)")
    ap.add_argument("-n", "--name", help="dictionary name in the header")
    opts = ap.parse_args(argv)

    src = normalize(opts.input)
    if not (os.path.isdir(src) or os.path.exists(src)
            or os.path.exists(src + ".ifo") or os.path.exists(src + ".index")):
        sys.exit("cannot read %s" % opts.input)

    fmt = opts.format or detect(src)
    if not fmt:
        sys.exit("cannot guess the format of %s, pass -f" % opts.input)
    if not opts.format:
        sys.stderr.write("%s: detected %s\n" % (opts.input, fmt))

    try:
        writer = TextWriter(opts.output or default_output(src))
    except OSError as e:
        sys.exit("cannot write %s: %s" % (opts.output, e))
    try:
        CONVERTERS[fmt](writer, src, opts.name)
    finally:
        writer.close()


if __name__ == "__main__":
    main()
