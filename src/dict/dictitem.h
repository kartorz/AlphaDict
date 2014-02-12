#ifndef _DICTITEM_H_
#define _DICTITEM_H_

#include "common.h"

class DictItem 
{
public:
	DictItem(AString& phoneticSymbols, AString&  );

	AString phoneticSymbols;
	AString content;
	inode_t ino;
};

#endif
