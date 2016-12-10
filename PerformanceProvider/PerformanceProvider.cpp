// PerformanceProvider.cpp : DLL ���� ���α׷��� ���� ������ �Լ��� �����մϴ�.
/**
  ������ ���� ������� �߰��ٸ� ��Ȱ�� �ϴ� ���� ���� ����� DLL(PerformanceProvider.dll)�Դϴ�.
  ���������� ��������� ī���� �����͸� ������Ʈ�ϰ� ���� ����Ϳ����� DLL�� ���ؼ� �� ���� �޾ư��� �����Դϴ�.
*/
#include "stdafx.h"
#include "PerformanceInfo.h"

static std::vector<PERFORMANCE_COUNTER> gCounterVector;
static CriticalSection gSync;
static PERF_OBJECT_TYPE gHeader;
static TCHAR gReadString[MAX_STRING_LENGTH] = {0, };
static DWORD gReadStringLength = sizeof(gReadString);
static TCHAR gReadHelpString[MAX_STRING_LENGTH] = {0, };
static DWORD gReadHelpStringLength = sizeof(gReadHelpString);
static TCHAR gObjectListString[MAX_STRING_LENGTH] = {0, };
static DWORD gLastIndex = 0;
static BYTE gEnd[2] = {0, 0};

BOOL AddString(LPTSTR source, LPTSTR addItem, DWORD &length){
	if(!source || !addItem){
		return FALSE;
	}

	CopyMemory(source + length / sizeof(TCHAR) - 1, addItem, _tcslen(addItem) * sizeof(TCHAR));
	length += (DWORD)_tcslen(addItem) * sizeof(TCHAR);

	CopyMemory(source + length / sizeof(TCHAR), gEnd, sizeof(gEnd));
	length += sizeof(gEnd);

	return TRUE;
}

BOOL AddDWORD(LPTSTR source, DWORD addItem, DWORD &length){
	if(!source){
		return FALSE;
	}

	TCHAR AddItem[16] = {0, };
	_sntprintf(AddItem, 16, _T("%d"), addItem);

	CopyMemory(source + length / sizeof(TCHAR) - 1, AddItem, _tcslen(AddItem) * sizeof(TCHAR));
	length += (DWORD)_tcslen(AddItem) * sizeof(TCHAR);

	CopyMemory(source + length / sizeof(TCHAR), gEnd, sizeof(gEnd));
	length += sizeof(gEnd);

	return TRUE;
}

BOOL GetIndex(LPTSTR source, LPTSTR counterName, DWORD &index){
	TCHAR* PreviosString = NULL;
	DWORD Length = (DWORD)_tcslen(source) + 1;

	if(!_tcscmp(source, counterName)){
		return FALSE;
	}

	while(Length){
		if(!_tcscmp(source + Length, counterName)){
			index = _ttoi(PreviosString);
			return TRUE;
		}

		PreviosString = source + Length;
		Length += (DWORD)_tcslen(source + Length) + 1;
	}

	return FALSE;
}

BOOL GetName(LPTSTR source, DWORD index, LPTSTR counterName){
	TCHAR* PreviosString = NULL;
	DWORD Length = (DWORD)_tcslen(source) + 1;
	TCHAR Index[16] = {0, };

	_itot(index, Index, 10);

	if(!_tcscmp(source, Index)){
		return FALSE;
	}

	while(Length){
		if(!_tcscmp(source + Length, Index)){
			Length += (DWORD)_tcslen(source + Length) + 1;
			_tcscpy(counterName, source + Length);
			return TRUE;
		}

		PreviosString = source + Length;
		Length += (DWORD)_tcslen(source + Length) + 1;
	}

	return FALSE;
}

extern "C" {
	/**
	  - �Լ�[Begin, End, AddCounter, UpdateCounter]�� ������ �Լ��Դϴ�.
	  - ������ �Լ������� �ڽ��� �ʿ��� ī���͸� �������Ϳ� �߰��ϰ� �� ���� �о� �޸𸮿� ������ �ִٰ� 
	    ���� ����Ǹ� ������Ʈ�ϴ� ���� �ֵ� ����Դϴ�.
	*/
	BOOL __declspec(dllexport) Begin(LPTSTR dllPath){
		gSync.enter();

		if(!dllPath){
			gSync.leave();
			return FALSE;
		}

		ZeroMemory(&gHeader, sizeof(gHeader));

		Registry Registry;

		Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\012"));
		Registry.getValue(_T("Counter"), gReadString, &gReadStringLength);
		Registry.getValue(_T("Help"), gReadHelpString, &gReadHelpStringLength);
		Registry.close();

		Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"));
		Registry.getValue(_T("Last Counter"), &gLastIndex);
		Registry.close();

		if(!Registry.open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\Performance Provider\\Performance"))){
			Registry.createKey(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\Performance Provider\\Performance"));

			_sntprintf(gObjectListString, MAX_STRING_LENGTH, _T("%d"), gLastIndex + 2);
			Registry.setValue(_T("Object List"), gObjectListString);
			Registry.setValue(_T("Open"), _T("PP_Open"));
			Registry.setValue(_T("Close"), _T("PP_Close"));
			Registry.setValue(_T("Collect"), _T("PP_Collect"));
			Registry.setValue(_T("Library"), dllPath);
			Registry.close();

			Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"));
			Registry.setValue(_T("Last Counter"), gLastIndex + 2);
			Registry.setValue(_T("Last Help"), gLastIndex + 3);
			Registry.close();

			AddDWORD(gReadString, gLastIndex + 2, gReadStringLength);
			AddString(gReadString, _T("Performance Provider"), gReadStringLength);

			AddDWORD(gReadHelpString, gLastIndex + 3, gReadHelpStringLength);
			AddString(gReadHelpString, _T("Performance Provider Help"), gReadHelpStringLength);

			Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\012"));
			Registry.setValueForMultiSz(_T("Counter"), gReadString, gReadStringLength);
			Registry.setValueForMultiSz(_T("Help"), gReadHelpString, gReadHelpStringLength);
			Registry.close();

			gLastIndex = gLastIndex + 2;

		}else{
			DWORD MaxStringLength = MAX_STRING_LENGTH;

			Registry.getValue(_T("Object List"), gObjectListString, &MaxStringLength);
			Registry.close();
		}

		TCHAR Seperator[] = _T(" ");
		LPTSTR Token = NULL;

		Token = _tcstok(gObjectListString, Seperator);

		DWORD FirstIndex = _ttoi(Token);

		gHeader.TotalByteLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION) + sizeof(gHeader) + sizeof(DWORD) * gCounterVector.size() + sizeof(DWORD);
		gHeader.DefinitionLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION) + sizeof(gHeader);
		gHeader.HeaderLength = sizeof(gHeader);
		gHeader.ObjectNameTitleIndex = FirstIndex;
		gHeader.ObjectNameTitle = NULL;
		gHeader.ObjectHelpTitleIndex = FirstIndex + 1;
		gHeader.ObjectHelpTitle = NULL;
		gHeader.DetailLevel = PERF_DETAIL_NOVICE;
		gHeader.NumCounters = (DWORD)gCounterVector.size();
		gHeader.DefaultCounter = 0;
		gHeader.NumInstances = PERF_NO_INSTANCES;
		gHeader.CodePage = 0;

		ZeroMemory(&gHeader.PerfTime, sizeof(gHeader.PerfTime));
		ZeroMemory(&gHeader.PerfFreq, sizeof(gHeader.PerfFreq));

		Token = _tcstok(NULL, Seperator);

		while(Token){
			DWORD Index = _ttoi(Token);

			PERFORMANCE_COUNTER pc;

			ZeroMemory(&pc, sizeof(pc));

			pc.Index = Index;
			pc.Value = 0;

			pc.Data.ByteLength = sizeof(pc.Data);
			pc.Data.CounterNameTitleIndex = Index;
			pc.Data.CounterNameTitle = NULL;
			pc.Data.CounterHelpTitleIndex = Index + 1;
			pc.Data.CounterHelpTitle = NULL;
			pc.Data.DefaultScale = 0;
			pc.Data.DetailLevel = PERF_DETAIL_NOVICE;
			pc.Data.CounterType = PERF_COUNTER_RAWCOUNT;
			pc.Data.CounterSize = sizeof(DWORD);
			pc.Data.CounterOffset = sizeof(DWORD);

			GetName(gReadString, Index, pc.Name);

			gCounterVector.push_back(pc);

			Token = _tcstok(NULL, Seperator);
		}

		gHeader.TotalByteLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION) + sizeof(gHeader) + sizeof(DWORD) * gCounterVector.size() + sizeof(DWORD);
		gHeader.DefinitionLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION) + sizeof(gHeader);
		gHeader.NumCounters = (DWORD)gCounterVector.size();

		gSync.leave();

		return TRUE;
	}

	BOOL __declspec(dllexport) End(VOID){
		return TRUE;
	}

	BOOL __declspec(dllexport) AddCounter(LPTSTR name){
		gSync.enter();

		if(!name){
			gSync.leave();
			return FALSE;
		}

		for(DWORD i = 0; i < gCounterVector.size(); i++){
			if(!_tcscmp(gCounterVector[i].Name, name)){
				gSync.leave();
				return FALSE;
			}
		}

		Registry Registry;
		Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"));
		Registry.getValue(_T("Last Counter"), &gLastIndex);
		Registry.close();

		// Step 1. String�ʿ� �־� �ְ�, ������ �Է��ϰ�, ������Ʈ���� ���� ������Ʈ�մϴ�.
		AddDWORD(gReadString, gLastIndex + 2, gReadStringLength);
		AddString(gReadString, name, gReadStringLength);

		AddDWORD(gReadHelpString, gLastIndex + 3, gReadHelpStringLength);
		AddString(gReadHelpString, name, gReadHelpStringLength);

		_sntprintf(gObjectListString, MAX_STRING_LENGTH, _T("%s %d"), gObjectListString, gLastIndex + 2);

		Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\012"));
		Registry.setValueForMultiSz(_T("Counter"), gReadString, gReadStringLength);
		Registry.setValueForMultiSz(_T("Help"), gReadHelpString, gReadHelpStringLength);
		Registry.close();

		// Step 2. Perflib�� Last Index ������Ʈ
		Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"));
		Registry.setValue(_T("Last Counter"), gLastIndex + 2);
		Registry.setValue(_T("Last Help"), gLastIndex + 3);
		Registry.close();

		// Step 3. Performance Provider�� �߰�
		Registry.open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\Performance Provider\\Performance"));
		Registry.setValue(_T("Object List"), gObjectListString);
		Registry.close();

		// Step 4. Vertor�� �߰� (�޸𸮿� ���� �ε�Ǿ� �ִ� ������ ������Ʈ)
		PERFORMANCE_COUNTER pc;

		pc.Index = gLastIndex + 2;
		pc.Value = 0;
		pc.Data.ByteLength = sizeof(pc.Data);
		pc.Data.CounterNameTitleIndex = gLastIndex + 2;
		pc.Data.CounterNameTitle = NULL;
		pc.Data.CounterHelpTitleIndex = gLastIndex + 3;
		pc.Data.CounterHelpTitle = NULL;
		pc.Data.DefaultScale = 0;
		pc.Data.DetailLevel = PERF_DETAIL_NOVICE;
		pc.Data.CounterType = PERF_COUNTER_RAWCOUNT;
		pc.Data.CounterSize = sizeof(DWORD);
		pc.Data.CounterOffset = sizeof(DWORD);

		GetName(gReadString, gLastIndex + 2, pc.Name);

		gCounterVector.push_back(pc);

		// Step 5. Header ����
		gHeader.TotalByteLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION) + sizeof(gHeader) + sizeof(DWORD) * gCounterVector.size() + sizeof(DWORD);
		gHeader.DefinitionLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION) + sizeof(gHeader);
		gHeader.NumCounters = (DWORD)gCounterVector.size();

		gLastIndex = gLastIndex + 2;

		gSync.leave();

		return TRUE;
	}

	BOOL __declspec(dllexport) UpdateCounter(LPTSTR name, DWORD value){
		gSync.enter();

		for(DWORD i = 0; i < gCounterVector.size(); i++){
			if(!_tcscmp(gCounterVector[i].Name, name)){
				gCounterVector[i].Value = value;
				gSync.leave();
				return TRUE;
			}
		}

		gSync.leave();

		return FALSE;
	}

	/**
	  - �Լ�[PP_Open, PP_Close, PP_Collect]�� ���� ������� �Լ��Դϴ�.
	  - ���� ������� �Լ������� �������� ������� ���� ī���� ���� ���� ����Ϳ� �ѱ�� ��Ȱ�� �մϴ�.
	*/
	DWORD __declspec(dllexport) WINAPI PP_Open(LPWSTR lpDevNames){
		gSync.enter();

		ZeroMemory(&gHeader, sizeof(gHeader));

		Registry Registry;

		Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\012"));
		Registry.getValue(_T("Counter"), gReadString, &gReadStringLength);
		Registry.getValue(_T("Help"), gReadHelpString, &gReadHelpStringLength);
		Registry.close();

		Registry.open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"));
		Registry.getValue(_T("Last Counter"), &gLastIndex);
		Registry.close();

		if(Registry.open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\Performance Provider\\Performance"))){
			DWORD MaxStringLength = MAX_STRING_LENGTH;

			Registry.getValue(_T("Object List"), gObjectListString, &MaxStringLength);

			Registry.close();

			TCHAR Seperator[] = _T(" ");
			LPTSTR Token = NULL;

			Token = _tcstok(gObjectListString, Seperator);

			DWORD FirstIndex = _ttoi(Token);

			gHeader.TotalByteLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION) + sizeof(gHeader) + sizeof(DWORD) * gCounterVector.size() + sizeof(DWORD);
			gHeader.DefinitionLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION) + sizeof(gHeader);
			gHeader.HeaderLength = sizeof(gHeader);
			gHeader.ObjectNameTitleIndex = FirstIndex;
			gHeader.ObjectNameTitle = NULL;
			gHeader.ObjectHelpTitleIndex = FirstIndex + 1;
			gHeader.ObjectHelpTitle = NULL;
			gHeader.DetailLevel = PERF_DETAIL_NOVICE;
			gHeader.NumCounters = (DWORD)gCounterVector.size();
			gHeader.DefaultCounter = 0;
			gHeader.NumInstances = PERF_NO_INSTANCES;
			gHeader.CodePage = 0;

			ZeroMemory(&gHeader.PerfTime, sizeof(gHeader.PerfTime));
			ZeroMemory(&gHeader.PerfFreq, sizeof(gHeader.PerfFreq));

			Token = _tcstok(NULL, Seperator);

			while(Token){
				DWORD Index = _ttoi(Token);

				PERFORMANCE_COUNTER pc;

				ZeroMemory(&pc, sizeof(pc));

				pc.Index = Index;
				pc.Value = 0;
				pc.Data.ByteLength = sizeof(pc.Data);
				pc.Data.CounterNameTitleIndex = Index;
				pc.Data.CounterNameTitle = NULL;
				pc.Data.CounterHelpTitleIndex = Index + 1;
				pc.Data.CounterHelpTitle = NULL;
				pc.Data.DefaultScale = 0;
				pc.Data.DetailLevel = PERF_DETAIL_NOVICE;
				pc.Data.CounterType = PERF_COUNTER_RAWCOUNT;
				pc.Data.CounterSize = sizeof(DWORD);
				pc.Data.CounterOffset = sizeof(DWORD);

				GetName(gReadString, Index, pc.Name);

				gCounterVector.push_back(pc);

				Token = _tcstok(NULL, Seperator);
			}

			gHeader.TotalByteLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION)+sizeof(gHeader)+sizeof(DWORD)* gCounterVector.size() + sizeof(DWORD);
			gHeader.DefinitionLength = (DWORD)gCounterVector.size() * sizeof(PERF_COUNTER_DEFINITION)+sizeof(gHeader);
			gHeader.NumCounters = (DWORD)gCounterVector.size();
		}

		gSync.leave();

		return(ERROR_SUCCESS);
	}

	DWORD __declspec(dllexport) WINAPI PP_Close(VOID){
		return(ERROR_SUCCESS);
	}

	DWORD __declspec(dllexport) WINAPI PP_Collect(LPWSTR lpValueName, LPVOID* lppData, LPDWORD lpcbTotalBytes, LPDWORD lpNumObjectTypes){
		gSync.enter();

		*lpcbTotalBytes = gHeader.TotalByteLength;
		*lpNumObjectTypes = (DWORD)gCounterVector.size() + 1;

		PBYTE* Data = (PBYTE*)lppData;

		CopyMemory(*Data, &gHeader, sizeof(gHeader));
		*Data += sizeof(gHeader);

		for(DWORD i = 0; i < gCounterVector.size(); i++){
			CopyMemory(*Data, &gCounterVector[i].Data, sizeof(gCounterVector[i].Data));
			*Data += sizeof(gCounterVector[i].Data);
		}

		DWORD ValueLength = (DWORD)(sizeof(DWORD)* gCounterVector.size());
		CopyMemory(*Data, &ValueLength, sizeof(DWORD));
		*Data += sizeof(DWORD);

		for(DWORD i = 0; i < gCounterVector.size(); i++){
			CopyMemory(*Data, &gCounterVector[i].Value, sizeof(DWORD));
			*Data += sizeof(DWORD);
		}

		gSync.leave();

		return(ERROR_SUCCESS);
	}
}

#pragma comment(linker, "/export:PP_Open=_PP_Open@4")
#pragma comment(linker, "/export:PP_Close=_PP_Close@0")
#pragma comment(linker, "/export:PP_Collect=_PP_Collect@16")
