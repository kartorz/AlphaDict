### Test Convert xml to dictionary ###

import subprocess

subprocess.call("../AlConvert ../../doc/alphadict_ch-en.xml ../../doc/alphadict_ch-en",  shell=True)

subprocess.call(["python", "dump_aldict.py", "../../doc/alphadict_ch-en.ad"])
