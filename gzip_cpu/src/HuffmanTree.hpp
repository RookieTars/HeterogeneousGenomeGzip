#ifndef __HFTREE_H_
#define __HFTREE_H_

#include "Common.h"


template<class W>
struct HuffmanTreeNode
{
	W _weight;  //Ȩֵ
	HuffmanTreeNode<W>* _left;
	HuffmanTreeNode<W>* _right;
	HuffmanTreeNode<W>* _parent;  

	
	HuffmanTreeNode(const W& weight)
		: _weight(weight)
		, _left(nullptr)
		, _right(nullptr)
		, _parent(nullptr)
	{}
};


template<class W>
class HuffmanTree
{
private:
	typedef HuffmanTreeNode<W> Node;    

	class Compare                      
	{
	public:
		bool operator()(const Node* left, const Node* right)
		{
			return left->_weight > right->_weight;
		}
	};

	void Destory(Node* root)  
	{
		if (root)
		{
			Destory(root->_left);
			Destory(root->_right);
			delete root;
			root = nullptr;
		}
	}

public:
	HuffmanTree()                        
		: _root(nullptr)
	{}

	HuffmanTree(const vector<W>& vw, const W& invalid)     
	{
		std::priority_queue <Node*, std::vector<Node*>, Compare> q;
		for (auto i : vw)
		{
			if (i == invalid)
				continue;
			q.push(new Node(i));
		}

		while (q.size() > 1)
		{
			Node* left = q.top();
			q.pop(); 
			Node* right = q.top();
			q.pop();

			Node* parent = new Node(left->_weight + right->_weight);
			parent->_left = left;
			left->_parent = parent;

			parent->_right = right;
			right->_parent = parent;

			q.push(parent);
		}
		_root = q.top();
	}

	~HuffmanTree()  
	{
		Destory(_root);
	}
	
	Node* GetRoot()
	{
		return _root;
	}

private:
	Node* _root;
};

#endif