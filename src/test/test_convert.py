### Build a dictionary from the text front-end and export it back ###

import os, subprocess, sys, tempfile

here = os.path.dirname(os.path.abspath(__file__))
aldctl = os.path.join(here, "..", "aldctl")
src = os.path.join(here, "my_en-ch.txt")


def run(*args):
    if subprocess.call([aldctl] + list(args)) != 0:
        sys.exit("failed: " + " ".join(args))


with tempfile.TemporaryDirectory() as tmp:
    dic = os.path.join(tmp, "my_en-ch.aldict")
    back = os.path.join(tmp, "back.txt")
    run("build", "-o", dic, src)
    run("info", dic)
    run("export", "-o", back, dic)

    # every entry must survive the round trip
    def entries(path):
        with open(path, encoding="utf-8") as f:
            return sum(1 for line in f if line.strip() == "[entry]")

    if entries(src) != entries(back):
        sys.exit("entry count changed: %d -> %d" % (entries(src), entries(back)))
    print("ok, %d entries round-tripped" % entries(src))
