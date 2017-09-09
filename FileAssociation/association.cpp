#include <Windows.h>
#include <tchar.h>
#include <shlobj.h>
#include "resource.h"

HWND    hWinMain = NULL;

TCHAR    szKeyEnter[] = TEXT("bkpfile");
TCHAR    szKeyExt1[] = TEXT(".bkp");
TCHAR    szKeyExt2[] = TEXT("bkpfile\\shell\\open\\command");
TCHAR    szParam[] = TEXT(" \"%1\"");


TCHAR    szDelSub1[] = TEXT("bkpfile\\shell\\open");
TCHAR    szDelSub2[] = TEXT("bkpfile\\shell");


void SetAssociate();
void DelAssociate();
BOOL IsAssociate();

int _tmain(int argc, TCHAR *argv[])
{
	if (argc < 2)
		return -1;
	if (_tcscmp(argv[1], _T("-is")) == 0)
	{
		_tprintf(IsAssociate() ? _T("true") : _T("false"));
		return 0;
	}
	else if (_tcscmp(argv[1], _T("-set")) == 0)
	{
		SetAssociate();
	}
	else if (_tcscmp(argv[1], _T("-del")) == 0)
	{
		DelAssociate();
	}
}

void SetAssociate()
{
	HKEY    hKey;
	LSTATUS status;

	status = RegCreateKey(HKEY_CLASSES_ROOT, szKeyExt1, &hKey);

	if (ERROR_SUCCESS == status)
	{
		RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *)szKeyEnter, sizeof(szKeyEnter));
		RegCloseKey(hKey);
	}

	status = RegCreateKey(HKEY_CLASSES_ROOT, szKeyExt2, &hKey);

	if (ERROR_SUCCESS == status)
	{
		TCHAR    szFileName[MAX_PATH] = _T("\"");
		GetModuleFileName(NULL, szFileName + 1, sizeof(szFileName) - 1 * sizeof(TCHAR));
		(_tcsrchr(szFileName, _T('\\')))[1] = 0; //删除文件名，只获得路径
		_tcscat(szFileName, _T("BKE_Creator.exe"));
		_tcscat(szFileName, _T("\""));
		_tcscat(szFileName, szParam);
		RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *)szFileName, (_tcslen(szFileName) + 1) * sizeof(TCHAR));
		RegCloseKey(hKey);
	}
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
}

void DelAssociate()
{
	HKEY hKey;
	RegDeleteKey(HKEY_CLASSES_ROOT, szKeyExt1);
	// NT下只能一层一层的删除
	// testfile//shell//open//command
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szDelSub1, 0, KEY_WRITE, &hKey))
	{
		RegDeleteKey(hKey, _T("command")); // 删除command这一层
		RegCloseKey(hKey);
	}
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szDelSub2, 0, KEY_WRITE, &hKey))
	{
		RegDeleteKey(hKey, _T("open")); // 删除open这一层
		RegCloseKey(hKey);
	}
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyEnter, 0, KEY_WRITE, &hKey))
	{
		RegDeleteKey(hKey, _T("shell")); // 删除shell这一层
		RegCloseKey(hKey);
	}
	RegDeleteKey(HKEY_CLASSES_ROOT, szKeyEnter); // 删除testfile这一层
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
}

BOOL IsAssociate()
{
	HKEY    hKey;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyExt1, 0, KEY_QUERY_VALUE, &hKey))
	{
		DWORD len;
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, 0, NULL, NULL, &len))
		{
			RegCloseKey(hKey);
			return len != 0 ? TRUE : FALSE;
		}
		RegCloseKey(hKey);
	}
	return FALSE;
}