//====================================================================
#include <Windows.h>
#include <tchar.h>
//====================================================================
#define HOOKAPI extern "C" __declspec(dllexport)
#include "kbHook.h"

#include "../../ScrLkr/ScrLkr/data.h"
#include "../../ScrLkr/ScrLkr/resource.h"
//====================================================================
#define MM_MATCH WM_USER + 101

extern HWND ghWnd;
extern HINSTANCE ghInst;

static HHOOK ghHook;
//====================================================================
BOOL ReadLockData(LOCKDATA &ld);
bool GetIniFilePath(TCHAR *lpszPath, int maxBuf = MAX_PATH);
UINT Str2Key(const TCHAR * key);
//====================================================================
LRESULT CALLBACK KeyboardHook(int nCode, WORD wParam, DWORD lParam)
{
	KBDLLHOOKSTRUCT *pkh = (KBDLLHOOKSTRUCT*)lParam;

	if (nCode == HC_ACTION)
	{
		BOOL fAlt	= (GetAsyncKeyState(VK_MENU)<0)?TRUE:FALSE;//(pkh->flags & LLKHF_ALTDOWN);
		BOOL fCtrl	= (GetAsyncKeyState(VK_CONTROL)<0)?TRUE:FALSE;
		BOOL fShift = (GetAsyncKeyState(VK_SHIFT)<0)?TRUE:FALSE;

		LOCKDATA ld;

		DWORD code = pkh->vkCode;
		if ((code == VK_ESCAPE ) ||
			(code == VK_TAB/* && fAlt*/) ||
		//	(code == VK_ESCAPE/* && fAlt*/) ||
			(code == VK_LWIN) ||
			(code == VK_RWIN)
			)
			return 1;

		HWND hwndFg = GetForegroundWindow();
		HWND hwndUnlock = FindWindow(_T("#32770"), _T("ÇëÊäÈë½âËøÃÜÂë"));
		if (hwndFg == hwndUnlock)
		{
			HWND hEdit = GetDlgItem(hwndUnlock, IDC_EDIT1);
			if (hEdit == GetFocus())
			{
			//	DWORD code = pkh->vkCode;
			//	if ((code>='a' && code<='z') ||
			//		(code>='A' && code<='Z') ||
			//		(code>='0' && code <='9'))
					return CallNextHookEx(ghHook, nCode, wParam, lParam);
			}

		}
		if (ReadLockData(ld))
		{
			UINT uKey = Str2Key(ld.key);

			BOOL fMatched = (fAlt == ld.fAlt &&
				fCtrl == ld.fCtl &&
				fShift == ld.fSft &&
				pkh->vkCode == uKey);

			if (fMatched)
			{
				PostMessage(ghWnd, MM_MATCH, 0, 0);
			}
		}
		return 1;
	}
	return CallNextHookEx(ghHook, nCode, wParam, lParam);
}

BOOL ReadLockData(LOCKDATA &ld)
{
	TCHAR iniFile[MAX_PATH] = {0};

	if (!GetIniFilePath(iniFile))
		return false;

	return GetPrivateProfileStruct(_T("Screen Lock"), _T("data"), &ld, sizeof(LOCKDATA), iniFile);
}

bool GetIniFilePath(TCHAR *lpszPath, int maxBuf/* = MAX_PATH*/)
{
	GetModuleFileName((HINSTANCE)GetWindowLong(ghWnd, GWL_HINSTANCE), lpszPath, maxBuf);

	lstrcpy(_tcsrchr(lpszPath, '\\')+1, _T("\\config.ini"));

	return true;
}

UINT Str2Key(const TCHAR * key)
{
	/* 0~9 A~Z */
	if (lstrlen(key) == 1)
		return key[0];

	/* F1~F9 */
	else if (lstrlen(key) == 2)
		return VK_F1 + (key[1] - '0') - 1;

	/* F10~F12 */
	else if (lstrlen(key) == 3)
		return VK_F10 + key[2] - '0';

	return -1;
}

HOOKAPI BOOL InstallHook(HWND hWnd, BOOL fInstall)
{
	BOOL nReturn = TRUE;
	ghWnd = hWnd;
//	if (hWnd == NULL)	MessageBox(0, L"NULL hWnd", L"test", 0);

	if (fInstall)
	{
		ghHook = (HHOOK)SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC)KeyboardHook, ghInst, 0);

		if (!ghHook)
		{
//			MessageBox(ghWnd, L"hook failed", L"test", 0);
			return FALSE;
		}
	}
	else
	{
		nReturn = UnhookWindowsHookEx(ghHook);
	}

	return nReturn;
}