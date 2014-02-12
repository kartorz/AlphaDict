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

import sys, os, codecs

from lib_aldictxml import *

usage = """Usage: convert_cedict [file_in] [file_out]
Convert cedict file to front-end xml of AlphaDict.
file_in:  The cedict file needed be converted.
file_out: Specify the detination file.
     Default is at the input file path appending ".xml" as a suffix to the output file
For example:
	convert_cdeict ../cedict  | convert_cedict ../cedict  /mydicts/xx.xml
"""

if len(sys.argv) < 2:
	print usage
	sys.exit()

# Open input file
try:
	f_input = codecs.open(sys.argv[1], mode='r', encoding='utf-8')
except IOError:
	sys.stderr.write("The file(%s) does not exist \n" %(sys.argv[1]))
	sys.exit()

if len(sys.argv) == 3:
	output_path = sys.argv[2]
else:
	output_path = sys.argv[1]
	i = output_path.rfind('.')
	if i == -1:
		output_path = sys.argv[1] + ".xml"
	else:
		output_path = output_path[:i] + ".xml"

if not xml_alphadict_openfile(output_path):
	f_input.close()
	sys.exit()

try:
	cedict_header = {}
	header = {}
	bwriter_header = False

	for line in f_input:
		# show osd information
		sys.stdout.write(".")

		if line.startswith("#!"):
			line = line.strip('#! \r\n')
			e = line.split('=',2)
			cedict_header[e[0]] = e[1]
			# print "%s:%s" %(e[0],e[1])
		elif not line.startswith('#'):
			if not bwriter_header:
				bwriter_header = True
				header["dictversion"] = (cedict_header["version"] 
							 + "." + cedict_header["subversion"])
				header["entries"] = cedict_header["entries"]
				header["srclan"] = "CN"
				header["publisher"] = cedict_header["publisher"]
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
finally:
	f_input.close()
	xml_alphadict_close()
	sys.stdout.write("\n")

