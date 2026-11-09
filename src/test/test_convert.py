### Build a dictionary from xml ###

import subprocess

subprocess.call("../aldict build -o alphadict_en-ch.aldict alphadict_en-ch.xml my_en-ch.xml",  shell=True)

#subprocess.call("../aldict build -o cedict_ch-en.aldict cedict_ch-en.xml", shell=True)

#subprocess.call(["python", "dump_aldict.py", "cedict_ch-en.aldict"])
