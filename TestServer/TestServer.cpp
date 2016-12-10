// TestServer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.

#include "stdafx.h"
#include "TestSession.h"
#include "TestIOCP.h"

int _tmain(int argc, _TCHAR* argv[]){
	
	// Winsock DLL 2.2 ������ �ε��մϴ�.
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	TestIOCP iocp;

	iocp.begin();

	_tprintf(_T("Test Server started at port %d\n"), TestConstant::DEFAULT_PORT);

	// ���� �����带 ��� ���·� ����� ����.
	getchar();

	iocp.end();

	WSACleanup();

	return 0;
}
