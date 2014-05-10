/** 
 *	@Copyright 2014  AlphaDict
 *	@Authors: LiQiong Lee
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */
#ifndef _TREE_TREE2_HPP_
#define _TREE_TREE2_HPP_

#include <vector>
#include <iterator>

namespace ktree {
template <typename T> class tree_node;
template <typename T> class kary_tree2;

template <typename T>
class kary_tree2 {
	typedef typename tree_node<T>::treeNodePtr  treeNodePtr;
public:
	kary_tree2(const T& d)
	{
		_root = new tree_node<T>(d);
	}

	~kary_tree2() { clear(); }

	treeNodePtr root() const { return _root; }

	void clear() const { remove(_root); }

private:
	treeNodePtr _root;
};


template <typename T>
class tree_node {
	friend class kary_tree2<T>;
public:
	typedef tree_node<T>*     	       treeNodePtr;
	typedef const tree_node<T>*  const_treeNodePtr;

	treeNodePtr parent() const
	{
		return _parent;
	}

	T& value()
	{
		return _data;
	}

	void operator=(const T& d)
	{
		_data = d;
	}

	std::vector<treeNodePtr>& children()
	{
		return _children;
	}

	treeNodePtr operator[](const int pos) const
	{
		return _children[pos];
	}

	treeNodePtr child(const int pos) const
	{
		return _children[pos];
	}

	treeNodePtr insert(const T& d, int pos=-1/*pos:begin(0);end(-1);*/)
 	{
		treeNodePtr pTree = new tree_node<T>(d);
		if (pos == -1)
			_children.push_back(pTree);
		else {
            typename std::vector<treeNodePtr>::iterator begin = _children.begin();
			_children.insert(begin+pos, pTree);
		}
		pTree->_parent = this;
		return pTree;
	}

	void clear()
	{
        _children.clear();
	}

private:
	tree_node(const T& d) : _data(d) {}
	tree_node();

	treeNodePtr  _parent;
    T _data;

	std::vector<treeNodePtr> _children;
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
		typename std::vector<tree_node<T> *>::const_iterator iter;
		const std::vector<tree_node<T> *>& children = pTree->children();
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
