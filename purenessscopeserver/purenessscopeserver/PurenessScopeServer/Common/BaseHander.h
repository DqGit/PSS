#ifndef _BASEREACTORHANDLER_H
#define _BASEREACTORHANDLER_H

#include "define.h"
#include "MessageBlockManager.h"
#include "LoadPacketParse.h"
#include "PacketParsePool.h"

//���е�handler�õ��Ĺ�������
//add by freeeyes

//�����ڴ�
void Recovery_Message(bool blDelete, char*& pMessage);

//udp�����������ݰ��ϳɺ���
bool Udp_Common_Send_Message(uint32 u4PacketParseInfoID, ACE_INET_Addr& AddrRemote, char*& pMessage, uint32 u4Len, const char* szIP, int nPort, bool blHead, uint16 u2CommandID, bool blDlete, ACE_Message_Block*& pMbData);

#endif
