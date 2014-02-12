/** 
 *	@Copyright 2013  AlphaDict
 *	@Authors: LiQiong Lee
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *=====================================================================
 * 
 * This file is a test appliction, also is a example and is a document for how to
 * use the k-ary tree.
 * 
 */
#include "kary_tree.hpp"

#include <stdio.h>

//using namespace ktree;

void dump_tree(ktree::tree_node<int>::treeNodePtr pTree)
{
	printf("%d  ",pTree->value());
}

int main(int argc, char* argv[])
{
	int node_nr;

	/* Create a tree with a root node */
	ktree::kary_tree<int> tree(0);

	ktree::tree_node<int>::treeNodePtr root = tree.root();
	ktree::tree_node<int>::treeNodePtr subTree;
	
	/* Add a child at the back. */
	root->insert(1);
	root->insert(2);

	root->insert(3, 0); // Insert a child at the begin
	root->insert(4, 1); // Insert a child at the position "1".

	/* Create the leftmost subtree */
	subTree = (*root)[0];
	subTree->insert(30);
	subTree->insert(31);

	/* Creat the second subtree */
	subTree = (*root)[1];
	subTree->insert(40);
	subTree->insert(41);
	subTree = (*subTree)[1];
	subTree->insert(400);
	subTree->insert(410);

	//*** Dump this tree ***//
	// Traverse the tree and do "dump_tree" when visiting every tree node.
	ktree::traverse(root, dump_tree);
	printf("\n");

	//*** Delete the leftmost subtree ***//
	subTree = (*root)[0];
	ktree::remove(subTree);

	ktree::traverse(root, dump_tree); //Traverse the tree and do "dump_tree" when visiting every tree node.
	printf("\n");
}

/** g++ -O0 -g -o test test.cpp **/
