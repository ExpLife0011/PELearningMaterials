///////////////////////////////////////////////////////////////////////
// ˵����                                                            //
// ����һ���򵥵�PE�ļ��������ߣ�û��ʲô������������ȫ�����Žṹ��  //
// ��PE�ļ�����һЩ�������������ҽ����Ĳ��ֺ��٣�����Ŀ¼����ֻ��  //
// ���˵�����֡�                                                  //
///////////////////////////////////////////////////////////////////////
// ������Ϣ                                                          //
// ���ߣ��������                                                    //
// ���ͣ�http://www.programlife.net/                                 //
// ���䣺stackexploit@gmail.com                                      //
///////////////////////////////////////////////////////////////////////
// �ر����ѣ�                                                        //
// �������һ�����͵İ��Ʒ���ߣ����������ѧϰ����֮�ã��벻Ҫ����ʽ//
// ���ϻ�����Ҫ���ϵ�ʹ�ñ�����������ɴ���ɵ�һ�к�����߸Ų�����//
///////////////////////////////////////////////////////////////////////
// ��Ȩ����                                                          //
// �������Ȩ�����ߴ���������У�ת���뱣���������֣�                //
///////////////////////////////////////////////////////////////////////
// ��Щ�ط���Ӧ���ǰѱ༭����Ϊֻ���ģ����Ҹ�ɽ����ˣ����ø���      //
///////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <commctrl.h>
#include <Tlhelp32.h>
#include "resource.h"

#pragma comment(lib, "comctl32")

HWND			g_hWnd;
HINSTANCE		g_hInstance;
TCHAR			szSrcTitle[] = TEXT("PEViewer");
TCHAR			szDstTitle[MAX_PATH + 32] = TEXT("");
static TCHAR	szFileName[MAX_PATH];

//
DLGPROC			g_oldMailProc;
DLGPROC			g_oldBlogProc;
HWND			g_hMailWnd;
HWND			g_hBlogWnd;

DWORD			g_dwMask;		// ʱ������
DWORD			g_dwImageBase;	// �����ַ
HANDLE			g_hFile;		// �ļ����

void UnImplementationTips()
{
	MessageBox(g_hWnd,
		TEXT("�˹��ܻ�û��ʵ�� (��o��)��"),
		TEXT("��PE����"),
		MB_OK);
}

BOOL GetPeFilePath(TCHAR szFileName[])
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];
	ZeroMemory(szFile, sizeof(szFile));
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize		= sizeof(ofn);
	ofn.lpstrFile		= szFile;
	ofn.nMaxFile		= sizeof(szFile);
	ofn.lpstrFilter		= TEXT("*.exe\0*.exe\0")
						  TEXT("*.dll\0*.dll\0")
						  TEXT("All Files\0*.*\0");
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST
						  | OFN_FILEMUSTEXIST;
	ofn.hwndOwner		= g_hWnd;

	if (GetOpenFileName(&ofn))
	{
		lstrcpy(szFileName, szFile);
		return TRUE;
	}
	return FALSE;
}

void EmptyCtrlValues()
{
	DWORD dwCtrlIDStart	= 1001;
	DWORD dwCtrlIDEnd	= 1016;
	for (DWORD dwIndex = dwCtrlIDStart; dwIndex <= dwCtrlIDEnd; ++dwIndex)
	{
		SetDlgItemText(g_hWnd, dwIndex, TEXT(""));
	}
	SetWindowText(g_hWnd, szSrcTitle);
}

void SetCtrlStyles()
{
	DWORD dwCtrlIDStart	= 1001;
	DWORD dwCtrlIDEnd	= 1016;
	for (DWORD dwIndex = dwCtrlIDStart; dwIndex <= dwCtrlIDEnd; ++dwIndex)
	{
		DWORD dwStyles = GetWindowLong(
							GetDlgItem(g_hWnd, dwIndex),
							GWL_STYLE);
		dwStyles |= ES_RIGHT;
		SetWindowLong(
			GetDlgItem(g_hWnd, dwIndex),
			GWL_STYLE,
			dwStyles);
	}
	HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
	SendMessage(g_hWnd, WM_SETICON, WPARAM(ICON_BIG), (LPARAM)(hIcon));
}

void SetCtrlValues(HANDLE hFile)
{
	IMAGE_DOS_HEADER dosHeader;
	IMAGE_NT_HEADERS ntHeader;
	IMAGE_FILE_HEADER fileHeader;
	IMAGE_OPTIONAL_HEADER optionalHeader;

	TCHAR szText[256] = TEXT("");
	TCHAR szFormat8[] = TEXT("%08X");
	TCHAR szFormat4[] = TEXT("%04X");
	DWORD dwOffset = 0, dwTemp;

	lstrcpy(szDstTitle, szSrcTitle);
	lstrcat(szDstTitle, TEXT(" - "));
	lstrcat(szDstTitle, szFileName);
	SetWindowText(g_hWnd, szDstTitle);

	ReadFile(hFile, &dosHeader, sizeof(dosHeader), &dwTemp, NULL);
	if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(g_hWnd, TEXT("���ļ�������Ч��PE�ļ���(�Ҳ���MZ)"),
			TEXT("��ʾ��Ϣ"), MB_ICONWARNING);
		return ;
	}
	
	dwOffset = dosHeader.e_lfanew;
	SetFilePointer(hFile, dwOffset, 0, FILE_BEGIN);
	ReadFile(hFile, &ntHeader, sizeof(ntHeader), &dwTemp, NULL);

	fileHeader = ntHeader.FileHeader;
	optionalHeader = ntHeader.OptionalHeader;

	if (ntHeader.Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(g_hWnd, TEXT("���ļ�������Ч��PE�ļ���(�Ҳ���PE)"),
			TEXT("��ʾ��Ϣ"), MB_ICONWARNING);
		return ;
	}

	// ��ڵ�RVA
	wsprintf(szText, szFormat8, optionalHeader.AddressOfEntryPoint);
	SetDlgItemText(g_hWnd, IDC_EDIT_ENTRYPOINT, szText);
	// װ�ص�ַ
	wsprintf(szText, szFormat8, optionalHeader.ImageBase);
	SetDlgItemText(g_hWnd, IDC_EDIT_IMAGEBASE, szText);
	g_dwImageBase = optionalHeader.ImageBase;
	// �����С
	wsprintf(szText, szFormat8, optionalHeader.SizeOfImage);
	SetDlgItemText(g_hWnd, IDC_EDIT_IMAGESIZE, szText);
	// �����ַ
	wsprintf(szText, szFormat8, optionalHeader.BaseOfCode);
	SetDlgItemText(g_hWnd, IDC_EDIT_CODEBASE, szText);
	// ���ݻ�ַ
	wsprintf(szText, szFormat8, optionalHeader.BaseOfData);
	SetDlgItemText(g_hWnd, IDC_EDIT_DATABASE, szText);
	// �����
	wsprintf(szText, szFormat8, optionalHeader.SectionAlignment);
	SetDlgItemText(g_hWnd, IDC_EDIT_MEMORYALIGN, szText);
	// �ļ�����
	wsprintf(szText, szFormat8, optionalHeader.FileAlignment);
	SetDlgItemText(g_hWnd, IDC_EDIT_FILEALIGN, szText);
	// ��־��
	wsprintf(szText, szFormat4, optionalHeader.Magic);
	SetDlgItemText(g_hWnd, IDC_EDIT_MAGIC, szText);
	// ��ϵͳ
	wsprintf(szText, szFormat4, optionalHeader.Subsystem);
	SetDlgItemText(g_hWnd, IDC_EDIT_SUBSYSTEM, szText);
	// ������Ŀ
	wsprintf(szText, szFormat4, fileHeader.NumberOfSections);
	SetDlgItemText(g_hWnd, IDC_EDIT_SECTIONNUM, szText);
	// ʱ�����ڱ�־
	// ����ȫ����Ϣ
	g_dwMask = fileHeader.TimeDateStamp;
	wsprintf(szText, szFormat8, fileHeader.TimeDateStamp);
	SetDlgItemText(g_hWnd, IDC_EDIT_TIMEDATE, szText);
	// �ײ���С
	wsprintf(szText, szFormat8, optionalHeader.SizeOfHeaders);
	SetDlgItemText(g_hWnd, IDC_EDIT_HEADERSIZE, szText);
	// ����ֵ
	wsprintf(szText, szFormat4, fileHeader.Characteristics);
	SetDlgItemText(g_hWnd, IDC_EDIT_CHARACTER, szText);
	// У���
	wsprintf(szText, szFormat8, optionalHeader.CheckSum);
	SetDlgItemText(g_hWnd, IDC_EDIT_CHECKSUM, szText);
	// ��ѡͷ��С
	wsprintf(szText, szFormat4, fileHeader.SizeOfOptionalHeader);
	SetDlgItemText(g_hWnd, IDC_EDIT_OPTIONALSIZE, szText);
	// RVA������С
	wsprintf(szText, szFormat8, optionalHeader.NumberOfRvaAndSizes);
	SetDlgItemText(g_hWnd, IDC_EDIT_RVASIZE, szText);
}

// ��Ҫ���ռ��notify��������ΪTRUE
BOOL CALLBACK StaticProc(HWND hwndDlg, 
						   UINT uMsg, 
						   WPARAM wParam, 
						   LPARAM lParam
						   )
{
	static TCHAR szBuffer[256];
	static WORD	 wFlag = 0;
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
		SetCursor(LoadCursor(NULL, IDC_HAND));
		return TRUE;

	case WM_LBUTTONDOWN:
		SetCursor(LoadCursor(NULL, IDC_HAND));
		return TRUE;

	case WM_LBUTTONUP:
		GetWindowText(hwndDlg, szBuffer, 
			sizeof(szBuffer)/sizeof(szBuffer[0]));
		ShellExecute(NULL, TEXT("open"),
			szBuffer,
			NULL, NULL,
			SW_SHOWNORMAL);
		return TRUE;

	default:
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		if (hwndDlg == g_hMailWnd)
		{
			return g_oldMailProc(hwndDlg, uMsg, wParam, lParam);
		}
		else if (hwndDlg == g_hBlogWnd)
		{
			return g_oldBlogProc(hwndDlg, uMsg, wParam, lParam);
		}
		break;
	}
	return 0;
}

// ʱ����뵽��������ת������
void MaskTimeConvert(SYSTEMTIME& stTime,
					 DWORD& dwMask,
					 BOOL Mask2Time = TRUE)
{
	// GMTʱ��1970��1��1��
	SYSTEMTIME sysTime1970;
	ZeroMemory(&sysTime1970, sizeof(SYSTEMTIME));
	sysTime1970.wYear		= 1970;
	sysTime1970.wMonth		= 1;
	sysTime1970.wDay		= 1;

	LARGE_INTEGER li;
	li.QuadPart = 0;
	// Ĭ��Ϊ���뵽���ڵ�ת��
	if (Mask2Time)
	{
		// תΪ�ļ�ʱ��(100����Ϊ��λ)
		FILETIME fTime1970;
		SystemTimeToFileTime(&sysTime1970, &fTime1970);
		// ������ת��Ϊ100����ĵ�λ
		li.LowPart = dwMask;
		li.QuadPart *= 1000 * 1000 * 10;
		li.LowPart += fTime1970.dwLowDateTime;
		li.HighPart += fTime1970.dwHighDateTime;
		// ������ת���ļ�ʱ��
		FILETIME fTime;
		fTime.dwLowDateTime = li.LowPart;
		fTime.dwHighDateTime = li.HighPart;
		// ���ļ�ʱ��ת��ΪGMTʱ��
		FileTimeToSystemTime(&fTime, &stTime);

		return ;
	}
	else//Time2Mask
	{
		// תΪ�ļ�ʱ��(100����Ϊ��λ)
		FILETIME fTime1970;
		SystemTimeToFileTime(&sysTime1970, &fTime1970);
		// ������ʱ��ת��Ϊ�ļ�ʱ��
		FILETIME fTimeNow;
		SystemTimeToFileTime(&stTime, &fTimeNow);
		// ʱ���ֵ����
		fTimeNow.dwLowDateTime -= fTime1970.dwLowDateTime;
		fTimeNow.dwHighDateTime -= fTime1970.dwHighDateTime;
		// ���ļ�ʱ��ת��Ϊ����
		li.LowPart = fTimeNow.dwLowDateTime;
		li.HighPart = fTimeNow.dwHighDateTime;
		li.QuadPart /= (1000 * 1000 * 10);
		///////////////////////////////////
		// ��֪��Ϊʲô������һ�����ӵ���ʧ
		///////////////////////////////////
		li.QuadPart += 1;
		// תΪ�ַ���
		dwMask = li.LowPart;

		return ;
	}
}

BOOL CALLBACK TimeDlgProc(HWND hwndDlg, 
						   UINT uMsg, 
						   WPARAM wParam, 
						   LPARAM lParam
						   )
{
	static HWND			hRadioWnd;
	static TCHAR		szTimeMask[16];
	static DWORD		dwMask;
	static SYSTEMTIME	stTime;
	static TCHAR		szBuffer[32];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// ��ѡ��ť��ѡ��
		CheckRadioButton(hwndDlg,
			IDC_RADIO_SETMASK,
			IDC_RADIO_SETTIME,
			IDC_RADIO_SETMASK);
		// ���ÿؼ�
		SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_TIME), EM_SETREADONLY, TRUE, 0);
		//EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_TIME), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_DLGTIME_DATE), FALSE);
		// ���ÿؼ�
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_MASK), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COPYANDCLOSE), TRUE);
		// ��ȡ���Ի����ʱ����Ϣ
		GetWindowText(GetDlgItem(g_hWnd, IDC_EDIT_TIMEDATE),
			szTimeMask,
			sizeof(szTimeMask) / sizeof(szTimeMask[0]));
		// ��ʱ����Ϣ���ݸ����Ի���ı༭��ؼ�
		SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_MASK),
			szTimeMask);
		///////////////////////////////////////////////////////////
		// ������ȷʱ��ѡ��
		dwMask = g_dwMask;
		MaskTimeConvert(stTime, dwMask, TRUE);
		// ����ʱ��
		ZeroMemory(szBuffer, sizeof(szBuffer));
		wsprintf(szBuffer, TEXT("%02d:%02d:%02d"),
			stTime.wHour, stTime.wMinute, stTime.wSecond);
		SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_TIME),
			szBuffer);
		// ��������
		ZeroMemory(szBuffer, sizeof(szBuffer));
		DateTime_SetSystemtime(
			GetDlgItem(hwndDlg, IDC_DLGTIME_DATE),
			GDT_VALID,
			&stTime);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO_SETMASK:
			// ��ѡ��ť��ѡ��
			CheckRadioButton(hwndDlg,
				IDC_RADIO_SETMASK,
				IDC_RADIO_SETTIME,
				IDC_RADIO_SETMASK);
			// ���ÿؼ�
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_TIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DLGTIME_DATE), FALSE);
			// ���ÿؼ�
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_MASK), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COPYANDCLOSE), TRUE);
			///////////////////////////////////////////////
			// ʱ����Ϣת��Ϊ����
			// ��ȡ������Ϣ
			DateTime_GetSystemtime(
				GetDlgItem(hwndDlg, IDC_DLGTIME_DATE),
				&stTime);
			// ��ȡʱ����Ϣ
			GetDlgItemText(hwndDlg, IDC_EDIT_DLGTIME_TIME,
				szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));
			if (szBuffer[0] >= '0' && szBuffer[0] <= '9')
			{
				stTime.wHour = (szBuffer[0] - '0') * 10 + (szBuffer[1] - '0');
				stTime.wMinute = (szBuffer[3] - '0') * 10 + (szBuffer[4] - '0');
				stTime.wSecond = (szBuffer[6] - '0') * 10 + (szBuffer[7] - '0');
				// ����ת��
				MaskTimeConvert(stTime, dwMask, FALSE);
				// תΪ�ַ���
				ZeroMemory(szBuffer, sizeof(szBuffer));
				wsprintf(szBuffer, TEXT("%08X"), dwMask);
				// �ж��Ƿ�仯
				TCHAR szBuffer2[32];
				GetDlgItemText(hwndDlg, IDC_EDIT_DLGTIME_MASK,
					szBuffer2, sizeof(szBuffer2)/sizeof(szBuffer2[0]));
				if (!lstrcmp(szBuffer, szBuffer2))
				{
					return TRUE;
				}
				// ���±���
				SetDlgItemText(hwndDlg,
					IDC_EDIT_DLGTIME_MASK,
					szBuffer);
			}

			return TRUE;

		case IDC_RADIO_SETTIME:
			CheckRadioButton(hwndDlg,
				IDC_RADIO_SETMASK,
				IDC_RADIO_SETTIME,
				IDC_RADIO_SETTIME);
			// ���ÿؼ�
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_TIME), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DLGTIME_DATE), TRUE);
			// ���ÿؼ�
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_MASK), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_COPYANDCLOSE), FALSE);
			///////////////////////////////////////////////////////////
			// ��ȡʱ���ı�
			GetDlgItemText(hwndDlg, IDC_EDIT_DLGTIME_MASK,
				szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));
			// ���ı�ת��Ϊʮ����������
			dwMask = 0;
			for (int i = 0; i < lstrlen(szBuffer); ++i)
			{
				dwMask *= 16;
				if (szBuffer[i] >= '0' && szBuffer[i] <= '9')
				{
					dwMask += szBuffer[i] - '0';
				}
				else if (szBuffer[i] >= 'A' && szBuffer[i] <= 'F')
				{
					dwMask += szBuffer[i] - 'A' + 10;
				}
				else
				{
					MessageBox(hwndDlg, TEXT("���ݸ�ʽ����"),
						TEXT("����"),
						MB_ICONERROR);
					SetDlgItemText(hwndDlg, IDC_EDIT_DLGTIME_MASK,
						TEXT(""));
					return TRUE;
				}
			}
			// �������
			g_dwMask = dwMask;
			// ������ȷʱ��ѡ��
			MaskTimeConvert(stTime, dwMask, TRUE);
			// ����ʱ��
			ZeroMemory(szBuffer, sizeof(szBuffer));
			wsprintf(szBuffer, TEXT("%02d:%02d:%02d"),
				stTime.wHour, stTime.wMinute, stTime.wSecond);
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_DLGTIME_TIME),
				szBuffer);
			// ��������
			ZeroMemory(szBuffer, sizeof(szBuffer));
			DateTime_SetSystemtime(
				GetDlgItem(hwndDlg, IDC_DLGTIME_DATE),
				GDT_VALID,
				&stTime);
			return TRUE;

		case IDC_BTN_COPYANDCLOSE:
			// ���Ƶ�������
			if (OpenClipboard(hwndDlg) && EmptyClipboard())
			{
				//��ȡ����
				HGLOBAL hMem;
				hMem = GlobalAlloc(GMEM_MOVEABLE,
					(lstrlen(szTimeMask) + sizeof(szTimeMask[0])) * sizeof(szTimeMask[0]));
				PVOID pBuff = (PVOID)GlobalLock(hMem);
				memcpy(pBuff, szTimeMask, 
					(lstrlen(szTimeMask) + sizeof(szTimeMask[0])) * sizeof(szTimeMask[0]));
				GlobalUnlock(hMem);

				//�������ݵ�������
				SetClipboardData(CF_UNICODETEXT, hMem);

				//�رռ�����
				CloseClipboard();
			}
			// �رմ���
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			return TRUE;

		default:
			return FALSE;
		}

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

// ��������ַת��Ϊ�����ַ
void RvaToVa(DWORD& dwRva, DWORD& dwVa)
{
	dwVa = g_dwImageBase + dwRva;
}

// �����ַת��Ϊ��������ַ
void VaToRva(DWORD& dwVa, DWORD& dwRva)
{
	dwRva = dwVa - g_dwImageBase;
}

// �ļ�ƫ�Ƶ������ַ
void OffsetToVa(DWORD& dwOffset, DWORD& dwVa)
{
	IMAGE_SECTION_HEADER	sectionHeader;
	IMAGE_SECTION_HEADER	emptyHeader;
	IMAGE_DOS_HEADER		dosHeader;

	DWORD	dwLength, dwTmp;
	DWORD	dwIndex = 0;
	// ��λ���ļ���ʼ��
	SetFilePointer(g_hFile, 0, 0, FILE_BEGIN);
	// ��ȡIMAGE_DOS_HEADER
	ReadFile(g_hFile, &dosHeader, 
		sizeof(IMAGE_DOS_HEADER),
		&dwTmp,
		NULL);
	dwLength = dosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS);
	// ��λ���ļ��ڱ�ʼ��
	SetFilePointer(g_hFile, dwLength, 0, FILE_BEGIN);
	// �ж�ƫ���Ƿ��ڽڱ�֮ǰ
	if (dwOffset < dwLength)
	{
		dwVa = dwOffset + g_dwImageBase;
		return ;
	}
	// �����ڱ�
	ZeroMemory(&emptyHeader, sizeof(IMAGE_SECTION_HEADER));
	while (1)
	{
		++dwIndex;
		ReadFile(g_hFile, &sectionHeader,
			sizeof(IMAGE_SECTION_HEADER),
			&dwTmp,
			NULL);
		// ��һ������
		if (dwIndex == 1 && dwOffset < sectionHeader.PointerToRawData)
		{
			dwVa = dwOffset + g_dwImageBase;
			return ;
		}
		if (!memcmp(&emptyHeader, &sectionHeader, sizeof(IMAGE_SECTION_HEADER)))
		{
			// ����ʧ��
			dwVa = 0;
			//MessageBox(g_hWnd, TEXT("��ַ������Χ��:-)"), TEXT("��ʾ��Ϣ"), MB_ICONERROR);
			return ;
		}
		dwTmp = sectionHeader.PointerToRawData;
		if (dwOffset >= dwTmp && 
			dwOffset < dwTmp + sectionHeader.SizeOfRawData)
		{
			// ���������ַ(RVA-��ַ)
			dwVa = sectionHeader.VirtualAddress 
				+ g_dwImageBase
				+ dwOffset - dwTmp;
			return ;
		}
	}

	return ;
}

// �ļ�ƫ�Ƶ���������ַ
void OffsetToRva(DWORD& dwOffset, DWORD& dwRva)
{
	DWORD dwVa = 0;
	OffsetToVa(dwOffset, dwVa);
	VaToRva(dwVa, dwRva);
}

// �����ַ���ļ�ƫ��
void VaToOffset(DWORD& dwVa, DWORD& dwOffset)
{
	IMAGE_SECTION_HEADER	sectionHeader;
	IMAGE_SECTION_HEADER	emptyHeader;
	IMAGE_DOS_HEADER		dosHeader;

	DWORD	dwLength, dwTmp;
	DWORD	dwIndex = 0;
	// ��λ���ļ���ʼ��
	SetFilePointer(g_hFile, 0, 0, FILE_BEGIN);
	// ��ȡIMAGE_DOS_HEADER
	ReadFile(g_hFile, &dosHeader, 
		sizeof(IMAGE_DOS_HEADER),
		&dwTmp,
		NULL);
	dwLength = dosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS);
	// ��λ���ļ��ڱ�ʼ��
	SetFilePointer(g_hFile, dwLength, 0, FILE_BEGIN);
	// �ж�ƫ���Ƿ��ڽڱ�֮ǰ
	if (dwVa - g_dwImageBase < dwLength)
	{
		dwOffset = dwVa - g_dwImageBase;
		return ;
	}
	// �����ڱ�
	ZeroMemory(&emptyHeader, sizeof(IMAGE_SECTION_HEADER));
	while (1)
	{
		++dwIndex;
		ReadFile(g_hFile, &sectionHeader,
			sizeof(IMAGE_SECTION_HEADER),
			&dwTmp,
			NULL);
		if (dwIndex == 1)	// ��һ������
		{
			if (dwVa - g_dwImageBase < sectionHeader.VirtualAddress)
			{
				// ����Ҫת��
				// ƫ�Ƶ�ַ��ΪRVA
				dwOffset = dwVa - g_dwImageBase;
				return ;
			}
		}
		if (!memcmp(&emptyHeader, &sectionHeader, sizeof(IMAGE_SECTION_HEADER)))
		{
			// ����ʧ��
			dwOffset = 0;
			//MessageBox(g_hWnd, TEXT("��ַ������Χ��:-)"), TEXT("��ʾ��Ϣ"), MB_ICONERROR);
			return ;
		}
		// ��ʼVA
		DWORD dwStartVa	= sectionHeader.VirtualAddress + g_dwImageBase;
		// ����VA
		DWORD dwEndVa	= dwStartVa + sectionHeader.SizeOfRawData;
		if (dwVa >= dwStartVa && dwVa < dwEndVa)
		{
			// ���������ַ(RVA-��ַ)
			dwOffset = dwVa - g_dwImageBase		// RVA
				- sectionHeader.VirtualAddress	// ��ʼRVA
				+ sectionHeader.PointerToRawData;
			return ;
		}
	}

	return ;
}

// �ļ�ƫ�Ƶ���������ַ
void RvaToOffset(DWORD& dwRva, DWORD& dwOffset)
{
	DWORD dwVa = 0;
	RvaToVa(dwRva, dwVa);
	VaToOffset(dwVa, dwOffset);
	
}

// ����RVA��ȡ������Ϣ
void GetSectionNameByRva(DWORD dwRva, TCHAR szBuffer[])
{
	// ��ȡ�ڱ���Ϣ��ر���
	IMAGE_SECTION_HEADER	sectionHeader;
	IMAGE_SECTION_HEADER	emptyHeader;
	IMAGE_DOS_HEADER		dosHeader;

	DWORD	dwLength, dwTmp;
	DWORD	dwIndex = 0;
	int		i;

	// ��ȡ�ڱ���Ϣ
	dwIndex = 0;
	// ��λ���ļ���ʼ��
	SetFilePointer(g_hFile, 0, 0, FILE_BEGIN);
	// ��ȡIMAGE_DOS_HEADER
	ReadFile(g_hFile, &dosHeader, 
		sizeof(IMAGE_DOS_HEADER),
		&dwTmp,
		NULL);
	/////////////////////////////////////////////////////////////////////////////
	// �ж�RVA��λ���Ƿ���IMAGE_DOS_HEADER
	/////////////////////////////////////////////////////////////////////////////
	if (dwRva >= 0 && dwRva < sizeof(dosHeader))
	{
		lstrcpy(szBuffer, TEXT("IMAGE_DOS_HEADER"));
		return ;
	}
	dwLength = dosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS);
	if (dwRva < (DWORD)(dosHeader.e_lfanew))
	{
		lstrcpy(szBuffer, TEXT("Dos Stub"));
		return ;
	}
	if (dwRva < dwLength)
	{
		lstrcpy(szBuffer, TEXT("IMAGE_NT_HEADERS"));
		return ;
	}
	// ��λ���ļ��ڱ�ʼ��
	SetFilePointer(g_hFile, dwLength, 0, FILE_BEGIN);
	// �����ڱ�
	ZeroMemory(&emptyHeader, sizeof(IMAGE_SECTION_HEADER));
	while (1)
	{
		ReadFile(g_hFile, &sectionHeader,
			sizeof(IMAGE_SECTION_HEADER),
			&dwTmp,
			NULL);
		if (!memcmp(&emptyHeader, &sectionHeader, sizeof(IMAGE_SECTION_HEADER)))
		{
			// ���ҽ���
			break;
		}
		/////////////////////////////////////////////////////////////////////////
		// ��ȡ����
		/////////////////////////////////////////////////////////////////////////
		ZeroMemory(szBuffer, sizeof(szBuffer));
		for (i = 0; i < IMAGE_SIZEOF_SHORT_NAME; ++i)
		{
			if (sectionHeader.Name[i] == '\0')
				break;
			szBuffer[i] = sectionHeader.Name[i];
		}
		szBuffer[i] = '\0';
		/////////////////////////////////////////////////////////////////////////
		// �жϵ�ַ��Χ
		/////////////////////////////////////////////////////////////////////////
		if (dwRva >= sectionHeader.VirtualAddress &&
			dwRva < sectionHeader.VirtualAddress + sectionHeader.Misc.VirtualSize)
		{
			return ;
		}
	}
	lstrcpy(szBuffer, TEXT("�޷���λ��ַ"));
	return ;
}

BOOL CALLBACK AddressDlgProc(HWND hwndDlg, 
						   UINT uMsg, 
						   WPARAM wParam, 
						   LPARAM lParam
						   )
{
	DWORD	dwRva;		// ��������ַ
	DWORD	dwVa;		// �����ַ
	DWORD	dwOffset;	// RAWƫ��
	DWORD	dwTmp;		// ��ʱ����
	TCHAR	szBuffer[32];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		// ��ѡ��ť��ѡ��
		CheckRadioButton(hwndDlg,
			IDC_RADIO_VA,
			IDC_RADIO_OFFSET,
			IDC_RADIO_RVA);
		// ����һЩ�ؼ�
		EnableWindow(
			GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_OFFSET),
			FALSE);
		EnableWindow(
			GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_VA),
			FALSE);
		EnableWindow(
			GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_RVA),
			TRUE);
		// ���������ı���
		EnableWindow(
			GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_SECTION),
			FALSE);

		return TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_RVADLG_CLOSE:
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			return TRUE;

		case IDC_RADIO_RVA:
			// ����һЩ�ؼ�
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_OFFSET),
				FALSE);
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_VA),
				FALSE);
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_RVA),
				TRUE);
			return TRUE;

		case IDC_RADIO_VA:
			// ����һЩ�ؼ�
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_OFFSET),
				FALSE);
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_VA),
				TRUE);
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_RVA),
				FALSE);

			return TRUE;

		case IDC_RADIO_OFFSET:
			// ����һЩ�ؼ�
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_OFFSET),// 1035
				TRUE);
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_VA),	// 1033
				FALSE);
			EnableWindow(
				GetDlgItem(hwndDlg, IDC_EDIT_RVADLG_RVA),	// 1034
				FALSE);
			return TRUE;

		case IDC_BTN_RVADLG_TRANS:
			if (g_hFile == NULL)
			{
				return TRUE;
			}
			for (int i = IDC_EDIT_RVADLG_VA; i <= IDC_EDIT_RVADLG_OFFSET; ++i)
			{
				// �ж����ĸ���ѡ��ť��ѡ����
				if (IsWindowEnabled(GetDlgItem(hwndDlg, i)))
				{
					// �������
					ZeroMemory(szBuffer, sizeof(szBuffer));
					dwTmp = 0;
					// ��ȡ����
					GetDlgItemText(hwndDlg, i, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));
					// ת������
					for (int j = 0; j < lstrlen(szBuffer); ++j)
					{
						dwTmp *= 16;
						if (szBuffer[j] >= '0' && szBuffer[j] <= '9')
						{
							dwTmp += szBuffer[j] - '0';
						}
						else if (szBuffer[j] >= 'A' && szBuffer[j] <= 'F')
						{
							dwTmp += szBuffer[j] - 'A' + 10;
						}
					}
					// ��ַת������
					if (i == IDC_EDIT_RVADLG_RVA)
					{
						dwRva = dwTmp;
						RvaToVa(dwRva, dwVa);
						RvaToOffset(dwRva, dwOffset);
					}
					else if (i == IDC_EDIT_RVADLG_VA)
					{
						dwVa = dwTmp;
						VaToRva(dwVa, dwRva);
						VaToOffset(dwVa, dwOffset);
					}
					else if (i == IDC_EDIT_RVADLG_OFFSET)
					{
						dwOffset = dwTmp;
						OffsetToRva(dwOffset, dwRva);
						OffsetToVa(dwOffset, dwVa);
					}
					// ��������
					ZeroMemory(szBuffer, sizeof(szBuffer));
					wsprintf(szBuffer, TEXT("%08X"), dwVa);
					SetDlgItemText(hwndDlg,
						IDC_EDIT_RVADLG_VA,
						szBuffer);
					wsprintf(szBuffer, TEXT("%08X"), dwRva);
					SetDlgItemText(hwndDlg,
						IDC_EDIT_RVADLG_RVA,
						szBuffer);
					wsprintf(szBuffer, TEXT("%08X"), dwOffset);
					SetDlgItemText(hwndDlg,
						IDC_EDIT_RVADLG_OFFSET,
						szBuffer);
					// ����������Ϣ
					GetSectionNameByRva(dwRva, szBuffer);
					SetDlgItemText(hwndDlg, IDC_EDIT_RVADLG_SECTION, szBuffer);
					break;
				}
			}

		default:
			break;

		}

	default:
		break;
	}
	return FALSE;
}

BOOL CALLBACK AboutDlgProc(HWND hwndDlg, 
						 UINT uMsg, 
						 WPARAM wParam, 
						 LPARAM lParam
						 )
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		g_hMailWnd = GetDlgItem(hwndDlg, IDC_STATIC_MAIL);
		g_hBlogWnd = GetDlgItem(hwndDlg, IDC_STATIC_BLOG);

		g_oldBlogProc = (DLGPROC)SetWindowLong(g_hBlogWnd,
							GWL_WNDPROC,
							(LONG)StaticProc);
		g_oldMailProc = (DLGPROC)SetWindowLong(g_hMailWnd,
							GWL_WNDPROC,
							(LONG)StaticProc);
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

BOOL CALLBACK SectionDlgProc(HWND hwndDlg, 
						   UINT uMsg, 
						   WPARAM wParam, 
						   LPARAM lParam
						   )
{
	static HWND	hListWnd;
	LV_COLUMN	lvc;
	LVITEM		lvi;
	DWORD		i;
	TCHAR		szText[256];
	TCHAR		lpColNames[6][32]	= {	TEXT("����"), TEXT("VOffset"), TEXT("VSize"), 
										TEXT("ROffset"), TEXT("RSize"), TEXT("��־")};
	DWORD		dwColWidths[]	= {75, 75, 75, 75, 75, 75};

	// ��ȡ�ڱ���Ϣ��ر���
	IMAGE_SECTION_HEADER	sectionHeader;
	IMAGE_SECTION_HEADER	emptyHeader;
	IMAGE_DOS_HEADER		dosHeader;

	DWORD	dwLength, dwTmp;
	DWORD	dwIndex = 0;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		hListWnd = GetDlgItem(hwndDlg, IDC_LIST_SECTION);
		// �����б���ͼ���		
		// ��ѡ�� + ����
		ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.pszText = szText;
		lvc.iImage = 1;
		lvc.fmt = LVCFMT_LEFT;

		// �����ͷ����
		for (i = 0; i < 6; ++i)
		{
			lvc.pszText		= lpColNames[i];
			lvc.cx			= dwColWidths[i];
			lvc.iSubItem	= i;
			if (ListView_InsertColumn(hListWnd, i, &lvc) == -1)
			{
				return 0;
			}
		}

		if (g_hFile == NULL)
		{
			return TRUE;
		}
		
		// ��ȡ�ڱ���Ϣ
		dwIndex = 0;
		// ��λ���ļ���ʼ��
		SetFilePointer(g_hFile, 0, 0, FILE_BEGIN);
		// ��ȡIMAGE_DOS_HEADER
		ReadFile(g_hFile, &dosHeader, 
			sizeof(IMAGE_DOS_HEADER),
			&dwTmp,
			NULL);
		dwLength = dosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS);
		// ��λ���ļ��ڱ�ʼ��
		SetFilePointer(g_hFile, dwLength, 0, FILE_BEGIN);
		// �����ڱ�
		ZeroMemory(&emptyHeader, sizeof(IMAGE_SECTION_HEADER));
		while (1)
		{
			ReadFile(g_hFile, &sectionHeader,
				sizeof(IMAGE_SECTION_HEADER),
				&dwTmp,
				NULL);
			if (!memcmp(&emptyHeader, &sectionHeader, sizeof(IMAGE_SECTION_HEADER)))
			{
				// ���ҽ���
				break;
			}
			/////////////////////////////////////////////////////////////////////////
			// ��������
			/////////////////////////////////////////////////////////////////////////
			ZeroMemory(&lvi,sizeof(lvi));
			lvi.mask		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
			lvi.state		= 0;
			lvi.stateMask	= 0;
			lvi.iItem		= dwIndex;
			lvi.iImage		= 0;
			lvi.iSubItem	= 0;
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ����
			/////////////////////////////////////////////////////////////////////////
			ZeroMemory(szText, sizeof(szText));
			for (i = 0; i < IMAGE_SIZEOF_SHORT_NAME; ++i)
			{
				if (sectionHeader.Name[i] == '\0')
					break;
				szText[i] = sectionHeader.Name[i];
			}
			// ��������
			lvi.pszText		= szText;
			lvi.cchTextMax	= lstrlen(lvi.pszText) + 1;
			ListView_InsertItem(hListWnd, &lvi);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡRVA
			/////////////////////////////////////////////////////////////////////////
			TCHAR szRva[32], szFormat[32] = TEXT("%08X");
			wsprintf(szRva, szFormat, sectionHeader.VirtualAddress);
			ListView_SetItemText(hListWnd, dwIndex, 1, szRva);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ�����ַ��С
			/////////////////////////////////////////////////////////////////////////
			TCHAR szRvaSize[32];
			wsprintf(szRvaSize, szFormat, sectionHeader.Misc.VirtualSize);
			ListView_SetItemText(hListWnd, dwIndex, 2, szRvaSize);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ����ƫ�Ƶ�ַ
			/////////////////////////////////////////////////////////////////////////
			TCHAR szOffset[32];
			wsprintf(szOffset, szFormat, sectionHeader.PointerToRawData);
			ListView_SetItemText(hListWnd, dwIndex, 3, szOffset);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ�����ַ��С
			/////////////////////////////////////////////////////////////////////////
			TCHAR szRawSize[32];
			wsprintf(szRawSize, szFormat, sectionHeader.SizeOfRawData);
			ListView_SetItemText(hListWnd, dwIndex, 4, szRawSize);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ�����ַ��С
			/////////////////////////////////////////////////////////////////////////
			TCHAR szFlag[32];
			wsprintf(szFlag, szFormat, sectionHeader.Characteristics);
			ListView_SetItemText(hListWnd, dwIndex, 5, szFlag);

			++dwIndex;
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// ����IMAGE_IMPORT_DESCRIPTOR
/////////////////////////////////////////////////////////////////////
void ParseIID(HWND& hListDll)
{
	if (g_hFile == NULL)
	{
		return ;
	}

	LVITEM			lvi;
	DWORD			i;
	TCHAR			szText[256];
	char			szBuffer[4];
	TCHAR			szFormat[] = TEXT("%08X");

	IMAGE_DOS_HEADER		dosHeader;
	IMAGE_NT_HEADERS		ntHeader;
	IMAGE_DATA_DIRECTORY	dataDir;
	IMAGE_DATA_DIRECTORY	emptyDir;
	IMAGE_IMPORT_DESCRIPTOR	iid;
	IMAGE_IMPORT_DESCRIPTOR	emptyIid;

	DWORD			dwTmp;
	DWORD			dwStartRva;
	DWORD			dwOffset;
	DWORD			dwIndex;
	// ��ʱ����
	DWORD			dwTmpRva;
	DWORD			dwTmpOffset;
	
	// �����ļ�ƫ��
	SetFilePointer(g_hFile, 0, 0, FILE_BEGIN);
	// ��ȡIMAGE_DOS_HEADER
	ReadFile(g_hFile, &dosHeader, sizeof(dosHeader),
		&dwTmp, NULL);
	// �����ļ�ƫ��
	SetFilePointer(g_hFile, dosHeader.e_lfanew, 0, FILE_BEGIN);
	// ��ȡIMAGE_NT_HEADERS
	ReadFile(g_hFile, &ntHeader, sizeof(ntHeader), &dwTmp, NULL);
	// ����Ŀ¼��
	dataDir = ntHeader.OptionalHeader.DataDirectory[1];
	ZeroMemory(&emptyDir, sizeof(emptyDir));
	// ��������Ϊ��
	if (!memcmp(&dataDir, &emptyDir, sizeof(dataDir)))
	{
		return ;
	}
	// ��ȡ�����RVA
	dwStartRva = dataDir.VirtualAddress;
	// ��RVAת��Ϊƫ�Ƶ�ַ
	RvaToOffset(dwStartRva, dwOffset);
	// �����ļ�ƫ��
	SetFilePointer(g_hFile, dwOffset, 0, FILE_BEGIN);

	// ��ʼ��ȡIID
	ZeroMemory(&emptyIid, sizeof(emptyIid));
	dwIndex = 0;
	while (1)
	{
		ReadFile(g_hFile,
			&iid,
			sizeof(iid),
			&dwTmp,
			NULL);
		if (!memcmp(&iid, &emptyIid, sizeof(iid)))
		{
			return ;
		}
		/////////////////////////////////////////////////////////////////////////
		// ��������
		/////////////////////////////////////////////////////////////////////////
		// DllName	OriginalFirstThunk	TimeDateStamp	ForwarderChain	Name	FirstThunk
		ZeroMemory(&lvi,sizeof(lvi));
		lvi.mask		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
		lvi.state		= 0;
		lvi.stateMask	= 0;
		lvi.iItem		= dwIndex;
		lvi.iImage		= 0;
		lvi.iSubItem	= 0;
		/////////////////////////////////////////////////////////////////////////
		// ��ȡDLL����
		/////////////////////////////////////////////////////////////////////////
		ZeroMemory(szText, sizeof(szText));
		dwTmpRva = iid.Name;
		RvaToOffset(dwTmpRva, dwTmpOffset);
		SetFilePointer(g_hFile, dwTmpOffset, 0, FILE_BEGIN);
		for (i = 0; ; ++i)
		{
			ReadFile(g_hFile, szBuffer, 1, &dwTmp, NULL);
			if (szBuffer[0] == '\0')
			{
				szText[i] = '\0';
				break;
			}
			szText[i] = szBuffer[0];
		}
		// ��������
		lvi.pszText		= szText;
		lvi.cchTextMax	= lstrlen(lvi.pszText) + 1;
		ListView_InsertItem(hListDll, &lvi);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡOriginalFirstThunk��RVA
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, szFormat, iid.OriginalFirstThunk);
		ListView_SetItemText(hListDll, dwIndex, 1, szText);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡʱ�����ڱ�־
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, szFormat, iid.TimeDateStamp);
		ListView_SetItemText(hListDll, dwIndex, 2, szText);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡForwarderChain
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, szFormat, iid.ForwarderChain);
		ListView_SetItemText(hListDll, dwIndex, 3, szText);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡ����
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, szFormat, iid.Name);
		ListView_SetItemText(hListDll, dwIndex, 4, szText);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡFirstThunk
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, szFormat, iid.FirstThunk);
		ListView_SetItemText(hListDll, dwIndex, 5, szText);

		// �����ļ�ָ��
		++dwIndex;
		SetFilePointer(g_hFile, 
			dwOffset + dwIndex * sizeof(IMAGE_IMPORT_DESCRIPTOR), 
			0, 
			FILE_BEGIN);
	}
}

// ������DLL����ĺ���
void GetFuntionInfo(HWND& hList, DWORD dwIidIndex)
{
	if (g_hFile == NULL)
	{
		return ;
	}

	LVITEM			lvi;
	DWORD			i;
	TCHAR			szText[256];
	char			szBuffer[4];
	TCHAR			szFormat[] = TEXT("%08X");

	IMAGE_DOS_HEADER		dosHeader;
	IMAGE_NT_HEADERS		ntHeader;
	IMAGE_DATA_DIRECTORY	dataDir;
	IMAGE_DATA_DIRECTORY	emptyDir;
	IMAGE_IMPORT_DESCRIPTOR	iid;
	IMAGE_THUNK_DATA		itd;
	IMAGE_THUNK_DATA		emptyItd;
	IMAGE_IMPORT_BY_NAME	iibn;

	DWORD			dwTmp;
	DWORD			dwStartRva;
	DWORD			dwOffset;
	DWORD			dwIndex;
	// ��ʱ����
	DWORD			dwTmpRva;
	DWORD			dwTmpOffset;

	// �����ļ�ƫ��
	SetFilePointer(g_hFile, 0, 0, FILE_BEGIN);
	// ��ȡIMAGE_DOS_HEADER
	ReadFile(g_hFile, &dosHeader, sizeof(dosHeader),
		&dwTmp, NULL);
	// �����ļ�ƫ��
	SetFilePointer(g_hFile, dosHeader.e_lfanew, 0, FILE_BEGIN);
	// ��ȡIMAGE_NT_HEADERS
	ReadFile(g_hFile, &ntHeader, sizeof(ntHeader), &dwTmp, NULL);
	// ����Ŀ¼��
	dataDir = ntHeader.OptionalHeader.DataDirectory[1];

	ZeroMemory(&emptyDir, sizeof(emptyDir));
	// ��������Ϊ��
	if (!memcmp(&dataDir, &emptyDir, sizeof(dataDir)))
	{
		return ;
	}

	// ��ȡ�����RVA
	dwStartRva = dataDir.VirtualAddress;
	// ��RVAת��Ϊƫ�Ƶ�ַ
	RvaToOffset(dwStartRva, dwOffset);
	// �����ļ�ƫ��ֵ
	dwOffset = dwOffset + dwIidIndex * (sizeof(IMAGE_IMPORT_DESCRIPTOR));
	// �����ļ�ƫ��
	SetFilePointer(g_hFile, dwOffset, 0, FILE_BEGIN);

	// ��ȡIID
	ReadFile(g_hFile,
		&iid,
		sizeof(iid),
		&dwTmp,
		NULL);
	// �����ļ�ƫ��ֵ
	dwTmpRva = iid.OriginalFirstThunk;
	RvaToOffset(dwTmpRva, dwTmpOffset);
	dwOffset = dwTmpOffset;
	SetFilePointer(g_hFile, dwOffset, 0, FILE_BEGIN);
	// ��ʼ������
	ZeroMemory(&emptyItd, sizeof(emptyItd));
	dwIndex = 0;
	dwTmpOffset = dwOffset;
	// ����б���ͼ����
	ListView_DeleteAllItems(hList);
	// ��ʼ��ȡ����
	while (1)
	{
		ReadFile(g_hFile,
			&itd,
			sizeof(itd),
			&dwTmp,
			NULL);
		if (!memcmp(&itd, &emptyItd, sizeof(itd)))
		{
			return ;
		}
		/////////////////////////////////////////////////////////////////////////
		// ��������
		/////////////////////////////////////////////////////////////////////////
		// DllName	OriginalFirstThunk	TimeDateStamp	ForwarderChain	Name	FirstThunk
		ZeroMemory(&lvi,sizeof(lvi));
		lvi.mask		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
		lvi.state		= 0;
		lvi.stateMask	= 0;
		lvi.iItem		= dwIndex;
		lvi.iImage		= 0;
		lvi.iSubItem	= 0;
		/////////////////////////////////////////////////////////////////////////
		// ��ȡThunk��RVA
		/////////////////////////////////////////////////////////////////////////
		OffsetToRva(dwTmpOffset, dwTmp);
		wsprintf(szText, szFormat, dwTmp);
		lvi.pszText		= szText;
		lvi.cchTextMax	= lstrlen(lvi.pszText) + 1;
		ListView_InsertItem(hList, &lvi);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡThunk��Offset
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, szFormat, dwTmpOffset);
		ListView_SetItemText(hList, dwIndex, 1, szText);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡThunk��ֵ
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, szFormat, itd.u1.ForwarderString);
		ListView_SetItemText(hList, dwIndex, 2, szText);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡHint
		/////////////////////////////////////////////////////////////////////////
		dwTmpRva = itd.u1.AddressOfData;
		// �ж�����Ż����ַ���
		if ((dwTmpRva & IMAGE_ORDINAL_FLAG32) == 0)
		{
			RvaToOffset(dwTmpRva, dwOffset);
			// �����ļ�ƫ��
			SetFilePointer(g_hFile, 
				dwOffset, 
				0, 
				FILE_BEGIN);
			// ��ȡIMAGE_IMPORT_BY_NAME
			ReadFile(g_hFile,
				&iibn,
				sizeof(iibn),
				&dwTmp,
				NULL);
			wsprintf(szText, TEXT("%04X"), iibn.Hint);
			ListView_SetItemText(hList, dwIndex, 3, szText);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡDLL����
			/////////////////////////////////////////////////////////////////////////
			ZeroMemory(szText, sizeof(szText));
			// �ṹ�����ʹ��iibn��СΪ4
			// ǰ�����ֽ�ΪHINT
			// �������ֽ�Ϊ�������Ƶ�ǰ�����ַ�
			szText[0] = ((BYTE *)&iibn)[2];
			szText[1] = ((BYTE *)&iibn)[3];
			for (i = 2; ; ++i)
			{
				ReadFile(g_hFile, szBuffer, 1, &dwTmp, NULL);
				if (szBuffer[0] == '\0')
				{
					szText[i] = '\0';
					break;
				}
				szText[i] = szBuffer[0];
			}
			// ��������
			lvi.pszText		= szText;
			lvi.cchTextMax	= lstrlen(lvi.pszText) + 1;
			ListView_SetItemText(hList, dwIndex, 4, szText);
		}
		else
		{
			dwTmpRva &= ~IMAGE_ORDINAL_FLAG32;
			ListView_SetItemText(hList, dwIndex, 3, TEXT("���"));
			wsprintf(szText, TEXT("%04X"), dwTmpRva);
			ListView_SetItemText(hList, dwIndex, 4, szText);
		}
		////////////////////////////////////////////////////////////////////
		// �����ļ�ָ��
		++dwIndex;
		dwTmpOffset += sizeof(DWORD);
		SetFilePointer(g_hFile, 
			dwTmpOffset, 
			0, 
			FILE_BEGIN);
	}
}

BOOL CALLBACK IatDlgProc(HWND hwndDlg, 
						   UINT uMsg, 
						   WPARAM wParam, 
						   LPARAM lParam
						   )
{
	static HWND		hListDll;
	static HWND		hListCall;
	LV_COLUMN		lvc;
	DWORD			i;
	TCHAR			lpColNames[6][32]	= {	TEXT("DllName"), TEXT("OriginalFirstThunk"), 
											TEXT("TimeDateStamp"), TEXT("ForwarderChain"), 
											TEXT("Name"), TEXT("FirstThunk")};
	TCHAR			lpColFunNames[5][32]	= {	TEXT("Thunk RVA"), TEXT("Thunk Offset"), 
											TEXT("Thunk Value"), TEXT("Hint"), 
											TEXT("Function")};
	DWORD			dwColWidths[]	= {100, 100, 100, 100, 75, 75};
	DWORD			dwColFunWidths[]	= {90, 90, 90, 90, 190};
	switch (uMsg)
	{
		//lResult = SendMessage(
		//	// returns LRESULT in lResult
		//	(HWND) hWndControl,      
		//	// handle to destination control
		//	(UINT) WM_NOTIFY,      // message ID
		//	(WPARAM) wParam,      // = (WPARAM) (int) idCtrl;
		//	(LPARAM) lParam      // = (LPARAM) (LPNMHDR) pnmh; );  
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_CLICK:
			if (((LPNMHDR)lParam)->idFrom == IDC_LIST_IAT_DLL)
			{
				// ��Ϣ��Ӧ
				NM_LISTVIEW*	pNMListView = (NM_LISTVIEW*)lParam;
				/////////////////////////////////////////////////////
				// ��ȡ������ϸ����
				/////////////////////////////////////////////////////
				GetFuntionInfo(hListCall, 
					pNMListView->iItem);
				// ����
				return TRUE;
			}
		default:
			break;
		}
		return FALSE;

	case WM_INITDIALOG:
		/////////////////////////////////////////////////////////////////////
		// ��ʼ���ؼ�
		/////////////////////////////////////////////////////////////////////
		hListDll = GetDlgItem(hwndDlg, IDC_LIST_IAT_DLL);
		// �����б���ͼ���		
		// ��ѡ�� + ����
		ListView_SetExtendedListViewStyle(hListDll, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.iImage = 1;
		lvc.fmt = LVCFMT_LEFT;

		// �����ͷ����
		for (i = 0; i < 6; ++i)
		{
			lvc.pszText		= lpColNames[i];
			lvc.cx			= dwColWidths[i];
			lvc.iSubItem	= i;
			if (ListView_InsertColumn(hListDll, i, &lvc) == -1)
			{
				return 0;
			}
		}
		/////////////////////////////////////////////////////////////////////
		// ����IMAGE_IMPORT_DESCRIPTOR
		/////////////////////////////////////////////////////////////////////
		ParseIID(hListDll);
		/////////////////////////////////////////////////////////////////////
		// ��ʼ���ؼ�
		/////////////////////////////////////////////////////////////////////
		hListCall = GetDlgItem(hwndDlg, IDC_LIST_IAT_CALL);
		// �����б���ͼ���		
		// ��ѡ�� + ����
		ListView_SetExtendedListViewStyle(hListCall, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.iImage = 1;
		lvc.fmt = LVCFMT_LEFT;

		// �����ͷ����
		for (i = 0; i < 5; ++i)
		{
			lvc.pszText		= lpColFunNames[i];
			lvc.cx			= dwColFunWidths[i];
			lvc.iSubItem	= i;
			if (ListView_InsertColumn(hListCall, i, &lvc) == -1)
			{
				return 0;
			}
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

void GetProcessList(HWND& hListProcess)
{
	HANDLE				hSnapshot;
	PROCESSENTRY32		pe32  = {0};
	BOOL				bHasNext = FALSE;
	LVITEM				lvi;
	DWORD				i;
	TCHAR				szText[256];

	hSnapshot = CreateToolhelp32Snapshot(
					TH32CS_SNAPPROCESS,	// �������̿���
					0);					// ����
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		MessageBox(hListProcess, TEXT("�������̿���ʧ�ܣ�"),
			TEXT("Tip"), MB_ICONERROR);
		return ;
	}

	// ����б���ͼ����
	ListView_DeleteAllItems(hListProcess);
	
	ZeroMemory(&pe32, sizeof(PROCESSENTRY32));
	pe32.dwSize = sizeof(PROCESSENTRY32);
	bHasNext = Process32First(hSnapshot, &pe32);
	i = 0;
	while (bHasNext)
	{
		/////////////////////////////////////////////////////////////////////////
		// ��������
		/////////////////////////////////////////////////////////////////////////
		ZeroMemory(&lvi,sizeof(lvi));
		lvi.mask		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
		lvi.state		= 0;
		lvi.stateMask	= 0;
		lvi.iItem		= i;
		lvi.iImage		= 0;
		lvi.iSubItem	= 0;

		HANDLE hModuleSnapshot = CreateToolhelp32Snapshot(
									TH32CS_SNAPMODULE,
									pe32.th32ProcessID);
		
		// ��PIDΪ0ʱ�����ɹ�������0��ζ�ŵ�ǰ����
		// ����Ҫ��һ���ر��ж�
		if (hModuleSnapshot != INVALID_HANDLE_VALUE 
			&& pe32.th32ProcessID != 0)
		{
			MODULEENTRY32 me32 = {0};
			me32.dwSize = sizeof(MODULEENTRY32);
			Module32First(hModuleSnapshot, &me32);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ����·��
			/////////////////////////////////////////////////////////////////////////
			lvi.pszText		= me32.szExePath;
			lvi.cchTextMax	= lstrlen(lvi.pszText) + 1;
			ListView_InsertItem(hListProcess, &lvi);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ����PID
			/////////////////////////////////////////////////////////////////////////
			wsprintf(szText, TEXT("%d"), pe32.th32ProcessID);
			ListView_SetItemText(hListProcess, i, 1, szText);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ���̾����ַ
			/////////////////////////////////////////////////////////////////////////
			wsprintf(szText, TEXT("%08X"), me32.modBaseAddr);
			ListView_SetItemText(hListProcess, i, 2, szText);
			/////////////////////////////////////////////////////////////////////////
			// ��ȡ���̾����С
			/////////////////////////////////////////////////////////////////////////
			wsprintf(szText, TEXT("%08X"), me32.modBaseSize);
			ListView_SetItemText(hListProcess, i, 3, szText);

			CloseHandle(hModuleSnapshot);
		}
		else
		{
			//TCHAR szTmp[256];
			//wsprintf(szTmp, TEXT("%d"), GetLastError());
			//MessageBox(NULL, szTmp, szTmp, MB_OK);
			////////////////////////////////////////////////////////////////////////

			lvi.pszText		= pe32.szExeFile;
			lvi.cchTextMax	= lstrlen(lvi.pszText) + 1;
			ListView_InsertItem(hListProcess, &lvi);

			wsprintf(szText, TEXT("%d"), pe32.th32ProcessID);
			ListView_SetItemText(hListProcess, i, 1, szText);

			lstrcpy(szText, TEXT("Error"));
			ListView_SetItemText(hListProcess, i, 2, szText);

			lstrcpy(szText, TEXT("Error"));
			ListView_SetItemText(hListProcess, i, 3, szText);

		}
		/////////////////////////////////////////////////////////////////////////
		pe32.dwSize = sizeof(PROCESSENTRY32);
		bHasNext = Process32Next(hSnapshot, &pe32);
		++i;
	}
	CloseHandle(hSnapshot);
}

void GetModuleList(HWND& hListModule, 
				   DWORD pid)
{
	HANDLE				hSnapshot;
	BOOL				bHasNext = FALSE;
	LVITEM				lvi;
	DWORD				i;
	TCHAR				szText[256];

	// ����б���ͼ����
	ListView_DeleteAllItems(hListModule);

	// ��PIDΪ0ʱ�����ɹ�������0��ζ�ŵ�ǰ����
	// ����Ҫ��һ���ر��ж�
	if (pid == 0)
	{
		return ;
	}

	hSnapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPMODULE,	// ����Module����
		pid);				// PID

	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		wsprintf(szText, TEXT("����ģ�����ʧ�ܣ�(�������: %d)"), GetLastError());
		MessageBox(hListModule, szText,
			TEXT("Tip"), MB_ICONERROR);
		return ;
	}

	MODULEENTRY32 me32 = {0};
	me32.dwSize = sizeof(MODULEENTRY32);
	bHasNext = Module32First(hSnapshot, &me32);
	i = 0;
	while (bHasNext)
	{
		/////////////////////////////////////////////////////////////////////////
		// ��������
		/////////////////////////////////////////////////////////////////////////
		ZeroMemory(&lvi,sizeof(lvi));
		lvi.mask		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
		lvi.state		= 0;
		lvi.stateMask	= 0;
		lvi.iItem		= i;
		lvi.iImage		= 0;
		lvi.iSubItem	= 0;

			
		/////////////////////////////////////////////////////////////////////////
		// ��ȡģ��·��
		/////////////////////////////////////////////////////////////////////////
		lvi.pszText		= me32.szExePath;
		lvi.cchTextMax	= lstrlen(lvi.pszText) + 1;
		ListView_InsertItem(hListModule, &lvi);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡ���̾����ַ
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, TEXT("%08X"), me32.modBaseAddr);
		ListView_SetItemText(hListModule, i, 1, szText);
		/////////////////////////////////////////////////////////////////////////
		// ��ȡ���̾����С
		/////////////////////////////////////////////////////////////////////////
		wsprintf(szText, TEXT("%08X"), me32.modBaseSize);
		ListView_SetItemText(hListModule, i, 2, szText);
		/////////////////////////////////////////////////////////////////////////
		me32.dwSize = sizeof(MODULEENTRY32);
		bHasNext = Module32Next(hSnapshot, &me32);
		++i;
	}
	CloseHandle(hSnapshot);
}

// ��������Ȩ�ޣ��ܹ���һЩϵͳ����
// ���Ƕ��ڰ�ȫ������Ľ�����Ȼ�򲻿�
BOOL EnableDebugPrivilege()
{
	HANDLE hToken = NULL;
	BOOL bRst = OpenProcessToken(
		GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	if (!bRst)
		return FALSE;

	TOKEN_PRIVILEGES tknPri = {0};
	tknPri.PrivilegeCount = 1;
	bRst = LookupPrivilegeValue(
		NULL, SE_DEBUG_NAME, &tknPri.Privileges[0].Luid);
	if (!bRst)
	{
		CloseHandle(hToken);
		return FALSE;
	}

	tknPri.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	bRst = AdjustTokenPrivileges(
		hToken, FALSE, &tknPri, 
		sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (!bRst)
	{
		CloseHandle(hToken);
		return FALSE;
	}

	DWORD dwRst = GetLastError();
	if (ERROR_SUCCESS != dwRst)
	{
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);
	return TRUE;
}


BOOL CALLBACK TaskMgrDlgProc(HWND hwndDlg, 
						   UINT uMsg, 
						   WPARAM wParam, 
						   LPARAM lParam
						   )
{
	static HWND		hListProcess;
	static HWND		hListModule;
	LV_COLUMN		lvc;
	DWORD			i;
	TCHAR			lpColNames[4][32]	= {	TEXT("����·��"), TEXT("PID"), 
											TEXT("�����ַ"), TEXT("�����С")};
	TCHAR			lpColMNames[3][32]	= {	TEXT("ģ��·��"), 
											TEXT("�����ַ"), TEXT("�����С")};
	DWORD			dwColWidths[]	= {300, 80, 80, 80};
	DWORD			dwColMWidths[]	= {380, 80, 80};
	DWORD			dwPid;
	TCHAR			szText[32];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		if(!EnableDebugPrivilege())
			MessageBox(hwndDlg, TEXT("��������Ȩ��ʧ�ܣ�"),
				TEXT("ע��"), MB_ICONWARNING);
		/////////////////////////////////////////////////////////////////////
		// ��ʼ���ؼ�
		/////////////////////////////////////////////////////////////////////
		hListProcess = GetDlgItem(hwndDlg, IDC_LIST_TASKMGR_PROCESS);
		// �����б���ͼ���		
		// ��ѡ�� + ����
		ListView_SetExtendedListViewStyle(hListProcess, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.iImage = 1;
		lvc.fmt = LVCFMT_LEFT;

		// �����ͷ����
		for (i = 0; i < 4; ++i)
		{
			lvc.pszText		= lpColNames[i];
			lvc.cx			= dwColWidths[i];
			lvc.iSubItem	= i;
			if (ListView_InsertColumn(hListProcess, i, &lvc) == -1)
			{
				return 0;
			}
		}
		/////////////////////////////////////////////////////////////////////
		// ��ʼ���ؼ�
		/////////////////////////////////////////////////////////////////////
		hListModule = GetDlgItem(hwndDlg, IDC_LIST_TASKMGR_MODULE);
		// �����б���ͼ���		
		// ��ѡ�� + ����
		ListView_SetExtendedListViewStyle(hListModule, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.iImage = 1;
		lvc.fmt = LVCFMT_LEFT;

		// �����ͷ����
		for (i = 0; i < 3; ++i)
		{
			lvc.pszText		= lpColMNames[i];
			lvc.cx			= dwColMWidths[i];
			lvc.iSubItem	= i;
			if (ListView_InsertColumn(hListModule, i, &lvc) == -1)
			{
				return 0;
			}
		}
		/////////////////////////////////////////////////////////////////////
		// ��ȡ�����б�
		/////////////////////////////////////////////////////////////////////
		GetProcessList(hListProcess);

		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_CLICK:
			if (((LPNMHDR)lParam)->idFrom == IDC_LIST_TASKMGR_PROCESS)
			{
				// ��Ϣ��Ӧ
				NM_LISTVIEW*	pNMListView = (NM_LISTVIEW*)lParam;
				/////////////////////////////////////////////////////
				// ��ȡ������ϸ����
				/////////////////////////////////////////////////////
				ListView_GetItemText(hListProcess,
					pNMListView->iItem,
					1,
					szText,
					sizeof(szText));
				//MessageBox(NULL, szText, szText, MB_OK);
				dwPid = 0;
				for (i = 0; i < lstrlen(szText); ++i)
				{
					dwPid *= 10;
					dwPid += szText[i] - '0';
				}
				GetModuleList(hListModule, 
					dwPid);
				// ����
				return TRUE;
			}
		default:
			break;
		}
		return FALSE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

// ���������
void ParseEat(HWND& hWnd, HWND& hList)
{
	// �ж��ļ�����Ƿ���Ч
	if (g_hFile == NULL || g_hFile == INVALID_HANDLE_VALUE)
	{
		return ;
	}

	LVITEM			lvi;
	DWORD			i;
	TCHAR			szText[256];
	char			szBuffer[4];
	TCHAR			szFormat[] = TEXT("%08X");

	IMAGE_DOS_HEADER		dosHeader;
	IMAGE_NT_HEADERS		ntHeader;
	IMAGE_DATA_DIRECTORY	dataDir;
	IMAGE_DATA_DIRECTORY	emptyDir;
	IMAGE_EXPORT_DIRECTORY	ied;

	DWORD			dwTmp;
	DWORD			dwStartRva;
	DWORD			dwOffset;
	DWORD			dwIndex;
	// ��ʱ����
	DWORD			dwTmpRva;
	DWORD			dwTmpOffset;

	// �����ļ�ƫ��
	SetFilePointer(g_hFile, 0, 0, FILE_BEGIN);
	// ��ȡIMAGE_DOS_HEADER
	ReadFile(g_hFile, &dosHeader, sizeof(dosHeader),
		&dwTmp, NULL);
	// �����ļ�ƫ��
	SetFilePointer(g_hFile, dosHeader.e_lfanew, 0, FILE_BEGIN);
	// ��ȡIMAGE_NT_HEADERS
	ReadFile(g_hFile, &ntHeader, sizeof(ntHeader), &dwTmp, NULL);
	// ����Ŀ¼��
	dataDir = ntHeader.OptionalHeader.DataDirectory[0];
	ZeroMemory(&emptyDir, sizeof(emptyDir));
	// ��������Ϊ��
	if (!memcmp(&dataDir, &emptyDir, sizeof(dataDir)))
	{
		return ;
	}
	// ��ȡ�����RVA
	dwStartRva = dataDir.VirtualAddress;
	// ��RVAת��Ϊƫ�Ƶ�ַ
	RvaToOffset(dwStartRva, dwOffset);
	// �����ļ�ƫ��
	SetFilePointer(g_hFile, dwOffset, 0, FILE_BEGIN);

	// ��ȡIMAGE_EXPORT_DIRECTORY
	ReadFile(g_hFile,
		&ied,
		sizeof(ied),
		&dwTmp,
		0);
	//////////////////////////////////////////////////////////////////////////////
	// ����ı�������
	//////////////////////////////////////////////////////////////////////////////
	// �����ƫ��
	wsprintf(szText, szFormat, dwOffset);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_EATOFFSET, szText);
	// ����ֵ
	wsprintf(szText, szFormat, ied.Characteristics);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_CHARACTER, szText);
	// ��ַ
	wsprintf(szText, szFormat, ied.Base);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_BASE, szText);
	// ����RVA
	wsprintf(szText, szFormat, ied.Name);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_NAME, szText);
	// �����ַ���
	RvaToOffset(ied.Name, dwTmpOffset);
	SetFilePointer(g_hFile, dwTmpOffset, 0, FILE_BEGIN);
	for (i = 0; ; ++i)
	{
		ReadFile(g_hFile, szBuffer, 1, &dwTmp, NULL);
		if (szBuffer[0] == '\0')
		{
			szText[i] = '\0';
			break;
		}
		szText[i] = szBuffer[0];
	}
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_DLLNAME, szText);
	// ��������
	wsprintf(szText, szFormat, ied.NumberOfFunctions);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_FUNNUM, szText);
	// ����������
	wsprintf(szText, szFormat, ied.NumberOfNames);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_FUNNAMENUM, szText);
	// ������ַ
	wsprintf(szText, szFormat, ied.AddressOfFunctions);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_FUNADDR, szText);
	// ��������ַ
	wsprintf(szText, szFormat, ied.AddressOfNames);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_FUNNAMEADDR, szText);
	// ������ŵ�ַ
	wsprintf(szText, szFormat, ied.AddressOfNameOrdinals);
	SetDlgItemText(hWnd, IDC_EDIT_EATDLG_ORDERADDR, szText);

	//////////////////////////////////////////////////////////////////////////////
	// ��������������
	//////////////////////////////////////////////////////////////////////////////
	dwIndex = 0;
	DWORD dwAddressOfFunctions;
	DWORD dwFunctionsRva;
	DWORD dwFunctionsOffset;
	DWORD dwFunctionsIndex;
	DWORD dwFunNameRva;
	DWORD dwFunNameOffset;
	DWORD j;
	DWORD dwAddressOfNameOrdinalsOffset;
	WORD  wIndex;
	RvaToOffset(ied.AddressOfFunctions, dwAddressOfFunctions);
	for (dwIndex = 0; dwIndex < ied.NumberOfFunctions; ++dwIndex)
	{
		// �������
		dwFunctionsIndex = ied.Base + dwIndex;
		wsprintf(szText, TEXT("%04X"), dwFunctionsIndex);
		ZeroMemory(&lvi,sizeof(lvi));
		lvi.mask		= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
		lvi.state		= 0;
		lvi.stateMask	= 0;
		lvi.iItem		= dwIndex;
		lvi.iImage		= 0;
		lvi.iSubItem	= 0;
		lvi.pszText		= szText;
		lvi.cchTextMax	= lstrlen(lvi.pszText) + 1;
		ListView_InsertItem(hList, &lvi);
		// ����RVA
		SetFilePointer(g_hFile, dwAddressOfFunctions + dwIndex*4, 0, FILE_BEGIN);
		ReadFile(g_hFile, &dwFunctionsRva, sizeof(DWORD), &dwTmp, NULL);
		wsprintf(szText, szFormat, dwFunctionsRva);
		ListView_SetItemText(hList, dwIndex, 1, szText);
		// ����Offset
		RvaToOffset(dwFunctionsRva, dwFunctionsOffset);
		wsprintf(szText, szFormat, dwFunctionsOffset);
		ListView_SetItemText(hList, dwIndex, 2, szText);
		// �ж��Ƿ����ַ������ֵ���
		RvaToOffset(ied.AddressOfNameOrdinals, dwAddressOfNameOrdinalsOffset);
		SetFilePointer(g_hFile, dwAddressOfNameOrdinalsOffset, 0, FILE_BEGIN);
		for (j = 0; j < ied.NumberOfNames; ++j)
		{
			ReadFile(g_hFile, &wIndex, sizeof(WORD), &dwTmp, NULL);
			if (dwFunctionsIndex == wIndex + ied.Base)
			{
				break;
			}
		}
		if (j != ied.NumberOfNames)
		{
			RvaToOffset(ied.AddressOfNames, dwFunNameOffset);
			SetFilePointer(g_hFile, dwFunNameOffset + j*4, 0, FILE_BEGIN);
			ReadFile(g_hFile, &dwFunNameRva, sizeof(DWORD), &dwTmp, NULL);
			RvaToOffset(dwFunNameRva, dwFunNameOffset);
			SetFilePointer(g_hFile, dwFunNameOffset, 0, FILE_BEGIN);

			for (i = 0; ; ++i)
			{
				ReadFile(g_hFile, szBuffer, 1, &dwTmp, NULL);
				if (szBuffer[0] == '\0')
				{
					szText[i] = '\0';
					break;
				}
				szText[i] = szBuffer[0];
			}
			ListView_SetItemText(hList, dwIndex, 3, szText);
		}
		else
		{
			ListView_SetItemText(hList, dwIndex, 3, TEXT("��ŵ���"));
		}
	}
}

BOOL CALLBACK EatDlgProc(HWND hwndDlg, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam
							 )
{
	static HWND		hListCall;
	LV_COLUMN		lvc;
	DWORD			i;
	TCHAR			lpColNames[4][32]	= {	TEXT("���"), TEXT("RVA"), 
											TEXT("ƫ��"), TEXT("������")};
	DWORD			dwColWidths[]	= {100, 100, 100, 275};
	DWORD			dwEdit;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		/////////////////////////////////////////////////////////////////////
		// ��ʼ���ؼ�
		/////////////////////////////////////////////////////////////////////
		hListCall = GetDlgItem(hwndDlg, IDC_LIST_EAT);
		// �����б���ͼ���		
		// ��ѡ�� + ����
		ListView_SetExtendedListViewStyle(hListCall, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.iImage = 1;
		lvc.fmt = LVCFMT_LEFT;

		// �����ͷ����
		for (i = 0; i < 4; ++i)
		{
			lvc.pszText		= lpColNames[i];
			lvc.cx			= dwColWidths[i];
			lvc.iSubItem	= i;
			if (ListView_InsertColumn(hListCall, i, &lvc) == -1)
			{
				return 0;
			}
		}
		/////////////////////////////////////////////////////////////////////
		// ���༭������Ϊֻ������
		/////////////////////////////////////////////////////////////////////
		for (dwEdit = IDC_EDIT_EATDLG_EATOFFSET;
			dwEdit <= IDC_EDIT_EATDLG_FUNNAMEADDR;
			++dwEdit)
		{
			SendMessage(GetDlgItem(hwndDlg, dwEdit), EM_SETREADONLY, TRUE, 0);
		}
		/////////////////////////////////////////////////////////////////////
		// ���������
		/////////////////////////////////////////////////////////////////////
		ParseEat(hwndDlg, hListCall);

		return TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}


BOOL CALLBACK DialogProc(HWND hwndDlg, 
						 UINT uMsg, 
						 WPARAM wParam, 
						 LPARAM lParam
						 )
{
	static HANDLE	hFile;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		g_hWnd = hwndDlg;
		SetCtrlStyles();
		return TRUE;

	case WM_CLOSE:
		if (hFile != NULL)
		{
			CloseHandle(hFile);
		}
		DestroyWindow(hwndDlg);
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_OPENFILE:
			if(!GetPeFilePath(szFileName))
				return TRUE;
			// �õ�һ�����ļ�
			if (hFile != NULL)
			{
				CloseHandle(hFile);
			}
			// ���ļ�
			hFile = CreateFile(szFileName,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_ARCHIVE,
						NULL);
			// �ж��ļ��Ƿ�ɹ���
			if (hFile == INVALID_HANDLE_VALUE)
			{
				MessageBox(hwndDlg, TEXT("�޷���ѡ�����ļ���"),
					TEXT("��ʾ��Ϣ"), MB_ICONERROR);
				EmptyCtrlValues();
				return FALSE;
			}
			// ����ȫ�־��
			g_hFile = hFile;
			// ��ȡ������Ϣ
			SetCtrlValues(hFile);
			return TRUE;

		case IDC_BTN_OK:
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			return TRUE;

		case IDC_BTN_ABOUT:
			DialogBoxParam(g_hInstance,
				MAKEINTRESOURCE(IDD_DLG_ABOUT),
				hwndDlg,
				AboutDlgProc,
				0);
			return TRUE;

		case IDC_BTN_DATETIME:
			DialogBoxParam(g_hInstance,
				MAKEINTRESOURCE(IDD_DLG_TIME),
				hwndDlg,
				TimeDlgProc,
				0);
			return TRUE;

		case IDC_BTN_ADDRESS:
			DialogBoxParam(g_hInstance,
				MAKEINTRESOURCE(IDD_DLG_ADDRESS),
				hwndDlg,
				AddressDlgProc,
				0);
			return TRUE;

		case IDC_BTN_SECTION:
			DialogBoxParam(g_hInstance,
				MAKEINTRESOURCE(IDD_DLG_SECTION),
				hwndDlg,
				SectionDlgProc,
				0);
			return TRUE;

		case IDC_BTN_IAT:
			DialogBoxParam(g_hInstance,
				MAKEINTRESOURCE(IDD_DLG_IAT),
				hwndDlg,
				IatDlgProc,
				0);
			return TRUE;

		case IDC_BTN_TASKMGR:
			DialogBoxParam(g_hInstance,
				MAKEINTRESOURCE(IDD_DLG_TASKMGR),
				hwndDlg,
				TaskMgrDlgProc,
				0);
			return TRUE;

		// �⼸����������ȥʵ����
		case IDC_BTN_SUBSYSTEM:
		case IDC_BTN_CHARACTER:
			UnImplementationTips();
			return TRUE;
		
		// �����Ľ���
		case IDC_BTN_EXPORT:
			DialogBoxParam(g_hInstance,
				MAKEINTRESOURCE(IDD_DLG_EXPORT),
				hwndDlg,
				EatDlgProc,
				0);
			return TRUE;
		
		default:
			break;
		}

	default:
		break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// ������
//////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nShowCmd)
{
	g_hInstance = hInstance;

	DialogBox(hInstance,
		MAKEINTRESOURCE(IDD_DLG_MAIN),
		NULL,
		DialogProc);

	return 0;
}
