//========================================================================================================
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
//========================================================================================================
#include <Windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <windowsx.h>
#include <Tlhelp32.h>

#include "WinMain.h"
#include "UnlockDlg.h"
#include "resource.h"
#include "../keybrdHook/kbHook.h"
//#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
#pragma comment(lib, "../Debug/keybrdHook.lib")
#else
#pragma comment(lib, "../Release/keybrdHook.lib")
#endif
//====================================================================

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE, LPTSTR, int nCmdShow)
{
	MSG msg = {0};

	CreateMutex(NULL, FALSE, szClsName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	if (!Register(hInst))
	{
		MessageBox(NULL, _T("Failed to register window!"), _T("System Error"), MB_ICONERROR);
		return 0;
	}
	if (!InitWnd())
	{
		MessageBox(NULL, _T("Failed to create window!"), _T("System Error"), MB_ICONERROR);
		return 0;
	}

	EnableDebugPrivilege();

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (/*msg.hwnd == hEditPw && */msg.message == WM_KEYDOWN)
			SendMessage(GetParent(hEditPw), WM_KEYDOWN, msg.wParam, msg.lParam);

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

ATOM Register(HINSTANCE hInst)
{
	WNDCLASSEX wcex;

	RtlSecureZeroMemory(&wcex, sizeof(WNDCLASSEX));

	ghInst = hInst;

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hbrBackground	= (HBRUSH) (COLOR_WINDOW+1);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon			= LoadIcon(ghInst, MAKEINTRESOURCE(IDI_APP));
	wcex.hIconSm		= LoadIcon(ghInst, MAKEINTRESOURCE(IDI_APP));
	wcex.hInstance		= ghInst;
	wcex.lpfnWndProc	= WndProc;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szClsName;
	wcex.style			= CS_OWNDC | CS_VREDRAW | CS_HREDRAW;

	return RegisterClassExW(&wcex);
}

BOOL InitWnd()
{
	HWND hWnd = CreateWindowEx(0, szClsName, _T("Screen Lock"),
		WS_OVERLAPPED | WS_SYSMENU,
		500, 300, WND_WIDTH, WND_HEIGHT,
		0, NULL, ghInst, (LPVOID)0);

	if (!hWnd)	return FALSE;

	RestoreWindowPos(hWnd, _T("mainWindow"));

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{

	case WM_CREATE:
		DoCreate(hWnd);
		break;

	case WM_COMMAND:
		DoCommand(hWnd, LOWORD(wParam), HIWORD(wParam));
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
			SendMessage(hBtnLock, BM_CLICK, 0, 0);
		break;

	case WM_CTLCOLORSTATIC:
		//SetBkColor((HDC)wParam, GetParentBkColor(hWnd));
		{
			LOGFONT lf;
			HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

			GetObject(hf, sizeof lf, &lf);
			lstrcpy(lf.lfFaceName, _T("宋体"));

			HFONT hFont = CreateFontIndirect(&lf);
			HDC hdc = (HDC)wParam;
			SelectObject(hdc, hFont);
		}
		return (long)GetStockBrush(NULL_BRUSH);

	case WM_MOUSEMOVE:
	case WM_NCMOUSEMOVE:
		if (!fCaptured && fLocked)
		{
			SetCapture(hWnd);
			ShowCursor(FALSE);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;

	case WM_DESTROY:
		SaveWindowPos(hWnd, _T("mainWindow"));
		LockNow(hWnd, false);
		HideMouse(false);
		PostQuitMessage(0);
		return 0;

	case MM_MATCH:
		{
			HideMouse(false);

			LOCKDATA ld;
			if (!ReadLockData(ld))
			{
				MessageBox(hWnd, _T("悲剧发生，无法读取配置文件！！！"), _T("错误"), MB_ICONERROR);
				return 0;
			}
			if (lstrcmpi(ld.password, _T(""))==0)
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			else
			{
				if (!fShowedUnlock)
				{
					fShowedUnlock = true;
					DialogBoxParam(ghInst, MAKEINTRESOURCE(IDD_UNLOCK), hWnd, UnlockDlgProc, 0);
					HideMouse(true);
					fShowedUnlock = false;
				}
			}
		}
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void DoCreate(HWND hWnd)
{
	//InitCommonControls();
	int x = 10;
	int y = 10;

	hwndStatic1 =  CreateWindow(_T("STATIC"), _T("解锁热键"), \
		WS_VISIBLE | WS_CHILD, 
		x, y, 65, 20, hWnd, NULL, ghInst, 0);

	y += 22;
	hwndAlt = CreateWindow(_T("BUTTON"), _T("Alt + "), \
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 
		x, y, 50, 25, hWnd, (HMENU)CB_ALT_ID, ghInst, 0);

	x += 50;
	hwndCtl = CreateWindow(_T("BUTTON"), _T("Ctrl + "), \
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 
		x, y, 55, 25, hWnd, (HMENU)CB_CTL_ID, ghInst, 0);

	x += 55;
	hwndSft = CreateWindow(_T("BUTTON"), _T("Shift + "), \
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 
		x, y, 65, 25, hWnd, (HMENU)CB_SFT_ID, ghInst, 0);

	x += 65;
	hwndKey = CreateWindow(_T("COMBOBOX"), _T(""), 
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, 60, 100, hWnd, (HMENU)CB_KEY_ID, ghInst, 0);

	x = 10;
	y += 35;

	hwndStatic2 =  CreateWindow(_T("STATIC"), _T("解锁密码"), \
		WS_VISIBLE | WS_CHILD, 
		x, y, 65, 20, hWnd, NULL, ghInst, 0);

	y += 22;
	hEditPw = CreateWindow(_T("EDIT"), _T(""), \
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | ES_PASSWORD,\
		x, y, 120, 25, hWnd, (HMENU)EDIT_PW_ID, ghInst, 0);

	x = WND_WIDTH - BTN_WIDTH - 15;
	y = WND_HEIGHT - BTN_HEIGHT - 30;

	hBtnLock = CreateWindow(_T("BUTTON"), _T("Lock Now!"),\
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, x, y,\
		BTN_WIDTH, BTN_HEIGHT, hWnd, (HMENU)BTN_LOCK_ID, ghInst, 0);

	SendMessage(hEditPw, EM_SETLIMITTEXT, 19, 0);
	SetFocus(hEditPw);
	InitKey();
}

void DoCommand(HWND hWnd, UINT uID, UINT uNotifyCode)
{
	switch(uID)
	{
	case BTN_LOCK_ID:
		{
			LOCKDATA ld;

			GetWindowText(hEditPw, ld.password, 19);

			if (lstrcmp(ld.password, _T(""))==0)
			{
				int sel = MessageBox(hWnd, _T("您尚未设置密码，这使得解锁不安全。是否返回设置密码？\r\n")
					_T("注：不设置密码则解锁时不用输入密码直接解开"),
					_T("密码为空"), MB_ICONQUESTION | MB_YESNO);
				if (sel == IDYES)
				{
					SetFocus(hEditPw);
					return ;
				}
			}

			ld.fAlt = Button_GetCheck(hwndAlt)==BST_CHECKED ? TRUE : FALSE;
			ld.fCtl = Button_GetCheck(hwndCtl)==BST_CHECKED ? TRUE : FALSE;
			ld.fSft = Button_GetCheck(hwndSft)==BST_CHECKED ? TRUE : FALSE;

			int index = SendMessage(hwndKey, CB_GETCURSEL, 0, 0);
			SendMessage(hwndKey, CB_GETLBTEXT, index, (LPARAM)ld.key);

			if (!WriteLockData(ld))
			{
				MessageBox(hWnd, _T("写配置文件时失败！这将导致无法锁屏！"),
					_T("错误"), MB_ICONERROR);
				return ;
			}

			LockNow(hWnd);
			HideMouse();
			fLocked = true;
			ShowWindow(hWnd, SW_HIDE);
		}
		break;
	}
}

void LockNow(HWND hWnd, bool fLock /* = true */)
{
	if (fLock)
	{
		if (!InstallHook(hWnd, TRUE))
		{
			MessageBox(hWnd, _T("Hook keybord failed!"), _T("Error"), MB_ICONERROR);
		}
	}
	else
	{
		InstallHook(hWnd, FALSE);
	}
}

void InitKey()
{
	for (TCHAR c = _T('A'); c<=_T('Z'); c++)
	{
		TCHAR ss[2] = {0};
		ss[0] = c;
		SendMessage(hwndKey, CB_ADDSTRING, 0, (LPARAM)ss);
	}
	for (TCHAR c = _T('0'); c<=_T('9'); c++)
	{
		TCHAR ss[2] = {0};
		ss[0] = c;
		SendMessage(hwndKey, CB_ADDSTRING, 0, (LPARAM)ss);
	}

	for (int i = 1; i<=12; i++)
	{
		TCHAR ss[4] = {0};
		wsprintf(ss, _T("F%d"), i);
		SendMessage(hwndKey, CB_ADDSTRING, 0, (LPARAM)ss);
	}

	LOCKDATA ld;
	if (ReadLockData(ld))
	{
		SendMessage(hwndAlt, BM_SETCHECK, ld.fAlt?BST_CHECKED:BST_UNCHECKED, 0);
		SendMessage(hwndCtl, BM_SETCHECK, ld.fCtl?BST_CHECKED:BST_UNCHECKED, 0);
		SendMessage(hwndSft, BM_SETCHECK, ld.fSft?BST_CHECKED:BST_UNCHECKED, 0);

		int index = SendMessage(hwndKey, CB_FINDSTRING, -1, (LPARAM)ld.key);
		if (index != CB_ERR)
			SendMessage(hwndKey, CB_SETCURSEL, index, 0);

		SetWindowText(hEditPw, ld.password);
	}
	else
	{
		SendMessage(hwndCtl, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hwndSft, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hwndKey, CB_SETCURSEL, 38, 0);
	}
}

bool GetIniFilePath(TCHAR *lpszPath, int maxBuf/* = MAX_PATH*/)
{
	GetModuleFileName(ghInst, lpszPath, maxBuf);
	if (!PathRemoveFileSpec(lpszPath))
		return false;

	lstrcat(lpszPath, _T("\\config.ini"));

	return true;
}

BOOL WriteLockData(const LOCKDATA &ld)
{
	TCHAR iniFile[MAX_PATH] = {0};

	if (!GetIniFilePath(iniFile))
		return false;

	return WritePrivateProfileStruct(_T("Screen Lock"), _T("data"),
		(LPVOID)&ld, sizeof(LOCKDATA), iniFile);
}

BOOL ReadLockData(LOCKDATA &ld)
{
	TCHAR iniFile[MAX_PATH] = {0};

	if (!GetIniFilePath(iniFile))
		return false;

	return GetPrivateProfileStruct(_T("Screen Lock"), _T("data"), &ld, sizeof(LOCKDATA), iniFile);
}

void SaveWindowPos(HWND hWnd, const TCHAR *key)
{
	WINDOWPLACEMENT wpl = {0};
	wpl.length = sizeof(WINDOWPLACEMENT);
	if (!GetWindowPlacement(hWnd, &wpl))
		return ;

	TCHAR iniFile[MAX_PATH];
	if (GetIniFilePath(iniFile))
		WritePrivateProfileStruct(_T("Screen Lock"), key, &wpl, sizeof(WINDOWPLACEMENT), iniFile);
}

void RestoreWindowPos(HWND hWnd, const TCHAR *key)
{
	TCHAR iniFile[MAX_PATH];
	if (!GetIniFilePath(iniFile))
		return ;

	WINDOWPLACEMENT wpl = {0};
	wpl.length = sizeof(WINDOWPLACEMENT);

	if (!GetPrivateProfileStruct(_T("Screen Lock"), key, &wpl, sizeof(WINDOWPLACEMENT), iniFile))
		return ;

	if (wpl.showCmd == SW_HIDE)
		wpl.showCmd = SW_SHOWNORMAL;

	SetWindowPlacement(hWnd, &wpl);
}

bool EnableDebugPrivilege()
{
	HANDLE hToken;
	HANDLE hProcess = GetCurrentProcess();

	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) )
		return false;

	TOKEN_PRIVILEGES tkp;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid) )
		return false;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0))
	{
		CloseHandle(hToken);
		return false;
	}
	CloseHandle(hToken);
	return true;
}

void HideMouse(bool fHide/*=true*/)
{
	if (fHide)
	{
		if (hThread)	return ;

		hThread = CreateThread(NULL, NULL, HideThread, 0, 0, &dwID);
	}
	else
	{
		if (hThread)
		{
			TerminateThread(hThread, 0);
			hThread = NULL;
		}
	}
}

DWORD WINAPI HideThread(LPVOID lParam)
{
	while (1)
	{
		SetCursorPos(0, 0);
		TerminateTaskMgr();

		Sleep(10);
	}

	return 1;
}

void TerminateTaskMgr()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (!hSnapshot)
		return ;

	PROCESSENTRY32 pe = {0};

	pe.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe))
		return ;

	bool bFound = false;

	do
	{
		if (lstrcmpi(pe.szExeFile, _T("taskmgr.exe"))==0)
		{
			bFound = true;
			break;
		}
	}while(Process32Next(hSnapshot, &pe));

	CloseHandle(hSnapshot);

	if (bFound)
	{
// 		MessageBox(0, L"found!",L"test'", 0);
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
		TerminateProcess(hProcess, 0);
	}
}