#-*-coding:utf-8-*-

""" A collection of auxiliary operations for creating front-end xml of alphadict 

The format of front-end of alphadict:
-------------------------------------
<alphadict>
<header>
		<version>1</version>
		<srclan/>
		<detlan/>
		<charset>utf-8</charset>
		<entries/>
		<publisher>[name]</publisher>
		<publishdate>[date]</publishdate>
		<dictversion/>
		<dictname/>
</header>
<dict>
        <word>
		<alias> other word </alias>
		<!-- This node means that this word shares the same meaning 
		with some other word,just like a symbol link. So,if use this node, 
		the other nodes can't be used anymore. They are mutual exclusion.
		see example: "鉴定" -->
		
		<!-- Define a normal word other than a word with a alias node -->
        	<phonetic>
                	<name1>xx</name>
                        <name2>xx</name2>
                </phonetic>
		<explanation>
                	<text>line1</text>
			<text>line2</text>
		</explanation>
		
		<!-- notes, sentences, etc. More explain about the word -->
		<note>
			<text>line1</text>
			<text>line2</text>
		</note>

		<!-- pictures, mp3, mp4, etc,some media resources.
		if it is a local file, the "url" is a local path. -->
		<resource>
			<name>url</name>
		</resource>
        </word>
</words>
</alphadict>

<word> e.g:
<鑒定>
	<phonetic>
		<拼音><jian4 ding4></拼音>
	</phonetic>
	<explanation>
		<text>na: evaluation;verification</text>
	</explanation>
</鑒定>
<鉴定>
	<alias>鑒定</alias>
</鉴定>

<boy>
	<phonetic>
		<US>noʊt</US>
		<UK>nəʊt</UK>
	</phonetic>
	<explanation>
		<text>n: 男孩；小子； 伙计</text>
		<text>int.表示惊奇</text>
	</explanation>
	<note>
		<text>男孩；男青年a male child or a young male person</text>
		<text>Sample Sentence:</text>
		<text>  1.This family was that of the merry barefoot boy.</text>
	</note>
	<resource>
		<boy>resource/boy.png</boy>
	</resource>
</boy>
--------------------------------------------------
"""
# Changes:
#   Init version: Sep 27, 2013 LiQiong Lee<LiQiong.kartorz.lee@gmail.com>

import codecs, xml.dom.minidom, sys

__all__ = ["xml_alphadict_openfile", "xml_alphadict_writeheader",
	   "xml_alphadict_close", "xml_alphadict_writeword"]

f_xml   = None
impl    = None
doc     = None
e_root  = None
e_words = None

def xml_alphadict_openfile(path):
	""" Open the xml file for writing """
	global f_xml, impl, doc, e_root

	try:
		f_xml = codecs.open(path, 'w', "utf-8")
	except IOError:
		sys.stderr.write("Can't write file(%s)" %(output_path))
                return False
        
	try:
		impl = xml.dom.minidom.getDOMImplementation()
		doc = impl.createDocument(path, "alphadict", None)
		e_root = doc.documentElement
		e_header = doc.createElement("header")
		return True
	except:
		return False

def xml_alphadict_close():
	global f_xml
	doc.writexml(f_xml, addindent="\t", newl="\n")
	f_xml.close()

def xml_alphadict_writeheader(header):
	"""
	[header] is a dictionary, Here are the keys:
	{"srclan": } -- source language which need to be translated.
	{"detlan": } -- destination language which source lauguage need to be translated to
	{"charset":} -- This version Only support utf-8. caller don't set this value.
	{"entries":} -- The total number of words.
	{"publisher":} -- Who publishs the dictionary
	{"publishdate":} -- The date that the dictionary was published or created.
	{"dictversion":} -- The version of the dictionary.
	{"dictname:"} -- The name of the dictionary.
	"""
	global f_xml, doc, e_root

	validkeys = ["srclan", "detlan", "charset", "entries", "publisher", 
		     "publishdate", "dictversion", "dictname", "version", "charset"]
	# Set default attribute.
	header["version"] = "1"
	header["charset"] = "utf-8"

	e_header = doc.createElement("header")
	for k, v in header.iteritems():
		if k in validkeys:
			e_k = create_element_textnode(k,v)
			e_header.appendChild(e_k)
		else:
			sys.stderr.write("xml_alphadict_writeheader:"
				"get a invalid key(%s)\n" %(k))
			
	e_root.appendChild(e_header)
	#doc.writexml(f_xml, addindent="\t", newl="\n")

def xml_alphadict_writeword(word_lt=None, phonetic_lt=None, explanation_lt=None, 
			    note_lt=None, resource_lt=None):
	"""
	[word_lt] is a list containing the words sharing the same content 
		e.g, the synonym.
	[phonetic_lt] is a list containing all the phonetic symbol.
		The item of this list is a list ("name", phonetic)	   
	[explain] is a list containg the short explain of a world.
	[note] is a list containg more explain of a world.
	[resources] is a list containing the resource file's url.
		The item of this list is a list("name", url)
	"""
	global f_xml, doc, e_root
	
	if not word_lt:
		sys.stderr.write("xml_alphadict_writeword: no words")
		
	e_word = doc.createElement(word_lt[0])

	e = create_element_childlist("phonetic", phonetic_lt)
	e_word.appendChild(e)

	e = create_element_childlist("explanation", explanation_lt, "text", e_word)

	create_element_childlist("note", note_lt, "text", e_word)
	
	e = create_element_childlist("resource", resource_lt, None, e_word)
	e_root.appendChild(e_word)

	if len(word_lt) > 1:
		e_a = doc.createElement("alias")
		for w in word_lt[1:]:
			e_w = doc.createElement(w)
			create_element_textnode("alias", word_lt[0], e_w)
			#e_root.appendChild(e_w)


def create_element_childlist(e, t, c=None, p=None):
	""" Create a dom element with a series text nodes

	    [return] the dom element of xml.
	    [p] the parent node.
	    [e] the name of the element.
	    [c] the name of the child element of "e". 
	    	if none, the name contained in t.
	    [t] is a list containing a series value of text node.
	    	if its item is list, it contains a pair ["name", "value"]
	"""
	if not t:
		return None

	e = doc.createElement(e)
	for i in t:
		if c:
			create_element_textnode(c, i, e)
		else:
			create_element_textnode(i[0], i[1], e)
		
	if p:
		p.appendChild(e)

	return e

def create_element_textnode(e, v, p=None):
	""" Create a dom element with a text node
	
	[return] the dom element of xml.
	[p] the parent node.
	[e] the name of the element.
	[v] the value of the text node.
	"""
	e = doc.createElement(e)
	n = doc.createTextNode(v)
	e.appendChild(n)
	if p:
		p.appendChild(e)

	return e
