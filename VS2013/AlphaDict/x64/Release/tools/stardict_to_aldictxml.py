#-*-coding:utf-8-*-
#! /usr/bin/python

""" A conveter for StartDict
StartDict is a popular open source software.
Here is the homepage: http://stardict.sourceforge.net

This module converts the StartDict's dictionary to the front-end xml of AlphaDict.
"""

import sys, os, codecs, struct

from lib_aldictxml import *

usage = """Usage: stardict_to_aldictxml [options]
Convert startdict's dictionary to front-end xml of AlphaDict.

    -i    The name of dictionary. eg: 'langdao-ec-gb' means that it includs all the files of this dict:
          'langdao-ec-gb.dict', 'langdao-ec-gb.idx' and other option files -- langdao-ec-gb.xx
    -o    Specify the output xml file.
          Default: append ".xml" as a suffix to the input file name.
    -p    Specity the name of phonetic chars. [Default: ' ']

For example:
    stardict_to_aldictxml  -i ~/xx/langdao/langdao-ec-gb  -o /mydicts/xx.xml  -p US
    stardict_to_aldictxml  -i ~/xx/langdao/langdao-ec-gb  
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

# Open input file
try:
    f_ifo = codecs.open(op_i+".ifo", mode='r', encoding='utf-8', errors='ignore')
    f_dict = codecs.open(op_i+".dict", mode='rb')
    f_idx =  open(op_i+".idx", mode='rb')
except IOError:
    sys.stderr.write("The file(%s[.ifo|.dict|.idx]) does not exist \n" %(op_i))
    sys.exit()

if not xml_alphadict_openfile(op_o):
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
    if "version" in star_header:
        last_dot =  star_header["version"].rfind(".");
        header["dictversion"] = star_header["version"][0:last_dot] + star_header["version"][last_dot+1:-1]
    if "wordcount" in star_header:
        header["entries"] = star_header["wordcount"]
    header["srclan"] = "any"
    header["detlan"] = "any"
    header["publisher"] = star_header.get("author", "stardict")
    header["publishdate"] = star_header.get("date", "2000.01.22").replace(".", "-")
    header["dictname"] = star_header.get("bookname", "")
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
        strend = idx_buf.find('\0')
        #print strend
        if strend == -1:
            print "can't find \0"
            print idx_buf
            break
        elif strend == 0:
           while idx_buf[strend] == '\0':
               strend += 1
    
           idx_buf = idx_buf[strend:]
           continue

        try:
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
        except:
            sys.stderr.write("read(%s), get a error, you can fix it manually, or sent a email to me" %(strinx))
            break
finally:
    f_idx.close()
    f_dict.close()
    f_ifo.close()
    xml_alphadict_close()
    sys.stdout.write("\n")
