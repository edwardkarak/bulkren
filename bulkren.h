#ifndef BULKREN_H
#define BULKREN_H

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <math.h>
#include <filesystem>

// flags for bulkren final argument
const unsigned OPT_ONLYEXT = 2;
const unsigned OPT_AUTOCALC = 4;
const unsigned OPT_HEX = 8;
const unsigned OPT_OCT = 16;

class extraPounds : public std::logic_error {
public:
	extraPounds() : std::logic_error { "Another series of pound signs was encountered. Press OK to revise the pattern." } { };
};

// convert every character in str to lowercase equivalent, and return the result. Caller must free return value
WCHAR *lowercase(const WCHAR *str);

// return the extension for the file indicated in path.
WCHAR *getExt(const WCHAR *path);

// return true if the extension in path1 equals that in path2
bool extsEqual(const WCHAR *path1, const WCHAR *path2);

// return the #files in dirpath whose extension is ext. 
// If ext is NULL, return the nuber of files in dirpath. 
// In either case, directories are ignored.
int countfiles(const WCHAR *dirpath, const WCHAR *ext = NULL);

// return the #digits in x, whose base is given by radix.
int calcndigits(int x, int radix = 10);

// rename all files in dirpath according to pattern given by pat. #files renamed successfully is returned. -1 is returned on error.
int bulkren(const WCHAR *dirpath, const WCHAR *pat, unsigned flags);

#endif