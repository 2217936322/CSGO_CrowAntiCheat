
#include"stdafx.h"
#include"Client.h"

CClient::CClient(void) :
	m_strServerIP(DEFAULT_SERVER_IP),
	m_strLocalIP(DEFAULT_LOCAL_IP),
	m_nPort(DEFAULT_PORT)

{
}

CClient::~CClient(void)
{

}

////////////////////////////////////////////////////////////////////
// ��ʼ��WinSock 2.2
bool CClient::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != nResult)
	{
		return false; // ����
	}
	return true;
}

void CClient::SockSendMessage(const CString & strMessage)
{
	send(sClient, strMessage, strMessage.GetLength(), 0); //sClientָ�����ĸ����ӷ��ͣ� szMessageָ�����������ݵı����ַ ��strlen(szMessage)ָ�����ݳ��� 
}

bool CClient::ConnetToServer()
{
	// �����ͻ����׽���      
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //AF_INETָ��ʹ��TCP/IPЭ���壻      
														 //SOCK_STREAM, IPPROTO_TCP����ָ��ʹ��TCPЭ��   
	if (INVALID_SOCKET == sClient)
	{
		printf("���󣺳�ʼ��Socketʧ�ܣ�������Ϣ��%d\n", WSAGetLastError());
		return false;
	}
	//����Զ�̷������ĵ�ַ��Ϣ      
	SOCKADDR_IN server;
	// ָ��Զ�̷������ĵ�ַ��Ϣ(�˿ںš�IP��ַ��)      
	memset(&server, 0, sizeof(SOCKADDR_IN)); //�Ƚ������ַ��server��Ϊȫ0      
	server.sin_family = PF_INET; //������ַ��ʽ��TCP/IP��ַ��ʽ      
	server.sin_port = htons(m_nPort); //ָ�����ӷ������Ķ˿ںţ�htons()���� converts values between the host and network byte order      
	//ָ�����ӷ�������IP��ַ      
	//�ṹSOCKADDR_IN��sin_addr�ֶ����ڱ���IP��ַ��sin_addr�ֶ�Ҳ��һ���ṹ�壬sin_addr.s_addr�������ձ���IP��ַ      
	//inet_addr()���ڽ� �����"127.0.0.1"�ַ���ת��ΪIP��ַ��ʽ 
	server.sin_addr.s_addr = inet_addr(m_strServerIP);   

	// ��ʼ���ӷ�����
	//�����ղ�ָ���ķ�������      
	//���Ӻ������sClient��ʹ���������      
	//server������Զ�̷������ĵ�ַ��Ϣ 
	if (SOCKET_ERROR == connect(sClient, (struct sockaddr *) &server, sizeof(SOCKADDR_IN)))
	{
		closesocket(sClient);
		printf("����������������ʧ�ܣ�\n");
		return false;
	}

	return true;
}