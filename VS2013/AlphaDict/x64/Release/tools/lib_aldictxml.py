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

  <words>
    <id_n word="word or phrase"> 
      <phonetic>
         <name1>xx</name1>
	 <name2>xx</name2>
      </phonetic>
      <explanation></explanation>
      <alias>
	    <as>word1</as>
	    <as>word2</as>
      </alias>
    </id_n>
  </words>
</alphadict>
--------------------------------------------------
"""
# Changes:
#   Init version: Sep 27, 2013 LiQiong Lee<LiQiong.kartorz.lee@gmail.com>

import codecs, xml.dom.minidom, sys

__all__ = ["xml_alphadict_openfile", "xml_alphadict_writeheader",
	   "xml_alphadict_close", "xml_alphadict_writeword", 
           "extract_phonetic_str", "get_opt"]

f_xml   = None
impl    = None
doc     = None
e_root  = None
e_words = None
entries = 0

def __trim_phrase(phrase=None):
    l = phrase.strip(' \t').split(' ') # with the leading and trailing characters removed.
    if len(l) == 1:
        return l[0]

    s = ''
    for e in l:
        if e != '':
            s += e + ' '

    s = s[0:-1] # remove ' '
    return s;

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
	global f_xml, doc, e_root, e_words

	validkeys = ["srclan", "detlan", "charset", "entries", "publisher", 
		     "publishdate", "dictversion", "dictname", "version"]
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
	e_words = doc.createElement("words")
	e_root.appendChild(e_words)
	#doc.writexml(f_xml, addindent="\t", newl="\n")

def xml_alphadict_writeword(word_lt=None, phonetic_lt=None, explanation_lt="", 
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
	global f_xml, doc, e_words, entries
	
	if not word_lt:
            sys.stderr.write("xml_alphadict_writeword: no words")
	    return
        entries = entries + 1
	## " AB    CD E F" --> ['', 'AB', '', '', '', 'CD', 'E', 'F']
        

	#strlist = word_lt[0].strip(' \t').split(' ')
	#attr_value = ''
        #for l in strlist:
        #    if l != '':
        #        attr_value += l + ' '

        #attr_value = attr_value[0:-1] # remove ' '
        attr_value = __trim_phrase(word_lt[0])
        tag = "id_" + str(entries)
	e_word = doc.createElement(tag)
        e_word.setAttribute("word", attr_value)

        if phonetic_lt != None and len(phonetic_lt) > 0:
	    e = create_element_childlist("phonetic", phonetic_lt)
	    e_word.appendChild(e)

	e = doc.createElement("explanation")
        expl = ""
        if len(explanation_lt) == 1:
            expl = explanation_lt[0];
        else:
	    for i in explanation_lt:
	        expl += i + '\n';
	    expl = expl[0:-1] # remove last ' '
        
        t = doc.createTextNode(expl)
	e.appendChild(t)
	e_word.appendChild(e)
	#create_element_childlist("note", note_lt, "text", e_word)
	#e = create_element_childlist("resource", resource_lt, None, e_word)

	if len(word_lt) > 1:
	    e_a = doc.createElement("alias")
	    for w in word_lt[1:]:
                #e_w = doc.createElement()
                s =  __trim_phrase(w)
		create_element_textnode("as", s, e_a)
            e_word.appendChild(e_a)

	e_words.appendChild(e_word)

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

# Helper fuctions
def extract_phonetic_str(strword, pcname, phonetic=None, sep='[]'):
    """ Extract a phonetic str from a phrase string
    
        format of strword: "xxxx [nn]".
        'sep' specifies the pair chars 
        around the phonetic string. 
    """
    p = []
    s = strword.find(sep[0])
    if s != -1:
        e = strword.find(sep[1], s+1)
        p.append(pcname)
        p.append(strword[s+1:e])
        phonetic.append(p)
        return strword[0:s]
    return strword

def get_opt(argv, op_list):
    """ parse user input
      
        format of cmd:  -[op] xx 
    """
    op_len = len(argv)
    i=1
    ret = {}
    while i < op_len:
        if sys.argv[i] in op_list:
            if i+1 < op_len:
                ret[sys.argv[i]] = sys.argv[i+1]
                i = i+2
                continue
            else:
                break
        i += 1
    return ret
    


