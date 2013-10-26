
#include <Windows.h>

#pragma data_seg("Shared")
HWND ghWnd = NULL;
HINSTANCE ghInst = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:Shared,rws")


BOOL WINAPI DllMain(HINSTANCE hDllInst, DWORD dwReason, LPVOID _Reserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		ghInst = hDllInst;
		
		break;
	}
	return TRUE;
}