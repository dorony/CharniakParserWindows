
#ifndef MECSTRING_H
#define MECSTRING_H

#define ECS gnu

#if ECS == gnu
using namespace std;
#include <string>
#define ECString wstring
#else
#include <bstring.h>
#define ECString string
#endif

#endif	/* ! MECSTRING_H */
#define Char wchar_t
#define atoi _wtoi
#define strcpy wcscpy
#define strchr wcschr

#define ostream wostream
#define istream wistream
#define ifstream wifstream
#define ofstream wofstream
#define istringstream wistringstream

#define cin wcin
#define cout wcout
#define cerr wcerr
#define sprintf wsprintf

