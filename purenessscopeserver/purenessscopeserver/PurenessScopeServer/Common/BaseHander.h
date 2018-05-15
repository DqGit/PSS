#ifndef _BASEREACTORHANDLER_H
#define _BASEREACTORHANDLER_H

#include "ace/Task_T.h"

#include "define.h"
#include "SendMessage.h"
#include "MessageBlockManager.h"
#include "LoadPacketParse.h"
#include "PacketParsePool.h"
#include "MakePacket.h"
#include "FileTest.h"

//���е�handler�õ��Ĺ�������
//add by freeeyes

//�����ڴ�
void Recovery_Message(bool blDelete, char*& pMessage);

//udp�����������ݰ��ϳɺ���
bool Udp_Common_Send_Message(uint32 u4PacketParseInfoID, ACE_INET_Addr& AddrRemote, char*& pMessage, uint32 u4Len,
                             const char* szIP, int nPort, bool blHead, uint16 u2CommandID, bool blDlete, ACE_Message_Block*& pMbData);

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
void Send_MakePacket_Queue(uint32 u4ConnectID, uint32 u4PacketParseID, CPacketParse* m_pPacketParse, uint8 u1Option,
                           ACE_INET_Addr& addrRemote, const char* pLocalIP, uint32 u4LocalPort);

//�ļ�ѹ�������빤���߳�
bool Tcp_Common_File_Message(uint32 u4ConnectID, IFileTestManager* pFileTest, bool blDelete, IBuffPacket*& pBuffPacket,
                             char* pPacketDebugData, uint32 u4PacketDebugSize, ACE_INET_Addr addrRemote, const char* pConnectName);

//���ص�ǰHandler����Ϣ
_ClientConnectInfo Tcp_Common_ClientInfo(uint32 u4ConnectID, ACE_INET_Addr addrRemote, uint32 u4AllRecvCount,
        uint32 u4AllSendCount, uint32 u4AllRecvSize, uint32 u4AllSendSize,
        ACE_Time_Value atvConnect, uint32 u4RecvQueueCount, uint64 u8RecvQueueTimeCost, uint64 u8SendQueueTimeCost);

//����������뷢�ͻ�����
bool Tcp_Common_Send_Input_To_Cache(uint32 u4ConnectID, uint32 u4PacketParseInfoID, uint32 u4SendMaxBuffSize,
                                    ACE_Message_Block* pBlockMessage, uint8 u1SendType, uint32& u4PacketSize,
                                    uint16 u2CommandID, bool blDelete, IBuffPacket*& pBuffPacket);

//��װ��������
bool Tcp_Common_Make_Send_Packet(uint32 u4ConnectID, uint8 u1SendType, uint32 u4PacketParseInfoID,
                                 uint32 u4SendMaxBuffSize, IBuffPacket*& pBuffPacket, uint16 u2CommandID,
                                 bool blDelete, ACE_Message_Block* pBlockMessage);

//����Manager��Ϣ���йر���Ϣ
bool Tcp_Common_CloseConnect_By_Queue(uint32 u4ConnectID, CSendMessagePool& objSendMessagePool,
                                      uint32 u4SendQueuePutTime, ACE_Task<ACE_MT_SYNCH>* pTask);

//����Manager��Ϣ���з���������Ϣ
bool Tcp_Common_Manager_Post_Message(uint32 u4ConnectID, IBuffPacket* pBuffPacket, uint8 u1SendType,
                                     uint16 u2CommandID, uint8 u1SendState, bool blDelete, int nMessageID,
                                     CSendMessagePool& objSendMessagePool, uint16 u2SendQueueMax,
                                     uint32 u4SendQueuePutTime, ACE_Task<ACE_MT_SYNCH>* pTask);

//��ʱ�����ͳ����־
void Tcp_Common_Manager_Timeout_CheckInfo(int nActiveConnectCount);

#endif
