
import sys
import struct
from ctypes import *

F_DICT = ""
BLOCK_SIZE = 256

if __name__ == '__main__':
    F_DICT = open(sys.argv[1], 'rb')
    header = F_DICT.read(BLOCK_SIZE)
    print "== deump header =="
    ## L is 8-bytes on 64-bit os.
    loc_chrindex = struct.unpack("<L",header[133:137])[0]
    loc_strindex = struct.unpack("<L",header[137:141])[0]
    loc_data     = struct.unpack("<L",header[141:145])[0]
    print "loc_index:(%d, %d, %d)" %(loc_chrindex, loc_strindex, loc_data)

    print "== char index =="
    F_DICT.seek(BLOCK_SIZE*(loc_chrindex-1))
    nbytes_charinx = BLOCK_SIZE*(loc_strindex - loc_chrindex)
    nbytes_strinx = BLOCK_SIZE*(loc_data - loc_chrindex)

    buf_charinx = F_DICT.read(nbytes_charinx)
    start = 0
    while (start <= nbytes_charinx):
        wchr = struct.unpack("<L", buf_charinx[start:start+4])[0]
        loc = struct.unpack("<L",buf_charinx[start+4:start+8])[0]
        length = struct.unpack("<H",buf_charinx[start+8:start+10])[0]
        print "char_index: %x:(<%x,%s>, %x, %x)" %(start, wchr, unichr(wchr), loc, length)

        start += 10
        if loc == 0 and wchr ==0:
            break
    
    print "== string index =="
    F_DICT.seek(BLOCK_SIZE*(loc_strindex-1))
    buf_strinx = F_DICT.read(nbytes_strinx)
    start = 0
    while (start <= nbytes_strinx):
        loc = struct.unpack("<L",buf_strinx[start:start+4])[0]
        strlen = struct.unpack("<B",buf_strinx[start+4:start+5])[0]
        strinx = buf_strinx[start+5:start+5+strlen*4]
        print "char_index: %x:(%x, %s)" %(start, loc, strinx.decode("utf_32_le"))
       
        start += 5 + strlen*4
        if strlen == 0:
            break

    
