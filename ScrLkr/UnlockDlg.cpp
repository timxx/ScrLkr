
#include <Windows.h>
#include <tchar.h>

#include "UnlockDlg.h"
#include "resource.h"
#include "data.h"

extern BOOL ReadLockData(LOCKDATA &ld);
extern void LockNow(HWND hWnd, bool fLock = true);
extern void SaveWindowPos(HWND hWnd, const TCHAR *key);
extern void RestoreWindowPos(HWND hWnd, const TCHAR *key);

BOOL CALLBACK UnlockDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RestoreWindowPos(hDlg, _T("unlockWindow"));
			SendDlgItemMessage(hDlg, IDC_EDIT1, EM_SETLIMITTEXT, 19, 0);
			RECT rect;
			GetWindowRect(hDlg, &rect);
			//SetCursorPos(rect.left, rect.top);
			ClipCursor(&rect);
		}
		return TRUE;

	case WM_COMMAND:
		{
			SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON1, 0);
			if (LOWORD(wParam) == IDC_BUTTON1)
			{
				TCHAR szPw[19] = {0};
				GetDlgItemText(hDlg, IDC_EDIT1, szPw, 19);

				LOCKDATA ld;
				if (ReadLockData(ld))
				{
					if (lstrcmp(szPw, ld.password)==0)
					{
						SaveWindowPos(hDlg, _T("unlockWindow"));
						LockNow(GetParent(hDlg), false);
						PostMessage(hDlg, WM_CLOSE, 0, 0);
						SendMessage(GetParent(hDlg), WM_CLOSE, 0, 0);
					}
					else
					{
						SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
						SendDlgItemMessage(hDlg, IDC_EDIT1, EM_SETSEL, 0, -1);
					}
				}
			}
		}
		break;

	case WM_CLOSE:
		SaveWindowPos(hDlg, _T("unlockWindow"));
		EndDialog(hDlg, IDCANCEL);
		return TRUE;
	}

	return FALSE;
}