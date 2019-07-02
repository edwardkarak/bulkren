#include "bulkren.h"
// append add to path, inserting a path separator in between
WCHAR *appendToPath(const WCHAR *path, const WCHAR *add)
{
	WCHAR *ret = new WCHAR[wcslen(path) + wcslen(add) + 2];
	wcscpy(ret, path);
	wcscat(ret, L"\\");
	wcscat(ret, add);

	return ret;
}

// get radix prefix, i.e, 0x or 0o based on the radix
const WCHAR *getRadixPrefix(int radix)
{
	switch (radix) {
		case 16:
			return L"0x";
		case 8:
			return L"0o";
	}
	return L"";
}

// get radix format string, i.e x for hexadecimal or o for octal
const WCHAR *getRadixFmtStr(int radix)
{
	switch (radix) {
		case 10:
			return L"d";
		case 16:
			return L"x";
		case 8:
			return L"o";
	}
	return L"Invalid radix";
}

// generate a filename guaranteed to be unique. Caller must free return value.
WCHAR *gettempname()
{
	WCHAR *ret = new WCHAR[MAX_PATH];
	char *tmpname = tmpnam(NULL);

	MultiByteToWideChar(CP_ACP, 0, tmpname, -1, ret, MAX_PATH);

	wcscpy(ret, wcsrchr(ret, L'\\') + 1);
	return ret;
}

WCHAR *lowercase(const WCHAR *str)
{
	WCHAR *buf = new WCHAR[wcslen(str) + 1];
	wcscpy(buf, str);

	return _wcslwr(buf);
}

WCHAR *getExt(const WCHAR *path)
{
	const WCHAR *ext = wcsrchr(path, L'.');
	if (ext == NULL)
		return L"";
	return (WCHAR *) ext;
}

bool extsEqual(const WCHAR *path1, const WCHAR *path2)
{
	path1 = lowercase(path1);
	path2 = lowercase(path2);

	const WCHAR *ext1, *ext2;
	ext1 = wcsrchr(path1, L'.');
	ext2 = wcsrchr(path2, L'.');

	delete[]path1;
	delete[]path2;

	// if one extension, but not the other, is null, the extensions can't be equal
	if ((ext1 == NULL && ext2 != NULL) || (ext1 != NULL && ext2 == NULL))
		return false;

	// neither file has an extension, so they are "equal"
	if (ext1 == NULL && ext2 == NULL)
		return true;

	return wcscmp(ext1, ext2) == 0;
}

// count no. of files in dirpath that match ext. If ext is NULL, count no. of files in dirpath. Dirs omitted.
int countfiles(const WCHAR *dirpath, const WCHAR *ext)
{
	int n = 0;
	for (const auto &entry : std::filesystem::directory_iterator(dirpath)) {
		if (entry.is_directory())
			continue;
		if (ext) {
			if (extsEqual(entry.path().c_str(), ext))
				++n;
		}
		else
			++n;
	}
	return n;
}

int calcndigits(int x, int radix)
{
	switch (radix) {
		case 10:
			return (int) ceil(log10(x));
		case 16:
			return (int) ceil(log(x) / log(16));	// log change of base
		case 8:
			return (int) ceil(log(x) / log(8));
	}
	return 0;
}

// return the format string based on the pattern string given by the user. ndigits is the number of digits of padding to provide. 
// If it is 0, this number is deduced from the pattern.
WCHAR *getfmtstr(const WCHAR *pat, int ndigits, int radix)
{
	int iPat, iFmt;
	bool seenPounds = false;

	WCHAR *fmt = new WCHAR[2*wcslen(pat)];
	for (iPat = iFmt = 0; pat[iPat]; ) {
		if (pat[iPat] == '#' && !seenPounds) {
			int k;
			for (k = iPat; pat[k] == '#'; ++k)
				;
			seenPounds = true;
			int patDigits = ndigits ? ndigits : (k - iPat);
			int fmtWritten = _swprintf(fmt + iFmt, L"%s%%0%d%s", getRadixPrefix(radix), patDigits, getRadixFmtStr(radix));

			if (ndigits != 0)
				for (; pat[iPat] == '#'; ++iPat)
					;
			else
				iPat += patDigits;

			iFmt += fmtWritten;
		}
		if (pat[iPat] == '#' && seenPounds)
			throw extraPounds { };
		else
			fmt[iFmt++] = pat[iPat++];
	}

	fmt[iFmt] = 0;
	return fmt;
}

int bulkren(const WCHAR *dirpath, const WCHAR *pat, unsigned flags)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	int nfile = 0;
	WCHAR *dirpathCopy;
	int ndigits = 0;
	int radix = 10;

	if (flags & OPT_HEX)
		radix = 16;
	if (flags & OPT_OCT)
		radix = 8;

	int nfiles = countfiles(dirpath, flags & OPT_ONLYEXT ? getExt(pat) : NULL);
	if (nfiles == 0)
		return 0;

	if (flags & OPT_AUTOCALC)
		ndigits = calcndigits(nfiles, radix);

	dirpathCopy = appendToPath(dirpath, L"*");
	hFind = FindFirstFile(dirpathCopy, &ffd);
	delete[]dirpathCopy;

	if (INVALID_HANDLE_VALUE == hFind)
		return -1;

	// ndigits says how many digits of padding to format (ignore pound signs in pattern).
	// If 0, then getfmtstr will not ignore the pound signs in the pattern.

	WCHAR *fmt = getfmtstr(pat, ndigits, radix);

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		if (flags & OPT_ONLYEXT && !extsEqual(ffd.cFileName, fmt))
			continue;
		else {
			WCHAR newname[MAX_PATH];
			_snwprintf(newname, MAX_PATH, fmt, ++nfile);

			WCHAR *oldname, *fullNewName;
			oldname = appendToPath(dirpath, ffd.cFileName);
			fullNewName = appendToPath(dirpath, newname);

			if (MoveFile(oldname, fullNewName) == 0)
				return -1;

			delete[]oldname;
			delete[]fullNewName;
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	delete[]fmt;
	return nfile;
}
