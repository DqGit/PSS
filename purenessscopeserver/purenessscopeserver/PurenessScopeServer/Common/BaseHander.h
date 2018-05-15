#ifndef _BASEREACTORHANDLER_H
#define _BASEREACTORHANDLER_H

#include "define.h"
#include "MessageBlockManager.h"
#include "LoadPacketParse.h"
#include "PacketParsePool.h"
#include "MakePacket.h"

//���е�handler�õ��Ĺ�������
//add by freeeyes

//�����ڴ�
void Recovery_Message(bool blDelete, char*& pMessage);

//udp�����������ݰ��ϳɺ���
bool Udp_Common_Send_Message(uint32 u4PacketParseInfoID, ACE_INET_Addr& AddrRemote, char*& pMessage, uint32 u4Len, const char* szIP, int nPort, bool blHead, uint16 u2CommandID, bool blDlete, ACE_Message_Block*& pMbData);

//udp������Ϣͷ����
bool Udp_Common_Recv_Head(ACE_Message_Block* pMBHead, CPacketParse* pPacketParse, uint32 u4PacketParseInfoID, uint32 u4Len);

//udp������Ϣ�庯��
bool Udp_Common_Recv_Body(ACE_Message_Block* pMBBody, CPacketParse* pPacketParse, uint32 u4PacketParseInfoID);

//udp����Ϣ����
bool Udp_Common_Recv_Stream(ACE_Message_Block* pMbData, CPacketParse* pPacketParse, uint32 u4PacketParseInfoID);

//�ύudp���ݵ������߳�
bool Udp_Common_Send_WorkThread(CPacketParse*& pPacketParse, ACE_INET_Addr addrRemote, ACE_INET_Addr addrLocal, ACE_Time_Value& tvCheck);

//�������ݻ���
void Recovery_Common_BuffPacket(bool blDelete, IBuffPacket* pBuffPacket);

//������Ϣ���ع����߳�
void Tcp_Common_Send_Message_Error(bool blDelete, IBuffPacket* pBuffPacket);

//TCP����Ϣ����
uint8 Tcp_Common_Recv_Stream(uint32 u4ConnectID, ACE_Message_Block* pMbData, CPacketParse* pPacketParse, uint32 u4PacketParseInfoID);

//�����ݷ����빤���߳���Ϣ����
void Send_MakePacket_Queue(uint32 u4ConnectID, uint32 u4PacketParseID, CPacketParse* m_pPacketParse, uint8 u1Option, ACE_INET_Addr& addrRemote, const char* pLocalIP, uint32 u4LocalPort);

#endif
