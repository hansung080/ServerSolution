// Server.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[]){
	// ���� �����忡�� COM library �ʱ�ȭ
	CoInitialize(NULL);

	// Winsock DLL 2.2 ���� �ε�
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if(!MiniDump::begin()){
		return 1;
	}

	ServerIOCP serverIOCP;
	if(serverIOCP.begin()){
		_tprintf(_T("Server started at port <%d>\n"), ServerConstant::DEFAULT_PORT);
		getchar(); // ���� �����带 ��� ���·� ����� ����.
		serverIOCP.end();
	}

	MiniDump::end();
	WSACleanup();
	CoUninitialize();

	return 0;
}
