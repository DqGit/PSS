#ifndef _BASECONSOLEHANDLE_H
#define _BASECONSOLEHANDLE_H

#include "BuffPacketManager.h"
#include "PacketConsoleParse.h"
#include "MessageBlockManager.h"

//���Ҫ���͵�Console����
bool Console_Common_SendMessage_Data_Check(uint32 u4ConnectID, IBuffPacket* pBuffPacket, uint8 u1OutputType, ACE_Message_Block*& pMbData);

#endif
