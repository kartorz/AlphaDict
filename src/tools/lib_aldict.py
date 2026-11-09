# -*- coding: utf-8 -*-

"""Helpers for writing the AlphaDict plain-text front-end.

The format is described in doc/front-end_format.txt and is consumed by
"aldctl build":

    [header]
    name = ...
    ...
    [entry]
    word = ...
    phonetic = ...
    explanation:
    ...

Callers keep the historical header keys (dictname, publishdate, srclan,
detlan); they are mapped to the text format here.  Explanation lines are
written verbatim inside an "explanation:" block, so they may contain '=',
'#' and blank lines.
"""

import sys

__all__ = ["aldict_openfile", "aldict_writeheader", "aldict_writeword",
           "aldict_close", "extract_phonetic_str", "get_opt"]

_f = None
_entries = 0

# text field name -> accepted keys in the caller's header dict
_HEADER_FIELDS = (
    ("name", ("name", "dictname")),
    ("publisher", ("publisher",)),
    ("date", ("date", "publishdate")),
    ("version", ("version",)),
    ("dictversion", ("dictversion",)),
    ("srclang", ("srclang", "srclan")),
    ("dstlang", ("dstlang", "detlan")),
)


def _one_line(value):
    """Collapse a value to one line: the format has no line escaping."""
    return " ".join(str(value).replace("\r", " ").split())


def aldict_openfile(path):
    """Open path for writing. Returns False when the file cannot be created."""
    global _f, _entries
    try:
        _f = open(path, "w", encoding="utf-8")
    except IOError as e:
        sys.stderr.write("cannot write %s: %s\n" % (path, e))
        return False
    _entries = 0
    _f.write("# AlphaDict dictionary text, see doc/front-end_format.txt\n")
    return True


def aldict_writeheader(header):
    """Write the [header] section from a dict with the historical keys."""
    _f.write("\n[header]\n")
    header = dict(header)
    header.setdefault("version", "1")
    for field, aliases in _HEADER_FIELDS:
        value = ""
        for key in aliases:
            if key in header and str(header[key]).strip():
                value = _one_line(header[key])
                break
        if value:
            _f.write("%s = %s\n" % (field, value))


def aldict_writeword(words, phonetics=None, explanation=None,
                     note=None, resource=None):
    """Write one [entry].

    words[0] is the head word, the remaining items become aliases.
    phonetics is a list of [name, value] pairs; the name is informational
    and is dropped because the text format stores only the value.
    explanation is a list of lines and is kept verbatim.
    """
    global _entries
    if not words or not str(words[0]).strip():
        sys.stderr.write("aldict_writeword: no word, entry skipped\n")
        return
    _entries += 1

    _f.write("\n[entry]\n")
    _f.write("word = %s\n" % _one_line(words[0]))

    for name, value in phonetics or []:
        value = _one_line(value) or _one_line(name)
        if value:
            _f.write("phonetic = %s\n" % value)

    for alias in words[1:]:
        alias = _one_line(alias)
        if alias:
            _f.write("alias = %s\n" % alias)

    lines = explanation or []
    if isinstance(lines, str):
        lines = [lines]
    flattened = []
    for item in lines:
        for line in str(item).splitlines() or [""]:
            flattened.append(line.rstrip("\r"))
    lines = flattened
    while lines and not lines[-1].strip():
        lines.pop()
    if lines:
        _f.write("explanation:\n")
        for line in lines:
            _f.write(line + "\n")


def aldict_close():
    """Flush and close the output file."""
    global _f
    if _f is not None:
        _f.close()
        _f = None


def extract_phonetic_str(strword, pcname, phonetic=None, sep="[]"):
    """Split "word [phonetic]" into the word and append [pcname, phonetic].

    Returns the word without the bracketed part.
    """
    if phonetic is None:
        phonetic = []
    start = strword.find(sep[0])
    if start != -1:
        end = strword.find(sep[1], start + 1)
        if end != -1:
            phonetic.append([pcname, strword[start + 1:end]])
            return strword[:start]
    return strword


def get_opt(argv, op_list):
    """Parse "-x value" options; returns {option: value}."""
    ret = {}
    i = 1
    while i < len(argv):
        if argv[i] in op_list:
            if i + 1 < len(argv):
                ret[argv[i]] = argv[i + 1]
                i += 2
                continue
            break
        i += 1
    return ret
