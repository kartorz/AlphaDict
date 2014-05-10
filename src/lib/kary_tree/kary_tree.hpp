/** 
 *	@Copyright 2013  AlphaDict
 *	@Authors: LiQiong Lee
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file ../../LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */
#ifndef _KARY_TREE_HPP_
#define _KARY_TREE_HPP_

#include "tree_node.hpp"

namespace ktree {
template <typename T>
class kary_tree {
	typedef typename tree_node<T>::treeNodePtr  treeNodePtr;
public:
	kary_tree(const T& d)
	{
		_root = new tree_node<T>(d);
	}

	~kary_tree() { clear(); }

	treeNodePtr root() const { return _root; }

	void clear() const { remove(_root); }

private:
	treeNodePtr _root;
};

template <typename T>
void erase(tree_node<T> *pTree)
{
  pTree->clear();
  delete pTree;
}

template <typename T>
void remove(tree_node<T> *pTree)
{
	traverse(pTree, erase); 
}

template <typename T>
void traverse(tree_node<T> *pTree, void(*visit)(tree_node<T> *pTree))
{
	if(!pTree) return;

	if(!pTree->children().empty()) {
		typename std::list<tree_node<T> *>::const_iterator iter;
		const std::list<tree_node<T> *>& children = pTree->children();
		for (iter = children.begin(); 
		     iter != children.end();
		     ++iter) {
		     traverse(*iter, visit);
		}
	}
	visit(pTree);
}

};
#endif
