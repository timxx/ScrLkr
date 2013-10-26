//====================================================================
#ifndef _WIN_MAIN_H
#define _WIN_MAIN_H
//====================================================================
#include "data.h"
//====================================================================
#define MM_MATCH WM_USER + 101

#define BTN_LOCK_ID	100
#define EDIT_PW_ID	101

#define CB_ALT_ID	102
#define CB_CTL_ID	103
#define CB_SFT_ID	104

#define CB_KEY_ID	105

#define WND_WIDTH	255
#define WND_HEIGHT	150

#define BTN_WIDTH	100
#define BTN_HEIGHT	50
//====================================================================
const TCHAR szClsName[] = _T("ScreenLock");
HINSTANCE ghInst = NULL;

HWND hBtnLock = NULL;
HWND hEditPw = NULL;
HWND hwndAlt = NULL,
	 hwndCtl = NULL,
	 hwndSft = NULL;
HWND hwndKey = NULL;
HWND hwndStatic1 = NULL,
	 hwndStatic2 = NULL;

bool fShowedUnlock = false;
bool fCaptured = false;
bool fLocked = false;

HANDLE hThread = NULL;
DWORD dwID = 0;
//====================================================================
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM Register(HINSTANCE hInst);
BOOL InitWnd();
void DoCreate(HWND hWnd);
void DoCommand(HWND hWnd, UINT uID, UINT uNotifyCode);
void LockNow(HWND hWnd, bool fLock = true);
void InitKey();
bool GetIniFilePath(TCHAR *lpszPath, int maxBuf = MAX_PATH);
BOOL WriteLockData(const LOCKDATA &ld);
BOOL ReadLockData(LOCKDATA &ld);
void SaveWindowPos(HWND hWnd, const TCHAR *key);
void RestoreWindowPos(HWND hWnd, const TCHAR *key);
bool EnableDebugPrivilege();
void HideMouse(bool fHide=true);
DWORD WINAPI HideThread(LPVOID lParam);
void TerminateTaskMgr();
//====================================================================

#endif