#ifndef __LOWLIB_AUTODETECTMEMORYLEAK_H__
#define __LOWLIB_AUTODETECTMEMORYLEAK_H__

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC // �޸� ���� Ž���� ���ؼ� ������ �־�� �Ѵ�.
#include <crtdbg.h>
#ifndef _CONSOLE
#include <cstdlib> // �ܼ� ���α׷��� �ƴ� ���, ������ �־�� �Ѵ�.
#endif // _CONSOLE

class _AutoDetectMemoryLeak {
public:
	_AutoDetectMemoryLeak(){
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#ifdef _CONSOLE // �ܼ� ���α׷��� ���, ��� Report�� STDOUT���� ������.
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

// ������ ������ �߻��Ͽ� �ּ� ó��
// ���� ���� : ����	2	error C2660: 'MemoryPool<Stream,50>::operator new' : �Լ��� 4���� �Ű� ������ ������� �ʽ��ϴ�.	c:\work\ws\visstu\serversolution\lowlib\smartpointer.h	10	1	Server
//#define DEBUG_NORMALBLOCK new ( _NORMAL_BLOCK, __FILE__, __LINE__ )
//#ifdef new
//#undef new
//#endif // new
//#define new DEBUG_NORMALBLOCK

#else // _CONSOLE : �ܼ� ���α׷��� �ƴ� ���, ��� Report�� DEBUG Window�� ������.
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

#endif // _CONSOLE

#ifdef malloc
#undef malloc
#endif // malloc
#define malloc(s) (_malloc_dbg( s, _NORMAL_BLOCK, __FILE__, __LINE__ ))
	}
};

// �ʱ�ȭ�� �����ڸ� ���� �ڵ����� ���ֱ� ���ؼ� ��ü�� ���������� �����մϴ�.
static _AutoDetectMemoryLeak _autoDetectMemoryLeak;

#endif // _MSC_VER && _DEBUG
#endif // __LOWLIB_AUTODETECTMEMORYLEAK_H__