#ifndef __COMMON_H__
#define __COMMON_H__

typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long long ull;
typedef unsigned int uint;

/* TODO:LZ77 */
#define MIN_MATCH 3
#define MAX_MATCH 258
static const ush WSIZE = 1 << 15;
#include <iostream>
using namespace std;
#include <vector>
using std::vector;

#include <string.h>
// using std::string;

#include <queue>
using std::queue;

#include <iostream>
using std::cin;
using std::cout;
using std::endl;

string GetFilePostFix(const string &filePath);
string GetFilePreFix(const string &filePath);
string GetFileName(const string &filePath);
string GetLine(FILE *fIn);

#pragma warning(disable : 4996)
#endif