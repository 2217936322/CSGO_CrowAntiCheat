#include "stdafx.h"
#include "PEreverse.h"
#include "dbghelp.h"
#pragma comment(lib,"Dbghelp.lib")
PackNamesList PackName;
//�������㵼�������������ڴ��ƫ����
DWORD PEreverse::RVA2Offset(PIMAGE_NT_HEADERS pNTHeader, DWORD dwExpotRVA)
{
	PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)((DWORD)pNTHeader + sizeof(IMAGE_NT_HEADERS));

	for (int i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
	{
		if (dwExpotRVA >= pSection[i].VirtualAddress && dwExpotRVA < (pSection[i].VirtualAddress + pSection[i].SizeOfRawData))
		{
			return pSection[i].PointerToRawData + (dwExpotRVA - pSection[i].VirtualAddress);
		}
	}

	return 0;
}
bool PEreverse::CheckName(const char * iName) {
	bool result = false;
	if (strlen(iName)>32)return result;//���Ȳ��ܳ���32���ַ�
	while (*iName)
	{
		if ((*iName >= 'a'&&*iName <= 'z' || *iName >= 'A'&&*iName <= 'Z') || ((*iName) >= '0' && (*iName) <= '9'))
		{
			result = true;
		}
		else {
			result = false;
			break;
		}
		iName++;
	}
	return result;
}

//ScanPack = �Ƿ��ϴ��ӿǳ��� / IsDll ���dll��ɨ��(GetModuleHandle��GetProcAddress��
int PEreverse::GetFileLevel(CString Patch, bool ScanPack/*, bool IsDll*/)
{
	int LevelNumber = 0;
	//int LevelNumber_ExternWindow = -1;
	int LevelNumber_DllLoader = 0;
	int LevelNumber_DriverLoader = 0;
	PEInportTableList ImportTableFunctionList;
	PEInportTableList::iterator i;
	string art, PackName;
	DWORD rangeStart, SizeOfImage;
	if (GetFileImportTable(Patch, ImportTableFunctionList, PackName) == PEREVERSESTATUS_SUCCESS)
	{
		
		for (i = ImportTableFunctionList.begin(); i != ImportTableFunctionList.end(); ++i)
			art += *i;
		//�����Ǽ��dllע�빤�ߵ� > 40 ������
		if (art.find("LoadLibrary") != string::npos)
			LevelNumber_DllLoader += 10, LevelNumber += 10;
		if (art.find("CreateToolhelp32Snapshot") != string::npos)
			LevelNumber_DllLoader += 10, LevelNumber += 10;
		if (art.find("CreateRemoteThread") != string::npos)
			LevelNumber_DllLoader += 20, LevelNumber += 10;
		if (art.find("ReadProcessMemory") != string::npos)
			LevelNumber_DllLoader += 20, LevelNumber += 10;
		if (art.find("OpenProcess") != string::npos)
			LevelNumber_DllLoader += 20, LevelNumber += 10;
		if (art.find("CreateProcess") != string::npos)
			LevelNumber_DllLoader += 10, LevelNumber += 10;
		if (art.find("CreateFile") != string::npos)
			LevelNumber_DllLoader += 10, LevelNumber += 10;
		if (art.find("WriteProcessMemory") != string::npos)
			LevelNumber_DllLoader += 20, LevelNumber += 10;
		if (art.find("Process32Next") != string::npos)
			LevelNumber_DllLoader += 10, LevelNumber += 10;
		//û��Ҫ���ж�ResumeThread����Ϊ������������������һ��
		if (art.find("SuspendThread") != string::npos)
			LevelNumber_DllLoader += 10, LevelNumber += 10;
		if (art.find("Thread32First") != string::npos)
			LevelNumber_DllLoader += 10, LevelNumber += 10;
		if (art.find("OpenThread") != string::npos)
			LevelNumber_DllLoader += 10, LevelNumber += 10;
		//�����������  > 40 ������
		if (art.find("DeviceIoControl") != string::npos)
			LevelNumber_DriverLoader += 20, LevelNumber += 10;
		if (art.find("OpenSCManager") != string::npos)
			LevelNumber_DriverLoader += 10, LevelNumber += 10;
		if (art.find("OpenService") != string::npos)
			LevelNumber_DriverLoader += 10, LevelNumber += 10;
		if (art.find("CloseServiceHandle") != string::npos)
			LevelNumber_DriverLoader += 10, LevelNumber += 10;
		if (art.find("CreateService") != string::npos)
			LevelNumber_DriverLoader += 10, LevelNumber += 10;
		if (art.find("StartService") != string::npos)
			LevelNumber_DriverLoader += 10, LevelNumber += 10;
		//�ڶ���: ��Ǻ�������:
		if ((PackName != "None" && ScanPack) || LevelNumber > 60 || LevelNumber_DriverLoader > 50 || LevelNumber_DllLoader > 40)
		{
			//�ϴ��ļ��������
			//...
			return FILELEVEL_HIGHT;
		}

	}
	else
	{
		printf("GetFileImportTable Error %d \n",GetLastError());
		return FILELEVEL_ERROR;
	}
	printf("The Number :%d \n", LevelNumber);
	return FILELEVEL_NORMAL;
}

string PEreverse::GetFilePackName(HANDLE FileHandle)
{
	string Result = "None";
	//���Ҫ�����ƶ�
	CFindPatternEx* FindPattern = new CFindPatternEx(FileHandle);
	for (int i = 0; i < 1; i++) //��ʱģ��һ����,���Ҫ�����ƶ˻�ȡ
	{
		PackNames temp;
		temp.Name = "VMProtect v.3.00 - 3.0x";
		temp.Sig = "EB 08 66 F8 15 00 00 00 00 00";
		PackName.push_back(temp);
	}
	PackNamesList::iterator i;
	for (i = PackName.begin(); i != PackName.end(); ++i)
	{
		PackNames art = *i;

		if (FindPattern->FindPatternEx(art.Sig))
		{
			Result = art.Name;
			printf("%s \n", art.Name.c_str());
		}
	}


	//printf("m_flFlashMaxAlpha : 0x%0x\n", FindPattern->FindPatternEx("0F 2F F2 0F 87 ? ? ? ? F3 0F 10 A1 ? ? ? ? 0F 2F CC 0F 83"));

	free(FindPattern); //��free�����
	return Result;
}
int PEreverse::GetFileImportTable(CString Patch, PEInportTableList &TableList,string &PackName)
{
	int i, j;
	HANDLE hFile = CreateFile(
		Patch, //PE�ļ���
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Create File Failed.\n");
		return PEREVERSESTATUS_ERRORMAPOFVIEW;
	}
	PackName = GetFilePackName(hFile);
	//return 0;
	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

	if (hFileMapping == NULL || hFileMapping == INVALID_HANDLE_VALUE)
	{
		printf("Could not create file mapping object (%d).\n", GetLastError());
		return PEREVERSESTATUS_ERRORMAPOFVIEW;
	}

	LPBYTE lpBaseAddress = (LPBYTE)MapViewOfFile(hFileMapping,   // handle to map object
		FILE_MAP_READ, 0, 0, 0);
	
	//ReadByte(lpBaseAddress,12);
	if (lpBaseAddress == NULL)
	{
		printf("Could not map view of file (%d).\n", GetLastError());
		return PEREVERSESTATUS_ERRORMAPOFVIEW;
	}
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(lpBaseAddress + pDosHeader->e_lfanew);
	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("\n������ȷ��PE�ļ� %s \n", Patch);
		UnmapViewOfFile(lpBaseAddress);
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return PEREVERSESTATUS_ERRORPEFILE;
	}
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeaders->OptionalHeader;
	//������rva��0x2a000;
	DWORD Rva_import_table;
	if ((int)pNtHeaders->FileHeader.SizeOfOptionalHeader == 224)
	{
		PIMAGE_NT_HEADERS32 pNtHeaders2 = (PIMAGE_NT_HEADERS32)(lpBaseAddress + pDosHeader->e_lfanew);
		Rva_import_table = pNtHeaders2->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	}
	else
		Rva_import_table = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if (Rva_import_table == 0)
	{
		printf("no import table!");
		UnmapViewOfFile(lpBaseAddress);
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return PEREVERSESTATUS_ERRORPEFILE;
	}
	DWORD* FileShit = (DWORD*)ImageRvaToVa(
		pNtHeaders, lpBaseAddress,
		pNtHeaders->OptionalHeader.AddressOfEntryPoint,
		NULL);
	//int NumberOfRead = 0;
	//printf("Byte: %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X.\n", FileShit[NumberOfRead], FileShit[NumberOfRead + 1], FileShit[NumberOfRead + 2], FileShit[NumberOfRead + 3], FileShit[NumberOfRead + 4], FileShit[NumberOfRead + 5], FileShit[NumberOfRead + 6], FileShit[NumberOfRead + 7], FileShit[8], FileShit[NumberOfRead + 9], FileShit[NumberOfRead + 10], FileShit[NumberOfRead + 11]);
	//return 0;
	//�����Ȼ���ڴ��ַ�����Ǽ�ȥ�ļ���ͷ�ĵ�ַ�������ļ���ַ��
	//�����ַ����ֱ�Ӵ������ȡ����Ҫ�Ķ�����
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)ImageRvaToVa(
		pNtHeaders,
		lpBaseAddress,
		Rva_import_table,
		NULL
	);
	//��ȥ�ڴ�ӳ����׵�ַ�������ļ���ַ�ˡ������ܼ򵥰ɣ�
	//printf("FileAddress Of ImportTable: %p\n", ((DWORD)pImportTable - (DWORD)lpBaseAddress));
	IMAGE_IMPORT_DESCRIPTOR null_iid;
	IMAGE_THUNK_DATA null_thunk;
	memset(&null_iid, 0, sizeof(null_iid));
	memset(&null_thunk, 0, sizeof(null_thunk));

	//ÿ��Ԫ�ش�����һ�������DLL��
	for (i = 0; memcmp(pImportTable + i, &null_iid, sizeof(null_iid)) != 0; i++)
	{
		LPCSTR szDllName = (LPCSTR)ImageRvaToVa(
			pNtHeaders, lpBaseAddress,
			pImportTable[i].Name, //DLL���Ƶ�RVA
			NULL);
	//  �õ���DLL������
	//	printf("-----------------------------------------\n");
	//	printf("[%d]: %s\n", i, szDllName);
	//	printf("-----------------------------------------\n");

		//����ȥ�����Ӹ�DLL����������Щ����
		//����������DLL�� IMAGE_TRUNK_DATA ���飨IAT�������ַ��ǰ��
		PIMAGE_THUNK_DATA32 pThunk = (PIMAGE_THUNK_DATA32)ImageRvaToVa(
			pNtHeaders, lpBaseAddress,
			pImportTable[i].OriginalFirstThunk, //��ע�⡿����ʹ�õ���OriginalFirstThunk
			NULL);

		for (j = 0; memcmp(pThunk + j, &null_thunk, sizeof(null_thunk)) != 0; j++)
		{
			PIMAGE_IMPORT_BY_NAME pFuncName = (PIMAGE_IMPORT_BY_NAME)ImageRvaToVa(
				pNtHeaders, lpBaseAddress,
				pThunk[j].u1.AddressOfData,
				NULL);
			if (pFuncName != NULL)
			{
				if (CheckName(pFuncName->Name))
				{
					TableList.push_back(pFuncName->Name);
					//printf("\t [%d] \t %ld \t %s\n", j, pFuncName->Hint, pFuncName->Name);
				}
			}
		}
	}
	//�ر��ļ����������
	UnmapViewOfFile(lpBaseAddress);
	CloseHandle(hFileMapping);
	CloseHandle(hFile);
	return PEREVERSESTATUS_SUCCESS;
}
