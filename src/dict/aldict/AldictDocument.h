#ifndef _ALDICTDOCUMENT_H_
#define _ALDICTDOCUMENT_H_

#include "alphadict.h"
#include "aldic_inner.h"
#include "kary_tree/kary_tree.hpp"

class AldictDocument
{
public:
	AldictDocument();
	void loadDict(const std::string& path);
	bool xmlToDict(const string& xmlPath,  const string& dictPath);
	static void dictToXml(const string& dictPath, const string& xmlPath);

private:
	ktree::kary_tree<AldictCharIndex> m_indexTree;
	struct AldictHeader m_header;
};

#endif
