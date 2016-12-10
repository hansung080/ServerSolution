#include "stdafx.h"
#include "ProtocolInfo.h"
#include "File.h"
#include "ProtocolFile.h"

ProtocolFile::ProtocolFile(VOID){
}

ProtocolFile::~ProtocolFile(VOID){
}

BOOL ProtocolFile::read(VOID* buffer, DWORD& protocolPos){
	PROTOCOL* protocols = reinterpret_cast<PROTOCOL*>(buffer);
	TCHAR data[MAX_LINE_LENGTH] = {0, };
	BOOL isParamOpen = FALSE;
	DWORD paramPos = 0;

	// srcFile���� �� ���ξ� �о data ���ۿ� ����.
	while(_fgetts(data, MAX_LINE_LENGTH, mFile)){
		TCHAR tempData[MAX_LINE_LENGTH] = {0, };
		DWORD tempDataPos = 0;

		for(DWORD i = 0; i < _tcslen(data); i++){
			if(data[i] != ' ' && data[i] != '\t' && data[i] != '\n' && data[i] != '\r'){
				tempData[tempDataPos++] = data[i];
			}
		}

		TCHAR* pos = _tcsstr(tempData, _T(","));
		// �ش� ���ο� ','�� �����ϴ� ���, Protocol ��.
		if(pos){
			if(!_tcsstr(tempData, _T("VERSION"))){
				_tcsncpy(protocols[protocolPos++].name, tempData, pos - tempData);
			}

		// �ش� ���ο� ','�� �������� �ʴ� ���, Parameter ��.
		}else{
			if(_tcsstr(tempData, _T("/*"))){
				isParamOpen = TRUE;
				paramPos = 0;
				continue;
			}

			if(_tcsstr(tempData, _T("*/"))){
				isParamOpen = FALSE;
				protocols[protocolPos - 1].paramCount = paramPos;
				paramPos = 0;
				continue;
			}

			if(_tcslen(tempData) != 0){
				if(isParamOpen){
					pos = _tcsstr(tempData, _T(":"));
					if(pos){
						_tcsncpy(protocols[protocolPos - 1].params[paramPos++].type, tempData, pos - tempData);
					}else{
						continue;
					}

					TCHAR* startPos = _tcsstr(tempData, _T("["));
					if(startPos){
						TCHAR* endPos = _tcsstr(tempData, _T("]"));
						TCHAR length[MAX_LINE_LENGTH] = { 0, };

						_tcsncpy(length, startPos + 1, endPos - startPos - 1);
						protocols[protocolPos - 1].params[paramPos - 1].length = _tstoi(length);
						_tcsncpy(protocols[protocolPos - 1].params[paramPos - 1].name, pos + 1, startPos - pos - 1);

					}else{
						protocols[protocolPos - 1].params[paramPos - 1].length = 0;
						_tcscpy(protocols[protocolPos - 1].params[paramPos - 1].name, pos + 1);
					}
				}
			}
		}
	}

	return TRUE;
}
