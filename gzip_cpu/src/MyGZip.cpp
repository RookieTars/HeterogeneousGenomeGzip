#include "MyGZip.h"
#include <cassert>
#include <map>
#include <chrono>
using namespace std::chrono;

/*
 * 保证一次窗口最后匹配的长度满足MAX_MATCH
 * 保证匹配MAX_MATCH后，更新哈希表还需MIN_MATCH个字符计算哈希地址
*/
const static ush MIN_LOOKAHEAD = MIN_MATCH + MAX_MATCH + 1;    //先行缓冲区最小值
const static ush MAX_MATCHDIST = WSIZE - MIN_LOOKAHEAD;    //最远匹配距离




MyGZip::MyGZip()
	: _pWin(new uch[2 * WSIZE])
{
	_LiteralLengthBuf.reserve(BUFF_SIZE);
	_DistBuf.reserve(BUFF_SIZE);
	_FlagBuf.reserve(BUFF_SIZE / 8);

	_LiteralLengthInfo.resize(286);    //字符-长度编码数组：0-255 256 256-285
	for (size_t i = 0; i < 286; ++i)
	{
		_LiteralLengthInfo[i]._elem = i;
		_LiteralLengthInfo[i]._code = 0;
		_LiteralLengthInfo[i]._codeLength = 0;
		_LiteralLengthInfo[i]._appearCount = 0;
	}
	_DistInfo.resize(30);    //距离编码数组：0-29
	for (size_t i = 0; i < 30; ++i)
	{
		_DistInfo[i]._elem = i;
		_DistInfo[i]._code = 0;
		_DistInfo[i]._codeLength = 0;
		_DistInfo[i]._appearCount = 0;
	}
}

// 析构
MyGZip::~MyGZip()
{
	delete[] _pWin;
	_pWin = nullptr;
}

// 生成压缩文件路径
string MyGZip::buildConFilePath(const string& filePath)
{
	//string FilePath = GetFilePreFix(filePath);
	string FilePath =filePath+ ".gzip";
	return FilePath;
}

// 生成解压缩文件路径
string MyGZip::buildDeConFilePath(const string& filePath, const  string& filePostFix)
{
	string FilePath = GetFilePreFix(filePath);
	FilePath += "_gzip.";
	FilePath += filePostFix;
	return FilePath;
}


// 压缩接口
void MyGZip::Deflate(string& filePath)
{
	/* LZ77压缩思想 */

	// 1、以二进制方式打开文件
	auto begin = high_resolution_clock::now();

	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "open error" << endl;
		return;
	}

	// 2、计算文件大小，判断是否需压缩
	fseek(fIn, 0, SEEK_END);
	ull fileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);
	if (fileSize <= MIN_MATCH)
	{
		cout << "The file size is smaller than the compression range" << endl;
		fclose(fIn);
		return;
	}

	

	// 4、通过MinMatch-1（前2个）字节计算初始哈希地址
	ush hashAddr = 0;   //哈希地址
	ush matchHead = 0;  //该字符的匹配链头
	for (uch i = 0; i < MIN_MATCH - 1; i++)
	{	
		_ht.InsertString(hashAddr, _pWin[i], i, matchHead);  // 这里调用InsertString函数只是为了初始化哈希地址
	}

	// 5、更新哈希表，并找到最长匹配，生成压缩数据
	// a.定义压缩文件
	string ConFilePath = buildConFilePath(filePath);
	fOut = fopen(ConFilePath.c_str(), "wb");      //压缩数据文件

	// 写入压缩信息
	string filePostFix = GetFilePostFix(filePath);
	fwrite(filePostFix.c_str(), filePostFix.size(), 1, fOut);    //添加源文件后缀
	fputc('\n', fOut);


	// 3、初始化先行缓冲区，读取一个窗口的数据
	ull lookAhead = fread(_pWin, 1, 2 * WSIZE, fIn);    //lookAhead：先行缓冲区大小

	// b.处理先行缓冲区数据
	uch bitInfo = 0;     //标记文件缓冲区
	uch bitCount = 0;    //标记文件缓冲区内容大小
	ush start = 0;       //插入哈希表的字符在窗口的下标
	while (lookAhead != 0)
	{
		ush matchLength = 0;    //匹配长度 matchLength[3,258]
		ush matchDist = 0;      //匹配距离

		// b-1.更新哈希表
		matchHead = 0;  //每次循环，插入新的三字符组，需重置匹配链头
		_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);   //更新matchHead，即当前三字符组在_prev的匹配链头
			
		// b-2.查找最长匹配
		if (matchHead)  //有匹配，需查找最长匹配
		{
			matchLength = LongestMatch(matchHead, start, matchDist, lookAhead);
		}
		
		// b-3.根据匹配情况添加缓冲区
		if (matchLength < MIN_MATCH)  //无匹配字符，直接添加源字符至缓冲区
		{
			lookAhead--;
			SaveLZ77Info(_pWin[start], 0, bitInfo, bitCount, lookAhead);
			start++;
		}
		else  //有最长匹配字符，添加<长度，距离>对至缓冲区
		{	
			lookAhead -= matchLength;
			SaveLZ77Info(matchLength - 3, matchDist, bitInfo, bitCount, lookAhead);

			// 匹配的字符串剩余部分需添加至哈希表
			matchLength -= 1;
			start += 1;
			while (matchLength--)
			{
				_ht.InsertString(hashAddr, _pWin[start + 2], start, matchHead);
				start++;
			}
		}
		// b-4.刷新窗口
		//cout << !feof(fIn) << endl;
		if (!feof(fIn)) {
			if (lookAhead <= MIN_LOOKAHEAD)
			{
				refreshWinow(fIn, lookAhead, start);
			}
		}
		else {
			_isLast = true;
				
		}

	}

	if (bitCount > 0 && bitCount < 8)    //bitInfo最后可能不够8比特位导致数据残留
	{
		_isLast = true;
		bitInfo <<= (8 - bitCount);
		_FlagBuf.push_back(bitInfo);
		HfCompress();
	}

	if (_isLast)
	{
		fclose(fOut);
		fclose(fIn);
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - begin);
		cout << "cost: " << duration.count() << " milliseconds" << endl;
		cout << "Compression is successful" << endl;
	}
}

// 查找最长匹配接口
ush MyGZip::LongestMatch(ush matchHead, ush start, ush& matchDist,ull lookAhead)
{
	ush matchLength = 0;
	
	int maxMatchCount = 256;    
	ush limit = start > MAX_MATCHDIST ? start - MAX_MATCHDIST : 0;    

	do{
		ush curMatchLength = 0;  //本次匹配长度（最大匹配长度258）

		uch* pstart = _pWin + start;      //指向先行缓冲区的待匹配字符
		uch* pend = pstart + MAX_MATCH;   //匹配限制 [pstart, pend)最大查找范围
		uch* pbegin = _pWin + matchHead;  //指向查找缓冲区的匹配字符
		while (pstart < pend && pbegin < pstart && *pstart == *pbegin)
		{
			pstart++;
			pbegin++;
			curMatchLength++;
		}
		if (curMatchLength > matchLength)
		{
			matchLength = curMatchLength;
			matchDist = start - matchHead;
		}
	} while ((_ht.GetPrevMatch(matchHead) > limit) && --maxMatchCount != 0);

	// 防御性编程
	if (matchDist > MAX_MATCHDIST)
		matchLength = 0;
	if(matchLength > lookAhead)
		matchLength=lookAhead;
	return matchLength;
}

// 保存LZ77压缩结果的缓冲区
void MyGZip::SaveLZ77Info(uch matchLength, ush matchDist, uch& bitInfo, uch& bitCount, ull lookAhead)
{
	// matchDist：距离（0-32768）
	// matchDist：0，matchLength传入原字符
	// matchDist：>0，matchLength传入长度（0-255）
	_LiteralLengthBuf.push_back(matchLength);    //传入字符/长度缓冲区

	bitInfo <<= 1;
	

	if (matchDist > 0)  
	{
		bitInfo |= 1;
		_DistBuf.push_back(matchDist);    
	}

	bitCount++;
	if (bitCount == 8)
	{
		_FlagBuf.push_back(bitInfo);    
		bitInfo = 0;
		bitCount = 0;
	}

	if (BUFF_SIZE == _LiteralLengthBuf.size())
	{
		if (bitCount > 0 && bitCount < 8)   
		{
			bitInfo <<= (8 - bitCount);
			_FlagBuf.push_back(bitInfo);
		
		}


		HfCompress();    
	}
}

void MyGZip::refreshWinow(FILE* fIn, ull& lookAhead, ush& start)
{
	if (start < WSIZE + MAX_MATCHDIST)    
		return;

	memcpy(_pWin, _pWin + WSIZE, WSIZE);
	start -= WSIZE;

	_ht.UpdateTable();

	if (!feof(fIn))
	{
		lookAhead += fread(_pWin + WSIZE, 1, WSIZE, fIn);
	}
	//cout << feof(fIn) << endl;
}

/* GZIP解压缩接口
 * 将huffman解压缩和LZ77解压缩结合
*/

void MyGZip::UNDeflate(string& filePath)
{
	if (GetFilePostFix(filePath) != "gzip")
	{
		cout << "The compression format does not match" << endl;
		return;
	}

	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "open error" << endl;
		return;
	}

	// 2、获取源文件后缀
	string filePostFix = GetLine(fIn);

	// 3、解压
	string DeFilePath = buildDeConFilePath(filePath, filePostFix);
	FILE* fOut = fopen(DeFilePath.c_str(), "wb");   
	FILE* fcur = fopen(DeFilePath.c_str(), "rb");

	while (true)
	{
		_isLast = (bool)fgetc(fIn);
		
		GetCodeInfo(fIn);
		vector<DeCodeTableNode> LiteralLengthTable;
		GetDeCodeTable(LiteralLengthTable, _LiteralLengthInfo);

		vector<DeCodeTableNode> DistTable;
		GetDeCodeTable(DistTable, _DistInfo);
		uch ch = 0;
		uch bitCount = 0;

		while (true)
		{
			ush elem = UNCompressElem(fIn, LiteralLengthTable, _LiteralLengthInfo, ch, bitCount);    //获取一个元素
			if (elem < 256)    
			{
				fputc(elem, fOut);
			}
			else if (elem == 256)    
			{
				break;
			}
			else  
			{

				size_t index_Interval = elem - 257;   

				uch extraCodeLen = lengthInterval[index_Interval].extraBit;  
				uint extraCode = 0;
				while (extraCodeLen--)
				{
					GetBit(fIn, extraCode, ch, bitCount);
				}

				// 得到真正长度 length[3-258]
				ush length = lengthInterval[index_Interval].interval[0] + extraCode;

				/* （2）获取二元组的dist [1-32768] */
				index_Interval = UNCompressElem(fIn, DistTable, _DistInfo, ch, bitCount);    //获取到距离区间码数组下标

				// 从fIn获取额外区间码
				extraCodeLen = distInterval[index_Interval].extraBit;
				extraCode = 0;
				while (extraCodeLen--)
				{
					GetBit(fIn, extraCode, ch, bitCount);
				}

				ush dist = distInterval[index_Interval].interval[0] + extraCode;

				fflush(fOut);
				fseek(fcur, 0 - dist, SEEK_END);    

				while (length--)
				{
					uch ch = fgetc(fcur);
					fputc(ch, fOut);
					fflush(fOut);  
				}
			}
		}

		if (_isLast)    
			break;
	}

	fclose(fIn);
	fclose(fOut);
	fclose(fcur);

	cout << "Decompressing successfully" << endl;
}

// 构建符号-位长表（直接更新在Info信息数组中即可）
void MyGZip::GetCodeInfo(FILE* fIn)
{
	
	_LiteralLengthInfo.clear();
	_DistInfo.clear();

	for (size_t i = 0; i < 286; ++i)    
	{
		uch codeLen = fgetc(fIn);
		if (0 != codeLen)   
		{
			hfNodeInfo elem;
			elem._elem = i;
			elem._codeLength = codeLen;
			_LiteralLengthInfo.push_back(elem);
		}
	}
	for (size_t i = 0; i < 30; ++i)    
	{
		uch codeLen = fgetc(fIn);
		if (0 != codeLen)
		{
			hfNodeInfo elem;
			elem._elem = i;
			elem._codeLength = codeLen;
			_DistInfo.push_back(elem);
		}
	}
	
	sort(_LiteralLengthInfo.begin(), _LiteralLengthInfo.end());
	sort(_DistInfo.begin(), _DistInfo.end());
}

// 构架解码表
void MyGZip::GetDeCodeTable(vector<DeCodeTableNode>& table, vector<hfNodeInfo>& elemInfo)
{
	std::map<uch, ush>m;
	for (auto& e : elemInfo)
	{
		m[e._codeLength]++;
	}

	
	size_t table_index = 0;  
	for (auto& e : m)
	{
		DeCodeTableNode deNode;
		deNode._deCodeLen = e.first;
		deNode._codeCount = e.second;
		
		if (0 == table_index)    
		{
			deNode._code = 0;
			deNode._index = 0;
		}
		else    
		{
			DeCodeTableNode& prevNode = table[table_index - 1];
			deNode._code = (prevNode._code + prevNode._codeCount) << (deNode._deCodeLen - prevNode._deCodeLen);
			deNode._index = prevNode._index + prevNode._codeCount;
		}

		table.push_back(deNode);
		table_index++;
	}
}

// 根据两表解压一个元素（原字符0-255、结束标志256、长度区间码257-285、距离区间码0-29）
ush MyGZip::UNCompressElem(FILE* fIn, vector<DeCodeTableNode>& table, vector<hfNodeInfo>& elemInfo, uch& ch, uch& bitCount)
{
	
	ush index_table = 0;                  //解码表下标
	uch codeLen = table[0]._deCodeLen;    //编码长度
	uint code = 0;                        //编码
	while (codeLen--)    
	{
		GetBit(fIn, code, ch, bitCount);
	}
	ush num = 0;
	while ((num = code - table[index_table]._code) >= table[index_table]._codeCount)    //遍历解码表
	{
		index_table++;
		ush otherCodeLen = table[index_table]._deCodeLen - table[index_table - 1]._deCodeLen;
		while (otherCodeLen--)   
		{
			GetBit(fIn, code, ch, bitCount);
		}
	}

	num += table[index_table]._index;   
	return elemInfo[num]._elem;    
}

// 获取一个比特位的编码
void MyGZip::GetBit(FILE* fIn, uint& code, uch& ch, uch& bitCount)
{
	if (bitCount == 0)
	{
		ch = fgetc(fIn);
		bitCount = 8;
	}
	code <<= 1;
	if (ch & 0x80)
		code |= 1;
	ch <<= 1;
	bitCount--;
}


// huffman压缩接口
void MyGZip::HfCompress()
{
	// 0、清空上次的Info信息数组
	ClearInfo();

	// 1、统计源文件中每个字节的出现次数
	StatAppearCount();

	// 2、用统计的结果创建哈夫曼树
	HuffmanTree<hfNodeInfo> LitrealLengthTree(_LiteralLengthInfo, hfNodeInfo());    //字符、长度树
	HuffmanTree<hfNodeInfo> DistTree(_DistInfo, hfNodeInfo());    //距离树

	// 3、获取编码位长（每个叶子节点的高度）
	GetCodeLen(LitrealLengthTree.GetRoot(), _LiteralLengthInfo);
	GetCodeLen(DistTree.GetRoot(), _DistInfo);

	// 4、生成哈夫曼编码
	GetHfCode(_LiteralLengthInfo);
	GetHfCode(_DistInfo);

	// 5、为压缩文件添加压缩信息（编码位长）
	AddCodeLen(fOut);

	// 6、使用编码对缓冲文件进行改写，并添加至压缩文件
	uch bitInfo = 0;
	uch bitCount = 0;
	ush index_FlagBuf = 0;
	ush index_DistBuf = 0;
	uch compressBitInfo = 0;
	uch compressBitCount = 0;

	for (size_t i = 0; i < _LiteralLengthBuf.size(); ++i)
	{
		if (bitCount == 0)    
		{
			bitInfo = _FlagBuf[index_FlagBuf++];
			bitCount = 8;
		}
		if (bitInfo & 0x80)   
		{
			CompressLength_Dist(_LiteralLengthBuf[i], _DistBuf[index_DistBuf++], compressBitInfo, compressBitCount);
		}
		else   
		{
			CompressLiteral(_LiteralLengthBuf[i], compressBitInfo, compressBitCount);
		}
		bitInfo <<= 1;
		bitCount--;
	}
	
	CompressLiteral(256, compressBitInfo, compressBitCount);

	if (compressBitCount > 0 && compressBitCount < 8)
	{
		compressBitInfo <<= (8 - compressBitCount);
		fputc(compressBitInfo, fOut);
	}

	
	_LiteralLengthBuf.clear();
	_DistBuf.clear();
	_FlagBuf.clear();
}

// 清空Info信息数组
void MyGZip::ClearInfo()
{
	for (auto& e : _LiteralLengthInfo)
	{
		e._appearCount = 0;
		e._code = 0;
		e._codeLength = 0;
	}
	for (auto& e : _DistInfo)
	{
		e._appearCount = 0;
		e._code = 0;
		e._codeLength = 0;
	}
}


// 统计字节出现次数
void MyGZip::StatAppearCount()
{
	uch bitInfo = 0;
	uch bitCount = 0;
	ush index_FlagBuf = 0;
	ush index_DistBuf = 0;

	for (size_t i = 0; i < _LiteralLengthBuf.size(); ++i)
	{
		if (bitCount == 0)    
		{
			bitInfo = _FlagBuf[index_FlagBuf++];
			bitCount = 8;
		}
		if (bitInfo & 0x80)    
		{
			_LiteralLengthInfo[GetIndexLengthCode(_LiteralLengthBuf[i])]._appearCount++;    
			_DistInfo[GetIndedDistCode(_DistBuf[index_DistBuf++])]._appearCount++;    
		}
		else    
		{
			_LiteralLengthInfo[_LiteralLengthBuf[i]]._appearCount++;   
		}
		bitInfo <<= 1;
		bitCount--;
	}
	_LiteralLengthInfo[256]._appearCount = 1;    //块结束标志
}

// 获取长度区间码 matchLength[0-255]+3 -> 区间码[257-285]
ush MyGZip::GetIndexLengthCode(ush matchLength)
{
	matchLength += 3;    
	size_t sz = sizeof(lengthInterval) / sizeof(lengthInterval[0]);
	for (size_t i = 0; i < sz; ++i)
	{
		if (matchLength >= lengthInterval[i].interval[0] &&
			matchLength <= lengthInterval[i].interval[1])
		{
			return lengthInterval[i].code;
		}
	}
	assert(0);
	return -1;
}


ush MyGZip::GetIndedDistCode(ush matchDist)
{
	size_t sz = sizeof(distInterval) / sizeof(distInterval[0]);
	for (size_t i = 0; i < sz; ++i)
	{
		if (matchDist >= distInterval[i].interval[0] &&
			matchDist <= distInterval[i].interval[1])
		{
			return distInterval[i].code;
		}
	}
	assert(0);
	return -1;
}


void MyGZip::GetCodeLen(HuffmanTreeNode<hfNodeInfo>* root, vector<hfNodeInfo>& elemInfo)
{
	uch len = 0;
	GetCodeLen(root, elemInfo, len);
}
void MyGZip::GetCodeLen(HuffmanTreeNode<hfNodeInfo>* root, vector<hfNodeInfo>& elemInfo, uch len)
{
	if (root == nullptr)
		return;
	if (nullptr == root->_left && nullptr == root->_right)
	{
		elemInfo[root->_weight._elem]._codeLength = len;
		return;
	}
	len++;
	GetCodeLen(root->_left, elemInfo, len);
	GetCodeLen(root->_right, elemInfo, len);
}


void MyGZip::GetHfCode(vector<hfNodeInfo>& elemInfo)
{
	vector<hfNodeInfo> temp(elemInfo);    
	sort(temp.begin(), temp.end());


	size_t index = 0;
	for (; index < temp.size(); ++index)
	{
		if (temp[index]._appearCount > 0)
			break;
	}
	assert(index < temp.size());

	temp[index]._code = 0;    
	elemInfo[temp[index]._elem]._code = 0;
	
	size_t levelCount = 1;    
	for (size_t i = index + 1; i < temp.size(); ++i)
	{
		if (temp[i]._codeLength == temp[i - 1]._codeLength)    
		{
			temp[i]._code = temp[i - 1]._code + 1;    
			elemInfo[temp[i]._elem]._code = elemInfo[temp[i-1]._elem]._code + 1;
			levelCount++;
		}
		else    
		{
	
			temp[i]._code = (temp[i - levelCount]._code + levelCount) << (temp[i]._codeLength - temp[i - 1]._codeLength);    //该行代码仅做测试（可略）
			elemInfo[temp[i]._elem]._code = (elemInfo[temp[i - levelCount]._elem]._code + levelCount) << (elemInfo[temp[i]._elem]._codeLength - elemInfo[temp[i - 1]._elem]._codeLength);
			
			levelCount = 1;
		}
	}
}


void MyGZip::AddCodeLen(FILE* fOut)
{
	if (_isLast)
		fputc(1, fOut);    
	else
		fputc(0, fOut);

	for (size_t i = 0; i < _LiteralLengthInfo.size(); ++i)   
		fputc(_LiteralLengthInfo[i]._codeLength, fOut);
	
	for (size_t i = 0; i < _DistInfo.size(); ++i)    
		fputc(_DistInfo[i]._codeLength, fOut);
}


void MyGZip::CompressLiteral(ush byte, uch& compressBitInfo, uch& compressBitCount)
{
	uint code = _LiteralLengthInfo[byte]._code;    
	uch codeLen = _LiteralLengthInfo[byte]._codeLength;    

	WriteInfo(code, codeLen, compressBitInfo, compressBitCount);    
}


void MyGZip::CompressLength_Dist(ush length, ush dist, uch& compressBitInfo, uch& compressBitCount)
{
	//长度
	int index = GetIndexLengthCode(length);
	uint code = _LiteralLengthInfo[index]._code;
	uch codeLen = _LiteralLengthInfo[index]._codeLength;
	WriteInfo(code, codeLen, compressBitInfo, compressBitCount);
	code = length + 3 - lengthInterval[index - 257].interval[0];
	codeLen = lengthInterval[index - 257].extraBit;
	WriteInfo(code, codeLen, compressBitInfo, compressBitCount);

	// 距离
	index = GetIndedDistCode(dist);
	code = _DistInfo[index]._code;
	codeLen = _DistInfo[index]._codeLength;
	WriteInfo(code, codeLen, compressBitInfo, compressBitCount);
	code = dist - distInterval[index].interval[0];
	codeLen = distInterval[index].extraBit;
	WriteInfo(code, codeLen, compressBitInfo, compressBitCount);
}


void MyGZip::WriteInfo(uint code, uch codeLen, uch& compressBitInfo, uch& compressBitCount)
{
	code <<= (32 - codeLen);    
	for (uch i = 0; i < codeLen; ++i)
	{
		compressBitInfo <<= 1;
		if (code & 0x80000000)
			compressBitInfo |= 1;

		compressBitCount++;
		if (compressBitCount == 8)
		{
			fputc(compressBitInfo, fOut);
			compressBitCount = 0;
			compressBitInfo = 0;
		}
		code <<= 1;    
	}
		
}