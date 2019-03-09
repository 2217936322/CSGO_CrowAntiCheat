#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once
#include "stdafx.h" 

#include <winsock2.h>
#include<iostream>

#define MAX_BUFFER_LEN 8196    
#define DEFAULT_PORT          12345                      // Ĭ�϶˿�
#define DEFAULT_LOCAL_IP            _T("127.0.0.1")            // Ĭ��IP��ַ
#define DEFAULT_SERVER_IP    _T("192.168.0.104") 
#define DEFAULT_THREADS       100                        // Ĭ�ϲ����߳���
#define DEFAULT_MESSAGE       _T("Hello!")   // Ĭ�ϵķ�����Ϣ


class CClient
{
public:
	CClient(void);
	~CClient(void);

public:
	// ����Socket��
	bool LoadSocketLib();
	// ж��Socket�⣬��������
	void UnloadSocketLib() { WSACleanup(); }
	// ��������IP��ַ
	void SetIP(const CString& strIP) { m_strServerIP = strIP; }
	// ���ü����˿�
	void SetPort(const int& nPort) { m_nPort = nPort; }
	void SockSendMessage(const CString& strMessage);
	// ���������������
	bool ConnetToServer();
private:

	CString   m_strServerIP;                                // �������˵�IP��ַ
	CString   m_strLocalIP;                                 // ����IP��ַ								
	SOCKET    sClient;                                 		//���������׽���      
	int       m_nPort;                                      // �����˿�
};
