// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once
//#define _AFXDLL
#define  _CRT_SECURE_NO_WARNINGS 
#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include <afx.h>
#include <winsvc.h>
#include <winioctl.h>
#include <stdio.h>

#include <stdlib.h>
#include <winioctl.h>
#include <string.h>
#include <crtdbg.h>
#include <assert.h>
#include <fltuser.h>

#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <mscat.h>
#include <wchar.h>
#include<winnt.h>
#include <string>
#include <Wincrypt.h>
#include <Psapi.h>
#include"tlhelp32.h"
#pragma comment(lib, "wintrust")
#define DRIVER_NAME "test"
#define DRIVER_PATH ".\\test.sys"
#define	DRIVER_ALTITUDE	"370030"
#define IOCTL_START	0x80001
#define IOCTL_GET_DATA 0x80002
typedef enum _SCANCHEAT_TYPE {
	SCANTYPE_FAST = 0, //ֻ���MD5
	SCANTYPE_HIIGHT //���md5 + ɨ��ģ�� + �ֹ�����ģ�� + ����ʽɨ��(�ϴ������)
} NSCANCHEAT_TYPE;
