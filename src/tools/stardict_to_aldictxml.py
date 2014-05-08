#-*-coding:utf-8-*-
#! /usr/bin/python

""" A conveter for StartDict
StartDict is a popular open source software.
Here is the homepage: http://stardict.sourceforge.net

This module converts the StartDict's dictionary to the front-end xml of AlphaDict.
"""

import sys, os, codecs, struct

from lib_aldictxml import *

usage = """Usage: stardict_to_aldictxml [dict] [xml]

    Convert startdict's dictionary to front-end xml of AlphaDict.
    dict:  The name of dictionary. eg: 'langdao-ec-gb' means that it includs all the files of this dict:
           'langdao-ec-gb.dict', 'langdao-ec-gb.idx' and other option files -- langdao-ec-gb.xx 
    xml:   Specify the outpu xml file. [Default] is at the input file path appending ".xml" as a suffix
           to the output file

    For example:
        stardict_to_aldictxml  ~/xx/langdao/langdao-ec-gb  |  ~/xx/langdao/langdao-ec-gb  /mydicts/xx.xml
"""

BLOCK_SIZE = 40960

if len(sys.argv) < 2:
    print usage
    sys.exit()

# Open input file
try:
    f_ifo = codecs.open(sys.argv[1]+".ifo", mode='r', encoding='utf-8')
    f_dict = codecs.open(sys.argv[1]+".dict", mode='rb')
    f_idx =  open(sys.argv[1]+".idx", mode='rb')
except IOError:
    sys.stderr.write("The file(%s[.ifo|.dict|.idx]) does not exist \n" %(sys.argv[1]))
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
    star_header = {}
    header = {}
    bwriter_header = False

    for line in f_ifo:
	inx = line.find("=")
        if inx != -1:
            star_header[line[0:inx]] = line[inx+1:-1] # no '\n'      

    # Write header
    last_dot =  star_header["version"].rfind(".");
    header["dictversion"] = star_header["version"][0:last_dot] + star_header["version"][last_dot+1:-1]
    header["entries"] = star_header["wordcount"]
    header["srclan"] = "any"
    header["detlan"] = "any"
    header["publisher"] = star_header["author"] + "[" + star_header["description"] + "]"
    header["publishdate"] = star_header["date"].replace(".", "-")
    header["dictname"] = star_header["bookname"]
    xml_alphadict_writeheader(header)

    idx_buf = ""
    addr_len = 4
    fmt = ">L"
    if "idxoffsetbits" in star_header and star_header["idxoffsetbits"] == "64":
        addr_len = 8
        fmt = ">Q"

    print "\n"
    idx_buf = f_idx.read()
    #total = 0
    total = 0
    while idx_buf != "" :
        # show osd information
        #sys.stdout.write(".")

        strend = idx_buf.find('\0')
        #print strend
        if strend == -1 or strend == 0:
            print "can't find \0"
            print idx_buf
            break

        try:
        #if True:
            strinx = idx_buf[0:strend]
            print strinx
            start = strend+1

            stroff = struct.unpack(fmt, idx_buf[start:start+addr_len])[0]
            #print("%d, %x")%(stroff, stroff)
            start = start+addr_len
            #print start
            strsize = struct.unpack(fmt, idx_buf[start:start+addr_len])[0]

            words = []
            explanation = []
            words.append(strinx.decode('utf-8', 'ignore'))
            f_dict.seek(stroff)
            expl = f_dict.read(strsize).decode('utf-8', 'ignore')
            explanation.append(expl)
            xml_alphadict_writeword(words, None, explanation)
            #print("%d, %d, 0x%x, %d")%(stroff, strsize, strsize, start+addr_len)            
            idx_buf = idx_buf[start+addr_len:]            
            #print "\n"        
        except:
            print "error"
            break            
finally:
    f_idx.close()
    f_dict.close()
    f_ifo.close()
    xml_alphadict_close()
    sys.stdout.write("\n")
