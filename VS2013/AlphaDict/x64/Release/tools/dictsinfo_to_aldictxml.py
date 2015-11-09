#!/usr/bin/python2.7
#-*-coding:utf-8-*-

""" A conveter for bilingual dictionaries on wetsit dicts.info

This module converts dicts.info's  bilingual dictionary file to the front-end xml of AlphaDict.
"""

# Dicts.info:
#   A free dictionaries Project.
#   Here is the websit: 
#     "http://www.dicts.info/"
#     "http://www.dicts.info/uddl.php"
#
# File Format
#   src-language-word ; src-language-word2  \t	Other-language-word
#   ...
#   where the pairs are separated by a tab character, and each pair is terminated by a carriage return character. 
#   April  ; Apr	四月 [sì yuè]	time
#
# Changes:
#   Init version: Jun 3, 2014 LiQiong Lee<LiQiong.kartorz.lee@gmail.com>

import sys, os, codecs

from lib_aldictxml import *

usage = """Usage: dictsinfo_to_aldictxml [options]
Convert dicts.info's bilingual dictionary file to front-end xml of AlphaDict.

    -i    The bilingual dict file.
    -o    Specify the output xml file.
          Default: append ".xml" as a suffix to the input file name.
    -p    Specity the name of phonetic chars. [Default: ' ']

For example:
    dictsinfo_to_aldictxml.py  -i foo.txt -o foo.xml  -p 拼音
    dictsinfo_to_aldictxml.py  -i foo.txt
    dictsinfo_to_aldictxml.py  -i foo.txt -p US
"""

op_p = ' '
op_i = ""
op_o = ""
op_list=["-i", "-o", "-p"]
op_dict = get_opt(sys.argv, op_list)

if "-i" not in op_dict:
        print usage
	sys.exit()

op_i = op_dict["-i"]
op_o = op_dict.get("-o", op_i + ".xml")
op_p = op_dict.get("-p", ' ')
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
	header = {}
        header["dictversion"] = "0"
        header["srclan"] = "any"
        header["detlan"] = "any"
        header["publisher"] = "www.dicts.info"
        header["publishdate"] = "2014-01-01"
        header["dictname"] = "bilingual dictionary"
        xml_alphadict_writeheader(header)

        for line in f_input:
            if line.startswith('#'):
                continue
            if line.find('\t') == -1:
                continue

            phonetics = []
            explanation = []
            words = []
            phrase = []
            phrase = line.split('\t')
            explanation.append(phrase[1])
            
            words = phrase[0].split(';')
            len_words = len(words)
            words[0] = extract_phonetic_str(words[0], op_p.decode('utf-8'), phonetics)
            
            temp = []
            i = 1
            while i < len_words:
                explanation.append(words[i]) # append synonym
            	words[i] = extract_phonetic_str(words[i], op_p.decode('utf-8'), temp)
                i += 1

            xml_alphadict_writeword(words, phonetics, explanation)
            try :
                print words[0]
            except:
                pass
finally:
	f_input.close()
	xml_alphadict_close()
	sys.stdout.write("\n")

