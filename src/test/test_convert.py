### Test Convert xml to dictionary ###

import subprocess

subprocess.call("../AlConvert -o alphadict_en-ch alphadict_en-ch.xml  my_en-ch.xml",  shell=True)

#subprocess.call("../AlConvert -o cedict_ch-en cedict_ch-en.xml",  shell=True)

#subprocess.call(["python", "dump_aldict.py", "cedict_ch-en.aldict"])
