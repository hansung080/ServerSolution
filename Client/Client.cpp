// Client.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
#include "stdafx.h"
#include "TestClientSession.h"
#include "MainClient.h"

int _tmain(int argc, _TCHAR* argv[]){
	// Winsock DLL 2.2 ���� �ε�
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	TestClientSession* clientSession = new TestClientSession();
	MainClient* mainClient = new MainClient(clientSession);

	mainClient->start();

	delete clientSession;
	delete mainClient;

	WSACleanup();

	return 0;
}