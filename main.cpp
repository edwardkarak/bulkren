#define UNICODE
#define _UNICODE

#include <shlobj.h>
#include <strsafe.h>

#include "bulkren.h"
#include "resource.h"

void errormsg(LPWSTR lpszFunction);

std::wstring loadStr(UINT idStr)
{
	const wchar_t* p = nullptr;
	int len = LoadString(nullptr, idStr, reinterpret_cast<LPWSTR>(&p), 0);
	if (len > 0) {
		auto x = std::wstring { p, static_cast<size_t>(len) };
		return x;
	}

	throw std::exception { };
}


// display messagebox with information about the last Win32 error.
void errormsg(LPWSTR lpszFunction)
{
	LPVOID lpMsgBuf, lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR) &lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID) LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCWSTR) lpMsgBuf) + lstrlen((LPCWSTR) lpszFunction) + 40) * sizeof(WCHAR));

	StringCchPrintf((LPWSTR) lpDisplayBuf,
					LocalSize(lpDisplayBuf) / sizeof(WCHAR),
					loadStr(IDS_WIN32ERROR).c_str(),
					lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCWSTR) lpDisplayBuf, NULL, MB_ICONSTOP);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

// display formatted message box
int msgbox(HWND hwnd, const std::wstring &caption, const std::wstring fmt, ...)
{
	WCHAR buf[1024];
	va_list ap;

	va_start(ap, fmt);
	_vsnwprintf(buf, sizeof(buf) / sizeof(WCHAR), fmt.c_str(), ap);

	va_end(ap);
	return MessageBox(hwnd, buf, caption.c_str(), 0);
}

// display dialog allowing user to select a directory
void choosedir(HWND hwnd, WCHAR *dirpath)
{
	BROWSEINFO bInfo;
	bInfo.hwndOwner = hwnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = dirpath;

	std::wstring title = loadStr(IDS_CHOOSEFOLDER);
	bInfo.lpszTitle = title.c_str();

	bInfo.ulFlags = 0;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
	if (lpItem != NULL) {
		SHGetPathFromIDList(lpItem, dirpath);

		std::wstring newText { loadStr(IDS_CHOSENDIR) };
		newText += dirpath;
		SetDlgItemText(hwnd, IDL_CHOSENDIR, newText.c_str());
	}
}

INT_PTR CALLBACK frameproc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
	static WCHAR dir[MAX_PATH];
	static bool dirchosen;
	static bool patgiven;

	switch (wm) {
		case WM_CLOSE:
			EndDialog(hwnd, IDCLOSE);
			return TRUE;
		case WM_INITDIALOG: {
			std::wstring text = loadStr(IDS_CHOSENDIR);
			SetDlgItemText(hwnd, IDL_CHOSENDIR, text.c_str());
			SendDlgItemMessage(hwnd, IDRAD_DEC, BM_SETCHECK, BST_CHECKED, 0);
			SendMessage(hwnd, WM_SETICON, 0,
				(LPARAM)
						LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RENAME)));
			return TRUE;
		}
		case WM_COMMAND:
			if (HIWORD(wp) == EN_CHANGE) {
				int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_PATBOX));
				patgiven = (bool) len;

				EnableWindow(GetDlgItem(hwnd, IDOK), dirchosen && patgiven);
			}

			switch (LOWORD(wp)) {
				case IDC_CHOOSEDIR:
				{
					choosedir(hwnd, dir);
					dirchosen = true;
					break;
				}
				case IDOK:
				{
					int n = GetWindowTextLength(GetDlgItem(hwnd, IDC_PATBOX));
					WCHAR *pat = new WCHAR[n + 1];

					unsigned flags = 0;
					int radix = 10;

					if (IsDlgButtonChecked(hwnd, IDCHK_ONLYEXT))
						flags |= OPT_ONLYEXT;
					if (IsDlgButtonChecked(hwnd, IDCHK_AUTOCALC))
						flags |= OPT_AUTOCALC;
					if (IsDlgButtonChecked(hwnd, IDRAD_HEX)) {
						flags |= OPT_HEX;
						radix = 16;
					} if (IsDlgButtonChecked(hwnd, IDRAD_OCT)) {
						flags |= OPT_OCT;
						radix = 8;
					}

					GetDlgItemText(hwnd, IDC_PATBOX, pat, n + 1);

					try {
						int ret = bulkren(dir, pat, flags);
						switch (ret) {
							case -1:
								errormsg(L"bulkren");
								break;

							case 0:
								msgbox(hwnd, L"", loadStr(IDS_NOTHING_RENAMED));
								break;

							default:
								if (flags & OPT_AUTOCALC)
									msgbox(hwnd, loadStr(IDS_CAP_SUCCESS), loadStr(IDS_SUCCESS_PADDING), ret, calcndigits(ret, radix));
								else
									msgbox(hwnd, loadStr(IDS_CAP_SUCCESS), loadStr(IDS_SUCCESS), ret);
						}
					}
					catch (const std::logic_error &e) {
						MessageBoxA(hwnd, e.what(), "Warning", MB_ICONWARNING);
					}
					

					delete[]pat;
					break;
				}
				case IDC_PAT_HELP:
					MSGBOXPARAMS msgbox = { 0 };
					msgbox.cbSize = sizeof(MSGBOXPARAMS);
					msgbox.hwndOwner = hwnd;
					msgbox.hInstance = GetModuleHandle(NULL);
					msgbox.lpszText = MAKEINTRESOURCE(IDS_ABOUT);
					msgbox.lpszCaption = MAKEINTRESOURCE(IDS_CAP_ABOUT);
					msgbox.dwStyle = MB_OK | MB_USERICON;
					msgbox.lpszIcon = MAKEINTRESOURCE(IDI_RENAME);

					MessageBoxIndirect(&msgbox);
					break;
			}
			return TRUE;
	}
	return FALSE;
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
	return (int) DialogBox(hInstance, MAKEINTRESOURCE(IDD_FRAME), NULL, frameproc);
}
