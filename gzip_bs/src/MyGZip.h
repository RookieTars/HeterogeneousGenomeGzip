#ifndef __MYGZIP_H__
#define __MYGZIP_H__

#include "HashTable.h"
#include "HuffmanTree.hpp"
#include <thread>
#include <mutex>
#include"energysampler/energysampler.h"
struct IntervalSolution
{
	ush code;
	uch extraBit;
	ush interval[2];
};

/*************************************************************/
static IntervalSolution distInterval[] = {
	{0, 0, {1, 1}},
	{1, 0, {2, 2}},
	{2, 0, {3, 3}},
	{3, 0, {4, 4}},
	{4, 1, {5, 6}},
	{5, 1, {7, 8}},
	{6, 2, {9, 12}},
	{7, 2, {13, 16}},
	{8, 3, {17, 24}},
	{9, 3, {25, 32}},
	{10, 4, {33, 48}},
	{11, 4, {49, 64}},
	{12, 5, {65, 96}},
	{13, 5, {97, 128}},
	{14, 6, {129, 192}},
	{15, 6, {193, 256}},
	{16, 7, {257, 384}},
	{17, 7, {385, 512}},
	{18, 8, {513, 768}},
	{19, 8, {769, 1024}},
	{20, 9, {1025, 1536}},
	{21, 9, {1537, 2048}},
	{22, 10, {2049, 3072}},
	{23, 10, {3073, 4096}},
	{24, 11, {4097, 6144}},
	{25, 11, {6145, 8192}},
	{26, 12, {8193, 12288}},
	{27, 12, {12289, 16384}},
	{28, 13, {16385, 24576}},
	{29, 13, {24577, 32768}}};

//[257-285]
static IntervalSolution lengthInterval[] = {
	{257, 0, {3, 3}},
	{258, 0, {4, 4}},
	{259, 0, {5, 5}},
	{260, 0, {6, 6}},
	{261, 0, {7, 7}},
	{262, 0, {8, 8}},
	{263, 0, {9, 9}},
	{264, 0, {10, 10}},
	{265, 1, {11, 12}},
	{266, 1, {13, 14}},
	{267, 1, {15, 16}},
	{268, 1, {17, 18}},
	{269, 2, {19, 22}},
	{270, 2, {23, 26}},
	{271, 2, {27, 30}},
	{272, 2, {31, 34}},
	{273, 3, {35, 42}},
	{274, 3, {43, 50}},
	{275, 3, {51, 58}},
	{276, 3, {59, 66}},
	{277, 4, {67, 82}},
	{278, 4, {83, 98}},
	{279, 4, {99, 114}},
	{280, 4, {115, 130}},
	{281, 5, {131, 162}},
	{282, 5, {163, 194}},
	{283, 5, {195, 226}},
	{284, 5, {227, 257}},
	{285, 0, {258, 258}}};

struct hfNodeInfo
{
	ush _elem;
	ush _appearCount;
	uint _code;
	uch _codeLength;

	hfNodeInfo(size_t appearCount = 0)
		: _appearCount(appearCount)
	{
		_code = 0;
		_codeLength = 0;
	}

	hfNodeInfo operator+(const hfNodeInfo &other) const
	{
		return hfNodeInfo(_appearCount + other._appearCount);
	}
	bool operator>(const hfNodeInfo &other) const
	{
		return _appearCount > other._appearCount;
	}
	bool operator==(const hfNodeInfo &other) const
	{
		return _appearCount == other._appearCount;
	}

	bool operator<(const hfNodeInfo &other) const
	{
		if (_codeLength == other._codeLength)
			return _elem < other._elem;
		else
			return _codeLength < other._codeLength;
	}
};

struct DeCodeTableNode
{
	uch _deCodeLen;
	uint _code;
	ush _codeCount;
	ush _index;
};

/*************************************************************/

class MyGZip
{
public:
	MyGZip();

	~MyGZip();

	void Deflate(string &filePath);

	void UNDeflate(string &filePath);

private:
	string buildConFilePath(const string &filePath);

	string buildDeConFilePath(const string &filePath, const string &filePostFix);

	/*******************************************************/
	/* LZ77 */

	ush LongestMatch(ush matchHead, ush start, ush &matchDist, ull lookAhead);

	void SaveLZ77Info(uch matchLength, ush matchDist, uch &bitInfo, uch &bitCount, ull lookAhead);

	void refreshWinow(FILE *fIn, ull &lookAhead, ush &start);

	/* Huffman */
	void HfCompress();
	void _HfCompress();
	void ClearInfo();
	int StatAppearCount();
	ush GetIndexLengthCode(ush matchLength);
	ush GetIndedDistCode(ush matchDist);

	void GetCodeLen(HuffmanTreeNode<hfNodeInfo> *root, vector<hfNodeInfo> &elemInfo);
	void GetCodeLen(HuffmanTreeNode<hfNodeInfo> *root, vector<hfNodeInfo> &elemInfo, uch len);

	void GetHfCode(vector<hfNodeInfo> &elemInfo);
	void AddCodeLen(FILE *fOut);

	void CompressLiteral(ush byte, uch &compressBitInfo, uch &compressBitCount);
	void CompressLength_Dist(ush length, ush dist, uch &compressBitInfo, uch &compressBitCount);
	void WriteInfo(uint code, uch codeLen, uch &compressBitInfo, uch &compressBitCount);

	void GetCodeInfo(FILE *fIn);
	void GetDeCodeTable(vector<DeCodeTableNode> &table, vector<hfNodeInfo> &elemInfo);
	ush UNCompressElem(FILE *fIn, vector<DeCodeTableNode> &table, vector<hfNodeInfo> &elemInfo, uch &ch, uch &bitCount);
	void GetBit(FILE *fIn, uint &code, uch &ch, uch &bitCount);

private:
	/* GZIP */

	const ush BUFF_SIZE = 0x8000;
	bool _isLast = false;
	FILE *fOut = nullptr;

	/* LZ77 */
	uch *_pWin;
	HashTable _ht;

	vector<uch> _LiteralLengthBuf[2];
	vector<ush> _DistBuf[2];
	vector<uch> _FlagBuf[2];
	/* Huffman */

	int lz77Buffer = 0;
	std::mutex changeBuffer;

	vector<hfNodeInfo> _LiteralLengthInfo;
	vector<hfNodeInfo> _DistInfo;

	vector<std::thread> threads;
	cpu_set_t cpu_set[3];
	energySampler e;
};

#endif