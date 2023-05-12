/** 
 *	@Copyright 2013  AlphaDict
 *	@Authors: LiQiong Lee
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */
#ifndef _TREE_NODE_HPP_
#define _TREE_NODE_HPP_

#include <list>
#include <iterator>


#if defined(_LINUX) && BOOST_VERSION  < 105000
template<class ForwardIt>
ForwardIt next(ForwardIt it, typename std::iterator_traits<ForwardIt>::difference_type n = 1)
{
    std::advance(it, n);
    return it;
}
#endif

namespace ktree {

template <typename T> class tree_node;
template <typename T> class kary_tree;

template <typename T>
class tree_node {
	friend class kary_tree<T>;
public:
	typedef tree_node<T>*  	           treeNodePtr;
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

	std::list<treeNodePtr>& children()
	{
		return _children;
	}

	treeNodePtr operator[](const int pos) const
	{
		return child(pos);
	}
	
	treeNodePtr child(const int pos) const
	{
		typename std::list<treeNodePtr>::const_iterator iter;
	#if defined WIN32 || BOOST_VERSION  >= 105000
		iter = std::next(_children.begin(), pos);
	#else
		iter = next(_children.begin(), pos);
	#endif
		return *iter;
	}
	
	treeNodePtr insert(const T& d, int pos=-1/*pos:begin(0);end(-1);*/)
 	{
		treeNodePtr pTree = new tree_node<T>(d);
		if (pos == -1)
			_children.push_back(pTree);
		else if(pos == 0)
			_children.push_front(pTree);
		else {
			typename std::list<treeNodePtr>::iterator iter;
		#if defined WIN32 || (BOOST_VERSION % 100) >= 50
			iter = std::next(_children.begin(), pos);
		#else
			iter = next(_children.begin(), pos);
		#endif
			_children.insert(iter, pTree);
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

	std::list<treeNodePtr> _children;
};

};

#endif
