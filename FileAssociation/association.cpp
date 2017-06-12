#include <Windows.h>
#include <tchar.h>
#include "resource.h"

HWND    hWinMain = NULL;

TCHAR    szKeyEnter[] = TEXT("bkpfile");
TCHAR    szKeyExt1[] = TEXT(".bkp");
TCHAR    szKeyExt2[] = TEXT("bkpfile//shell//open//command");
TCHAR    szParam[] = TEXT(" \"%1\"");


TCHAR    szDelSub1[] = TEXT("bkpfile//shell//open");
TCHAR    szDelSub2[] = TEXT("bkpfile//shell");


void SetAssociate();
void DelAssociate();
BOOL IsAssociate();

int _tmain(int argc, TCHAR *argv[])
{
	if (argc < 1)
		return -1;
	if (_tcscmp(argv[0], _T("-is")) == 0)
	{
		_tprintf(IsAssociate() ? _T("true") : _T("false"));
		return 0;
	}
	else if (_tcscmp(argv[0], _T("-set")) == 0)
	{
		SetAssociate();
	}
	else if (_tcscmp(argv[0], _T("-del")) == 0)
	{
		DelAssociate();
	}
}

void SetAssociate()
{
	HKEY    hKey;

	if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, szKeyExt1, &hKey))
	{
		RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *)szKeyEnter, sizeof(szKeyEnter));
		RegCloseKey(hKey);
	}

	if (ERROR_SUCCESS == RegCreateKey(HKEY_CLASSES_ROOT, szKeyExt2, &hKey))
	{
		TCHAR    szFileName[MAX_PATH] = _T("\"");
		GetModuleFileName(NULL, szFileName + 1, sizeof(szFileName) - 1 * sizeof(TCHAR));
		(_tcsrchr(szFileName, _T('\\')))[1] = 0; //ɾ���ļ�����ֻ���·��
		_tcscat(szFileName, _T("BKE_Creator.exe"));
		_tcscat(szFileName, _T("\""));
		_tcscat(szFileName, szParam);
		RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE *)szFileName, (_tcslen(szFileName) + 1) * sizeof(TCHAR));
		RegCloseKey(hKey);
	}
}

void DelAssociate()
{
	HKEY hKey;
	RegDeleteKey(HKEY_CLASSES_ROOT, szKeyExt1);
	// NT��ֻ��һ��һ���ɾ��
	// testfile//shell//open//command
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szDelSub1, 0, KEY_WRITE, &hKey))
	{
		RegDeleteKey(hKey, _T("command")); // ɾ��command��һ��
		RegCloseKey(hKey);
	}
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szDelSub2, 0, KEY_WRITE, &hKey))
	{
		RegDeleteKey(hKey, _T("open")); // ɾ��open��һ��
		RegCloseKey(hKey);
	}
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyEnter, 0, KEY_WRITE, &hKey))
	{
		RegDeleteKey(hKey, _T("shell")); // ɾ��shell��һ��
		RegCloseKey(hKey);
	}
	RegDeleteKey(HKEY_CLASSES_ROOT, szKeyEnter); // ɾ��testfile��һ��
}

BOOL IsAssociate()
{
	HKEY    hKey;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szKeyExt1, 0, KEY_WRITE, &hKey))
	{
		RegCloseKey(hKey);
		return TRUE;
	}
	return FALSE;
}