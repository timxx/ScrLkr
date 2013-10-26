
struct LOCKDATA
{
	LOCKDATA()
	{
		fAlt = FALSE;
		fCtl = fSft = TRUE;
		SecureZeroMemory(key, 4);
		SecureZeroMemory(password, 19);
	}

	BOOL fAlt;
	BOOL fCtl;
	BOOL fSft;
	TCHAR key[4];

	TCHAR password[19];
};