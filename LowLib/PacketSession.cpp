#include "stdafx.h"
#include "Constant.h"
#include "Crypt.h"
#include "CriticalSection.h"
#include "MultiThreadSync.h"
#include "NetworkInfo.h"
#include "PacketInfo.h"
#include "CircularQueue.h"
#include "NetworkSession.h"
#include "PacketSession.h"

PacketSession::PacketSession(VOID){
	memset(mPacketBuffer, 0, sizeof(mPacketBuffer));
	mRemainLength = 0;
	mCurrentPacketNumber = 0;
	mLastReadPacketNumber = 0;
}

PacketSession::~PacketSession(VOID){
}

BOOL PacketSession::begin(VOID){
	ThreadSync sync;

	memset(mPacketBuffer, 0, sizeof(mPacketBuffer));
	mRemainLength = 0;
	mCurrentPacketNumber = 0;
	mLastReadPacketNumber = 0;

	if(!mWriteQueue.begin()){
		return FALSE;
	}

	return NetworkSession::begin();
}

BOOL PacketSession::end(VOID){
	ThreadSync sync;

	mLastReadPacketInfoVectorForUDP.clear();

	if(!mWriteQueue.end()){
		return FALSE;
	}
	
	return NetworkSession::end();
}

BOOL PacketSession::readPacketForIOCP(DWORD readLength){
	ThreadSync sync;

	if(!NetworkSession::readForIOCP(mPacketBuffer + mRemainLength, readLength)){
		return FALSE;
	}

	mRemainLength += readLength;

	return TRUE;
}

BOOL PacketSession::readPacketForEventSelect(VOID){
	ThreadSync sync;

	DWORD readLength = 0;

	if(!NetworkSession::readForEventSelect(mPacketBuffer + mRemainLength, readLength)){
		return FALSE;
	}

	mRemainLength += readLength;

	return TRUE;
}

BOOL PacketSession::parsePacket(DWORD& protocol, BYTE* data, DWORD& dataLength){
	ThreadSync sync;

	if(!data){
		return FALSE;
	}

	if(mRemainLength < sizeof(DWORD)/*length*/){
		return FALSE;
	}

	DWORD tLength = 0;
	memcpy(&tLength, mPacketBuffer, sizeof(DWORD));
	if(tLength <= 0 || tLength > MAX_BUFFER_LENGTH){ // Invalid Packet
		mRemainLength = 0;
		return FALSE;
	}

	if(tLength <= mRemainLength){
		// ��Ŷ���� length�� �� ������ �κ��� ��ȣȭ
		Crypt::decrypt(mPacketBuffer + sizeof(DWORD)/*length*/, mPacketBuffer + sizeof(DWORD)/*length*/, tLength - sizeof(DWORD)/*length*/);

		DWORD tPacketNumber = 0;
		DWORD tProtocol = 0;

		memcpy(&tPacketNumber, mPacketBuffer + sizeof(DWORD)/*length*/, sizeof(DWORD));
		memcpy(&tProtocol, mPacketBuffer + sizeof(DWORD)/*length*/ + sizeof(DWORD)/*packetNumber*/, sizeof(DWORD));

		// �Ľ̿Ϸ�� ��Ŷ ������ �Ķ���Ϳ� ����
		protocol = tProtocol;
		dataLength = tLength - sizeof(DWORD)/*length*/ - sizeof(DWORD)/*packetNumber*/ - sizeof(DWORD)/*protocol*/;
		memcpy(data, mPacketBuffer + sizeof(DWORD)/*length*/ + sizeof(DWORD)/*packetNumber*/ + sizeof(DWORD)/*protocol*/, dataLength);

		// ���� ���� ����
		mRemainLength -= tLength;

		// ���� ��Ŷ�� ������ ���, mPacketBuffer�� ���� ��Ŷ�� mPacketBuffer�� ó�� ��ġ�� �̵�
		if(mRemainLength > 0){
			memmove(mPacketBuffer, mPacketBuffer + tLength, mRemainLength);

		// ���� ��Ŷ�� �������� ���� ���, mPacketBuffer�� �ʱ�ȭ
		}else{
			mRemainLength = 0;
			memset(mPacketBuffer, 0, sizeof(mPacketBuffer));
		}

		// ������ ���� ��Ŷ ��ȣ ����
		if(tPacketNumber <= mLastReadPacketNumber){
			return FALSE;
		}else{
			mLastReadPacketNumber = tPacketNumber;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL PacketSession::writePacket(DWORD protocol, const BYTE* data, DWORD dataLength){
	ThreadSync sync;

	if(!data){
		return FALSE;
	}

	DWORD tLength = sizeof(DWORD)/*length*/ + sizeof(DWORD)/*packetNumber*/ + sizeof(DWORD)/*protocol*/ + dataLength/*data*/;
	if(tLength > MAX_BUFFER_LENGTH){
		return FALSE;
	}

	mCurrentPacketNumber++;

	PACKET_INFO packet;
	packet.set(tLength, mCurrentPacketNumber, protocol, data);

	// ��Ŷ���� length�� �� ������ �κ��� ��ȣȭ
	Crypt::encrypt((BYTE*)&packet + sizeof(DWORD)/*length*/, (BYTE*)&packet + sizeof(DWORD)/*length*/, tLength - sizeof(DWORD)/*length*/);

	// write�� ����Ǵ� ���� packet�� �޸�(Queue)�� ������ �д�. (writeComplete�� Queue���� ����)
	PACKET_INFO* packetPtr = mWriteQueue.enq(packet);

	return NetworkSession::write((BYTE*)packetPtr, tLength);
} 

BOOL PacketSession::readFromPacketForIOCP(LPSTR remoteAddress, USHORT& remotePort, DWORD readLength){
	ThreadSync sync;

	if (!NetworkSession::readFromForIOCP(remoteAddress, remotePort, mPacketBuffer + mRemainLength, readLength)){
		return FALSE;
	}

	mRemainLength += readLength;

	return TRUE;
}

BOOL PacketSession::readFromPacketForEventSelect(LPSTR remoteAddress, USHORT& remotePort){
	ThreadSync sync;

	DWORD readLength = 0;

	if (!NetworkSession::readFromForEventSelect(remoteAddress, remotePort, mPacketBuffer + mRemainLength, readLength)){
		return FALSE;
	}

	mRemainLength += readLength;

	return TRUE;
}

BOOL PacketSession::parseFromPacket(LPSTR remoteAddress, USHORT remotePort, DWORD& protocol, BYTE* data, DWORD& dataLength){
	ThreadSync sync;

	if(!data){
		return FALSE;
	}

	if(mRemainLength < sizeof(DWORD)/*length*/){
		return FALSE;
	}

	DWORD tLength = 0;
	memcpy(&tLength, mPacketBuffer, sizeof(DWORD));
	if(tLength <= 0 || tLength > MAX_BUFFER_LENGTH){ // Invalid Packet
		mRemainLength = 0;
		return FALSE;
	}

	if(tLength <= mRemainLength){
		// ��Ŷ���� length�� �� ������ �κ��� ��ȣȭ
		Crypt::decrypt(mPacketBuffer + sizeof(DWORD)/*length*/, mPacketBuffer + sizeof(DWORD)/*length*/, tLength - sizeof(DWORD)/*length*/);

		DWORD tPacketNumber = 0;
		DWORD tProtocol = 0;

		memcpy(&tPacketNumber, mPacketBuffer + sizeof(DWORD)/*length*/, sizeof(DWORD));
		memcpy(&tProtocol, mPacketBuffer + sizeof(DWORD)/*length*/ +sizeof(DWORD)/*packetNumber*/, sizeof(DWORD));

		// �Ľ̿Ϸ�� ��Ŷ ������ �Ķ���Ϳ� ����
		protocol = tProtocol;
		dataLength = tLength - sizeof(DWORD)/*length*/ -sizeof(DWORD)/*packetNumber*/ -sizeof(DWORD)/*protocol*/;
		memcpy(data, mPacketBuffer + sizeof(DWORD)/*length*/ +sizeof(DWORD)/*packetNumber*/ +sizeof(DWORD)/*protocol*/, dataLength);

		// ���� ���� ����
		mRemainLength -= tLength;

		// ���� ��Ŷ�� ������ ���, mPacketBuffer�� ���� ��Ŷ�� mPacketBuffer�� ó�� ��ġ�� �̵�
		if(mRemainLength > 0){
			memmove(mPacketBuffer, mPacketBuffer + tLength, mRemainLength);

		// ���� ��Ŷ�� �������� ���� ���, mPacketBuffer�� �ʱ�ȭ
		}else{
			mRemainLength = 0;
			memset(mPacketBuffer, 0, sizeof(mPacketBuffer));
		}

		// ������ ���� ��Ŷ ��ȣ ����
		for(DWORD i = 0; i < mLastReadPacketInfoVectorForUDP.size(); i++){
			READ_PACKET_INFO readPacket = mLastReadPacketInfoVectorForUDP[i];
			if(strcmp(readPacket.remoteAddress, remoteAddress) == 0 && readPacket.remotePort == remotePort){
				if(readPacket.packetNumber < tPacketNumber){
					mLastReadPacketInfoVectorForUDP[i].packetNumber = tPacketNumber;
					return TRUE;
				}else{
					return FALSE;
				}
			}
		}

		// UDP�� ��� ���� ��Ŷ ������ ���Ϳ� ������ �д�.
		READ_PACKET_INFO readPacket;
		readPacket.set(remoteAddress, remotePort, tPacketNumber);
		mLastReadPacketInfoVectorForUDP.push_back(readPacket);

		return TRUE;
	}

	return FALSE;
}

BOOL PacketSession::writeToPacket(LPCSTR remoteAddress, USHORT remotePort, DWORD protocol, const BYTE* data, DWORD dataLength){
	ThreadSync sync;

	if(!data){
		return FALSE;
	}

	DWORD tLength = sizeof(DWORD)/*length*/ + sizeof(DWORD)/*packetNumber*/ + sizeof(DWORD)/*protocol*/ + dataLength/*data*/;
	if(tLength > MAX_BUFFER_LENGTH){
		return FALSE;
	}

	mCurrentPacketNumber++;

	PACKET_INFO packet;
	packet.set(tLength, mCurrentPacketNumber, protocol, data);

	// ��Ŷ���� length�� �� ������ �κ��� ��ȣȭ
	Crypt::encrypt((BYTE*)&packet + sizeof(DWORD)/*length*/, (BYTE*)&packet + sizeof(DWORD)/*length*/, tLength - sizeof(DWORD)/*length*/);

	// write�� ����Ǵ� ���� packet�� �޸�(Queue)�� ������ �д�. (writeComplete�� Queue���� ����)
	PACKET_INFO* packetPtr = mWriteQueue.enq(packet);

	return NetworkSession::writeTo(remoteAddress, remotePort, (BYTE*)packetPtr, tLength);
}

BOOL PacketSession::writeComplete(VOID){
	ThreadSync sync;

	return mWriteQueue.deq();
}

BOOL PacketSession::resetUDP(VOID){
	ThreadSync sync;

	mLastReadPacketInfoVectorForUDP.clear();

	return TRUE;
}
