#include "Common.h"

string GetFilePostFix(const string& filePath)
{
	return filePath.substr(filePath.find_last_of('.') + 1);
}
string GetFilePreFix(const string& filePath)
{
	return filePath.substr(0, filePath.find_last_of('.'));
}
string GetFileName(const string& filePath)
{
	size_t start = filePath.find_last_of('\\') + 1;
	size_t end = filePath.find_last_of('.');
	return filePath.substr(start, end - start);
}
string GetLine(FILE* fIn)
{
	string ret;
	while (!feof(fIn))
	{
		char ch = fgetc(fIn);
		if (ch == '\n')
		{
			break;
		}
		ret += ch;
	}
	return ret;
}