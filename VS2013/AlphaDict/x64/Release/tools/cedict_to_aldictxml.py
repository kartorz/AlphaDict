#!/usr/bin/python2.7
#-*-coding:utf-8-*-

""" A conveter for CEDICT

This module converts the CEDICT file to the front-end xml of AlphaDict.
"""

# CEDICT:
#   The CEDICT is a project to create a Chinese-English dictionary.
#   Here is the websit: 
#     "http://www.mdbg.net/chindict/chindict.php?page=cedict"
#
# The basic format of a CC-CEDICT entry is:
#   Traditional Simplified [pin1 yin1] /English equivalent 1/equivalent 2/
#   For example:
#   中國 中国 [Zhong1 guo2] /China/Middle Kingdom/
#
#
# Changes:
#   Init version: Sep 22, 2013 LiQiong Lee<LiQiong.kartorz.lee@gmail.com>
#       -- support cedict 1.0.
 
import sys, os, codecs

from lib_aldictxml import *

usage = """Usage: cedict_to_aldictxml [options]
Convert cedict file to front-end xml of AlphaDict.

    -i    The cedict file needed be converted.
    -o    Specify the output xml file.
          Default: append ".xml" as a suffix to the input file name.

For example:
    cedict_to_aldictxml.py  -i foo.txt -o foo.xml
    cedict_to_aldictxml.py  -i foo.txt
"""

op_i = ""
op_o = ""
op_list=["-i", "-o", "-p"]
op_dict = get_opt(sys.argv, op_list)

if "-i" not in op_dict:
        print usage
	sys.exit()

op_i = op_dict["-i"]
op_o = op_dict.get("-o", op_i + ".xml")


if len(sys.argv) < 2:
	print usage
	sys.exit()

# Open input file
try:
	f_input = codecs.open(op_i, mode='r', encoding='utf-8', errors='ignore')
except IOError:
	sys.stderr.write("The file(%s) does not exist \n" %(op_i))
	sys.exit()

if not xml_alphadict_openfile(op_o):
	f_input.close()
	sys.exit()

try:
	cedict_header = {}
	header = {}
	bwriter_header = False

	for line in f_input:
		# show osd information
		#sys.stdout.write(".")

		if line.startswith("#!"):
			line = line.strip('#! \r\n')
			e = line.split('=',2)
			cedict_header[e[0]] = e[1]
			# print "%s:%s" %(e[0],e[1])
		elif not line.startswith('#'):
			if not bwriter_header:
				bwriter_header = True
				header["dictversion"] = (cedict_header.get("version", "0") 
							 + "." + cedict_header.get("subversion", "0"))
				header["entries"] = cedict_header.get("entries", "0")
				header["srclan"] = "any"
                                header["detlan"] = "any"
				header["publisher"] = cedict_header.get("publisher", "cedict")
                                # 2014-05-18T08:58:12Z | 2014-05-18
                                s = cedict_header.get("date", 2000-1-1).find('T')
                                if s != -1:
                                    header["publishdate"] = cedict_header["date"][:s]
                                else:
                                    header["publishdate"] = cedict_header["date"]
				header["dictname"] = "cc-cedict dictionary"

				xml_alphadict_writeheader(header)

			# read words
			phonetics = []
			phonetic = []
			words = []
			s = line.find('[')
			e = line.find(']', s+1)
			phonetic.append(u"拼音")
			phonetic.append(line[s+1:e])
			phonetics.append(phonetic)

			temp = line[:s-1]
			words = temp.split(' ')
			
			s = line.find('/', e)
			e = line.rfind('/', s)
			temp = line[s+1:e]
			explanation = temp.split('/')
			# print explanation
			xml_alphadict_writeword(words, phonetics, explanation)
			try :
				print words[0]
			except:
				pass
finally:
	f_input.close()
	xml_alphadict_close()
	sys.stdout.write("\n")

