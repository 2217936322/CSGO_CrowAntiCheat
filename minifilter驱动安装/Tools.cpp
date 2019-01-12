#include "stdafx.h"
#include "Tools.h"
#include "DriverConnecter.h"
#include "PEreverse.h"
PEreverse PEreverser;
void SendMd52Server(LPVOID p)
{
	ThreadParms Parms = *(ThreadParms*)p;
	CString FileDirectory = Parms.FileDirectory;
	int ScanType = Parms.ScanType;
	bool ReportToServer = false;
	CString md5;
	MyTools::GetFileMd5(FileDirectory, md5);
	if(ScanType == SCANTYPE_HIIGHT)
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, Parms.PID);
		MEMORY_BASIC_INFORMATION mbi_thunk;
		PVOID AllocationBase = NULL;
		TCHAR FilePath[MAX_PATH];
		for (LPSTR Addr = (LPSTR)0x00000000; ::VirtualQueryEx(hProcess, Addr, &mbi_thunk, sizeof(mbi_thunk)); Addr = LPSTR(mbi_thunk.BaseAddress) + mbi_thunk.RegionSize)
		{
			if ((mbi_thunk.AllocationBase > AllocationBase) && (GetMappedFileName(hProcess, mbi_thunk.BaseAddress, FilePath, _countof(FilePath)) > 0))
			{
				printf("MODULE:%x, %s \n", AllocationBase, FilePath);

				AllocationBase = mbi_thunk.AllocationBase;

			}
		}
		//���md5 + ɨ��ģ�� + �ֹ�����ģ�� + ����ʽɨ��(�ϴ������)
		// ...
		ReportToServer = PEreverser.GetFileLevel(FileDirectory, true) == FILELEVEL_HIGHT ? true : false;
	}
	//... send md5 to server
}
void MyTools::CheckFileIsCheat(CString FileDirectory,int ScanType,DWORD PID)
{
	ThreadParms Parms;
	Parms.FileDirectory = FileDirectory;
	Parms.ScanType = ScanType;
	Parms.PID = PID;
	//���߳̿���
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendMd52Server, &Parms, NULL, NULL);
	return;
}
BOOL MyTools::GetFileMd5(CString FileDirectory, CString &strFileMd5)
{
	HANDLE hFile = CreateFile(FileDirectory, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)                                        //���CreateFile����ʧ��  
	{
		//��ʾCreateFile����ʧ�ܣ����������š�visual studio�п��ڡ����ߡ�>��������ҡ������ô���ŵõ�������Ϣ��  
		CloseHandle(hFile);
		return FALSE;
	}
	HCRYPTPROV hProv = NULL;
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)       //���CSP��һ����Կ�����ľ��
	{
		return FALSE;
	}
	HCRYPTPROV hHash = NULL;
	//��ʼ������������hash������������һ����CSP��hash������صľ��������������������    CryptHashData���á�
	if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)
	{
		return FALSE;
	}
	DWORD dwFileSize = GetFileSize(hFile, 0);    //��ȡ�ļ��Ĵ�С
	if (dwFileSize == 0xFFFFFFFF)               //�����ȡ�ļ���Сʧ��  
	{
		return FALSE;
	}
	byte* lpReadFileBuffer = new byte[dwFileSize];
	DWORD lpReadNumberOfBytes;
	if (ReadFile(hFile, lpReadFileBuffer, dwFileSize, &lpReadNumberOfBytes, NULL) == 0)        //��ȡ�ļ�  
	{
		return FALSE;
	}
	if (CryptHashData(hHash, lpReadFileBuffer, lpReadNumberOfBytes, 0) == FALSE)      //hash�ļ�  
	{
		return FALSE;
	}
	delete[] lpReadFileBuffer;
	CloseHandle(hFile);          //�ر��ļ����
	BYTE *pbHash;
	DWORD dwHashLen = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0))
	{
		return FALSE;
	}
	pbHash = (byte*)malloc(dwHashLen);
	if (CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0))//���md5ֵ 
	{
		for (DWORD i = 0; i<dwHashLen; i++)         //���md5ֵ 
		{
			TCHAR str[2] = { 0 };
			CString strFilePartM = _T("");
			_stprintf(str, _T("%02x"), pbHash[i]);
			strFileMd5 += str;
		}
	}

	//�ƺ���
	if (CryptDestroyHash(hHash) == FALSE)          //����hash����  
	{
		return FALSE;
	}
	if (CryptReleaseContext(hProv, 0) == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

//�ر��ļ��ض���ϵͳ
BOOL MyTools::DisableWow64FsRedirection(void)
{
	PVOID   pOldValue = NULL;
	typedef BOOL(WINAPI *pfnWow64DisableWow64FsRedirection)(PVOID *OldValue);
	static pfnWow64DisableWow64FsRedirection pWow64DisableWow64 = (pfnWow64DisableWow64FsRedirection)GetProcAddress(GetModuleHandle(TEXT("Kernel32.dll")), "Wow64DisableWow64FsRedirection");

	if (pWow64DisableWow64)
	{
		return pWow64DisableWow64(&pOldValue);
	}

	return FALSE;
}

//�����ļ��ض���ϵͳ
BOOL MyTools::RevertWow64FsRedirection(void)
{

	PVOID   pOldValue = NULL;
	typedef BOOL(WINAPI *pfnWow64RevertWow64FsRedirection)(PVOID OldValue);
	static pfnWow64RevertWow64FsRedirection pWow64RevertWow64 = (pfnWow64RevertWow64FsRedirection)GetProcAddress(GetModuleHandle(TEXT("Kernel32.dll")), "Wow64RevertWow64FsRedirection");

	//if (IsWowo64System())
	{
		if (pWow64RevertWow64)
		{
			return pWow64RevertWow64(&pOldValue);
		}
	}

	return FALSE;
}

//���ض�����ļ�
BOOL MyTools::RedirectionCreateFile(const wchar_t* pFilePath, HANDLE& hFile)
{
	BOOL bRet = FALSE;
	assert(NULL != pFilePath);

	//�ر��ļ��ض���ϵͳ
	BOOL bDisableWow64FsRedirection = DisableWow64FsRedirection();
	hFile = CreateFileW(pFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		bRet = TRUE;
	}
	//�����ļ��ض���ϵͳ
	if (bDisableWow64FsRedirection)
	{
		RevertWow64FsRedirection();
	}
	return bRet;
}

//��ȡ�ļ�����ǩ��
wchar_t* MyTools::GetCertName(wchar_t* pFilePath)
{
	HCERTSTORE hStore = NULL;
	HCRYPTMSG hMsg = NULL;
	PCCERT_CONTEXT pCertContext = NULL;
	BOOL bResult = FALSE;
	DWORD dwEncoding, dwContentType, dwFormatType;
	PCMSG_SIGNER_INFO pSignerInfo = NULL;
	DWORD dwSignerInfo = 0;
	CERT_INFO CertInfo;
	wchar_t* pCertName = NULL;
	DWORD dwData = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD NumberOfBytesRead = 0;
	DWORD dwFilesize = 0;
	BYTE* pBuff = NULL;
	BOOL bDisableWow64FsRedirection = FALSE;
	memset(&CertInfo, 0, sizeof(CertInfo));
	if (IsBadReadPtr(pFilePath, sizeof(DWORD)) != 0)
	{
		return NULL;
	}


	do
	{
		if (!RedirectionCreateFile(pFilePath, hFile))
			break;

		dwFilesize = GetFileSize(hFile, NULL);
		pBuff = new BYTE[dwFilesize + 1];
		assert(NULL != pBuff);
		RtlZeroMemory(pBuff, dwFilesize + 1);
		if (ReadFile(hFile, pBuff, dwFilesize, &NumberOfBytesRead, NULL) == FALSE)
		{
			CloseHandle(hFile);
			break;
		}
		CloseHandle(hFile);


		CERT_BLOB Object = { 0 };
		Object.cbData = dwFilesize;
		Object.pbData = pBuff;
		bResult = CryptQueryObject(CERT_QUERY_OBJECT_BLOB, &Object
			, CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, CERT_QUERY_FORMAT_FLAG_BINARY
			, 0, &dwEncoding, &dwContentType, &dwFormatType, &hStore, &hMsg, NULL);
		if (!bResult)
		{
			// ���ʧ�ܣ�����ԭ�е��жϷ�ʽ��ִ��һ�飬ȷ���˴α�����������ǰ�Ĵ��봦��Ч��
			//�ر��ļ��ض���ϵͳ
			bDisableWow64FsRedirection = DisableWow64FsRedirection();
			bResult = CryptQueryObject(CERT_QUERY_OBJECT_FILE, pFilePath
				, CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, CERT_QUERY_FORMAT_FLAG_BINARY
				, 0, &dwEncoding, &dwContentType, &dwFormatType, &hStore, &hMsg, NULL);
			if (bDisableWow64FsRedirection)
			{
				RevertWow64FsRedirection();
			}
			if (!bResult)break;
		}

		bResult = CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &dwSignerInfo);
		if (!bResult)break;

		pSignerInfo = (PCMSG_SIGNER_INFO) new char[dwSignerInfo];
		if (NULL == pSignerInfo)break;
		ZeroMemory(pSignerInfo, dwSignerInfo);

		bResult = CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, (PVOID)pSignerInfo, &dwSignerInfo);
		if (!bResult)break;

		CertInfo.Issuer = pSignerInfo->Issuer;
		CertInfo.SerialNumber = pSignerInfo->SerialNumber;
		pCertContext = CertFindCertificateInStore(hStore, ENCODING, 0, CERT_FIND_SUBJECT_CERT, (PVOID)&CertInfo, NULL);
		if (NULL == pCertContext)break;

		dwData = CertGetNameString(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, NULL, 0);
		if (1 >= dwData)
			break;

		pCertName = new wchar_t[dwData + 1];
		if (NULL == pCertName)break;
		ZeroMemory(pCertName, (dwData + 1) * sizeof(wchar_t));

		if (!(CertGetNameStringW(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, pCertName, dwData)))
			break;



	} while (FALSE);

	SafeDeleteArraySize(pBuff);
	SafeDeleteArraySize(pSignerInfo);
	if (pCertContext != NULL) CertFreeCertificateContext(pCertContext);
	if (hStore != NULL) CertCloseStore(hStore, 0);
	if (hMsg != NULL) CryptMsgClose(hMsg);

	return pCertName;
}


//����ļ��Ƿ���ǩ��
wchar_t* MyTools::GetFileCat(wchar_t* lpFileName)
{
	WINTRUST_DATA wd = { 0 };
	WINTRUST_FILE_INFO wfi = { 0 };
	WINTRUST_CATALOG_INFO wci = { 0 };
	CATALOG_INFO ci = { 0 };
	HCATADMIN hCatAdmin = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwCnt = 0;
	PBYTE pbyHash = NULL;
	wchar_t* pszMemberTag = NULL;
	HCATINFO hCatInfo = NULL;
	HRESULT hr;
	static GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	const GUID gSubsystem = DRIVER_ACTION_VERIFY;
	wchar_t* pCatalogFile = NULL;




	do
	{

		if (!CryptCATAdminAcquireContext(&hCatAdmin, &gSubsystem, 0))
			break;

		if (!RedirectionCreateFile(lpFileName, hFile))
			break;

		if (CryptCATAdminCalcHashFromFileHandle(hFile, &dwCnt, pbyHash, 0) && dwCnt > 0 && ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			pbyHash = new BYTE[dwCnt];
			ZeroMemory(pbyHash, dwCnt);
			if (CryptCATAdminCalcHashFromFileHandle(hFile, &dwCnt, pbyHash, 0) == FALSE)
			{
				CloseHandle(hFile);
				break;
			}
		}
		else
		{
			CloseHandle(hFile);
			break;
		}
		CloseHandle(hFile);

		hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, pbyHash, dwCnt, 0, NULL);
		if (NULL == hCatInfo)
		{
			wfi.cbStruct = sizeof(WINTRUST_FILE_INFO);
			wfi.pcwszFilePath = lpFileName;
			wfi.hFile = NULL;
			wfi.pgKnownSubject = NULL;
			wd.cbStruct = sizeof(WINTRUST_DATA);
			wd.dwUnionChoice = WTD_CHOICE_FILE;
			wd.pFile = &wfi;
			wd.dwUIChoice = WTD_UI_NONE;
			wd.fdwRevocationChecks = WTD_REVOKE_NONE;
			wd.dwStateAction = WTD_STATEACTION_IGNORE;
			wd.dwProvFlags = WTD_SAFER_FLAG;
			wd.hWVTStateData = NULL;
			wd.pwszURLReference = NULL;
		}
		else
		{
			if (CryptCATCatalogInfoFromContext(hCatInfo, &ci, 0))
			{
				pszMemberTag = new wchar_t[dwCnt * 2 + 1];
				ZeroMemory(pszMemberTag, (dwCnt * 2 + 1) * sizeof(wchar_t));
				for (DWORD dw = 0; dw < dwCnt; ++dw)
				{
					wsprintfW(&pszMemberTag[dw * 2], L"%02X", pbyHash[dw]);

				}

				wci.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
				wci.pcwszCatalogFilePath = ci.wszCatalogFile;
				wci.pcwszMemberFilePath = lpFileName;
				wci.pcwszMemberTag = pszMemberTag;

				wd.cbStruct = sizeof(WINTRUST_DATA);
				wd.pCatalog = &wci;
				wd.dwUIChoice = WTD_UI_NONE;
				wd.dwUnionChoice = WTD_CHOICE_CATALOG;
				wd.fdwRevocationChecks = WTD_STATEACTION_VERIFY;
				wd.dwStateAction = WTD_STATEACTION_VERIFY;
				wd.dwProvFlags = 0;
				wd.hWVTStateData = NULL;
				wd.pwszURLReference = NULL;

			}


		}


		hr = WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &action, &wd);
		if (SUCCEEDED(hr) || wcslen(ci.wszCatalogFile) > 0)
		{
			//����cat�ļ�
			pCatalogFile = new wchar_t[MAX_PATH];
			ZeroMemory(pCatalogFile, MAX_PATH * sizeof(wchar_t));
			CopyMemory(pCatalogFile, ci.wszCatalogFile, wcslen(ci.wszCatalogFile) * sizeof(wchar_t));
		}
		if (NULL != hCatInfo)
		{
			CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
		}


	} while (FALSE);


	if (hCatAdmin)
	{
		CryptCATAdminReleaseContext(hCatAdmin, 0);
	}




	SafeDeleteArraySize(pbyHash);
	SafeDeleteArraySize(pszMemberTag);
	return pCatalogFile;
}

//��ȡ�ļ�����ǩ��
wchar_t* MyTools::GetFileCertNameA(wchar_t* pFilePath)
{
	wchar_t* pCertName = NULL;
	wchar_t* pCatFilePath = NULL;

	//��ȡ�ļ�����ǩ��
	pCertName = GetCertName(pFilePath);
	if (pCertName == NULL)
	{
		//��ȡ�ļ�cat
		pCatFilePath = GetFileCat(pFilePath);
		if (pCatFilePath)
		{
			//��ȡcat�ļ�����ǩ��
			pCertName = GetCertName(pCatFilePath);
		}
	}

	SafeDeleteArraySize(pCatFilePath);
	return pCertName;
}
BOOL MyTools::CheckFileTrust(wchar_t* lpFileName)
{
	HANDLE hFile;
	if (!RedirectionCreateFile(lpFileName, hFile))
		return true;
	if (GetFileCertNameA(lpFileName) != NULL)
	{
		//��ǩ�������������֤�ǲ����ڰ��������
		//printf("%ws \n", GetFileCertNameA(lpFileName));
		return true;
	}
	else
		return false;
}
std::string MyTools::PID2FilePatch(DWORD process_id)
{
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
	if (process == NULL)
		return std::string();
	char file_path[MAX_PATH] = { 0 };
	GetModuleFileNameEx(process, NULL, file_path, MAX_PATH);
	CloseHandle(process);
	return std::string(file_path);
}
