### Test Convert xml to dictionary ###

import subprocess

subprocess.call("../AlConvert cedict_ch-en.xml  cedict_ch-en",  shell=True)

subprocess.call(["python", "dump_aldict.py", "cedict_ch-en.aldict"])
