#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include"Common.h"

const ush HASH_BITS = 15;   
const ush HASH_SIZE = (1 << HASH_BITS);    
const ush HASH_MASK = HASH_SIZE - 1;    
class HashTable
{
public:
	HashTable();
	~HashTable();
	
	void InsertString(ush& hashAddr, uch ch, ush pos, ush& matchHead);   
	ush GetPrevMatch(ush& hashAddr);  
	void UpdateTable();    

private:
	void HashFunc(ush& hashAddr, uch ch);
	ush H_SHIFT();

private:
	ush* _prev;
	ush* _head;
};

#endif