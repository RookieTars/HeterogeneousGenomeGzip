#include"HashTable.h"


HashTable::HashTable()
	: _prev(new ush[2 * HASH_SIZE])
	, _head(_prev + HASH_SIZE)
{
	
	memset(_prev, 0, sizeof(_prev[0])* 2 * HASH_SIZE);
}

HashTable::~HashTable()
{
	delete[] _prev;
	_prev = nullptr;
	_head = nullptr;
}


void HashTable::HashFunc(ush& hashAddr, uch ch)
{
	hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
}
ush HashTable::H_SHIFT()
{
	return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
}


void HashTable::InsertString(ush& hashAddr, uch ch, ush pos, ush& matchHead)
{
	HashFunc(hashAddr, ch);

	_prev[pos & HASH_MASK] = _head[hashAddr];  
	matchHead = _prev[pos & HASH_MASK];  
	_head[hashAddr] = pos;  
}


ush HashTable::GetPrevMatch(ush& matchHead)
{
	matchHead = _prev[matchHead & HASH_MASK];
	return matchHead;
}


void HashTable::UpdateTable()
{
	for (ush i = 0; i < HASH_SIZE; i++)
	{
		if (_head[i] < WSIZE)
			_head[i] = 0;
		else
			_head[i] -= WSIZE;
	}
	for (ush i = 0; i < HASH_SIZE; i++)
	{
		if (_prev[i] < WSIZE)
			_prev[i] = 0;
		else
			_prev[i] -= WSIZE;
	}
}