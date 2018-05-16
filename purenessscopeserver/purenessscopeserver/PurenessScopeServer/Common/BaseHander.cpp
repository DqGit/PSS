#include "BaseHander.h"

void Recovery_Message(bool blDelete, char*& pMessage)
{
    if (true == blDelete)
    {
        SAFE_DELETE_ARRAY(pMessage);
    }
}

bool Udp_Common_Send_Message(_Send_Message_Param obj_Send_Message_Param, ACE_INET_Addr& AddrRemote, char*& pMessage, ACE_Message_Block*& pMbData)
{
    int nErr = AddrRemote.set(obj_Send_Message_Param.m_nPort, obj_Send_Message_Param.m_pIP);

    if (nErr != 0)
    {
        OUR_DEBUG((LM_INFO, "[Udp_Common_Send_Message]set_address error[%d].\n", errno));
        Recovery_Message(obj_Send_Message_Param.m_blDlete, pMessage);
        return false;
    }

    if (obj_Send_Message_Param.m_blHead == true)
    {
        //�����Ҫƴ�Ӱ�ͷ����ƴ�Ӱ�ͷ
        uint32 u4SendLength = App_PacketParseLoader::instance()->GetPacketParseInfo(obj_Send_Message_Param.m_u4PacketParseInfoID)->Make_Send_Packet_Length(0, obj_Send_Message_Param.m_u4Len, obj_Send_Message_Param.m_u2CommandID);
        pMbData = App_MessageBlockManager::instance()->Create(u4SendLength);

        if (NULL == pMbData)
        {
            OUR_DEBUG((LM_INFO, "Udp_Common_Send_Message]pMbData is NULL.\n"));
            Recovery_Message(obj_Send_Message_Param.m_blDlete, pMessage);
            return false;
        }

        if (false == App_PacketParseLoader::instance()->GetPacketParseInfo(obj_Send_Message_Param.m_u4PacketParseInfoID)->Make_Send_Packet(0, pMessage, obj_Send_Message_Param.m_u4Len, pMbData, obj_Send_Message_Param.m_u2CommandID))
        {
            OUR_DEBUG((LM_INFO, "Udp_Common_Send_Message]Make_Send_Packet is false.\n"));
            Recovery_Message(obj_Send_Message_Param.m_blDlete, pMessage);
            return false;
        }

        Recovery_Message(obj_Send_Message_Param.m_blDlete, pMessage);
        return true;
    }
    else
    {
        //����Ҫƴ�Ӱ�ͷ��ֱ�ӷ���
        pMbData = App_MessageBlockManager::instance()->Create(obj_Send_Message_Param.m_u4Len);

        if (NULL == pMbData)
        {
            OUR_DEBUG((LM_INFO, "Udp_Common_Send_Message]pMbData is NULL.\n"));
            Recovery_Message(obj_Send_Message_Param.m_blDlete, pMessage);
            return false;
        }

        //�����ݿ�����ACE_Message_Block������
        memcpy_safe(pMessage, obj_Send_Message_Param.m_u4Len, pMbData->wr_ptr(), obj_Send_Message_Param.m_u4Len);
        pMbData->wr_ptr(obj_Send_Message_Param.m_u4Len);

        Recovery_Message(obj_Send_Message_Param.m_blDlete, pMessage);
        return true;
    }
}

bool Udp_Common_Recv_Head(ACE_Message_Block* pMBHead, CPacketParse* pPacketParse, uint32 u4PacketParseInfoID, uint32 u4Len)
{
    _Head_Info obj_Head_Info;
    bool blStateHead = App_PacketParseLoader::instance()->GetPacketParseInfo(u4PacketParseInfoID)->Parse_Packet_Head_Info(0, pMBHead, App_MessageBlockManager::instance(), &obj_Head_Info);

    if (false == blStateHead)
    {
        return false;
    }
    else
    {
        pPacketParse->SetPacket_IsHandleHead(false);
        pPacketParse->SetPacket_Head_Curr_Length(obj_Head_Info.m_u4HeadCurrLen);
        pPacketParse->SetPacket_Body_Src_Length(obj_Head_Info.m_u4BodySrcLen);
        pPacketParse->SetPacket_CommandID(obj_Head_Info.m_u2PacketCommandID);
        pPacketParse->SetPacket_Head_Message(pMBHead);
    }

    if (u4Len != pPacketParse->GetPacketHeadSrcLen() + pPacketParse->GetPacketBodySrcLen())
    {
        return false;
    }

    return true;
}

bool Udp_Common_Recv_Body(ACE_Message_Block* pMBBody, CPacketParse* pPacketParse, uint32 u4PacketParseInfoID)
{
    _Body_Info obj_Body_Info;
    bool blStateBody = App_PacketParseLoader::instance()->GetPacketParseInfo(u4PacketParseInfoID)->Parse_Packet_Body_Info(0, pMBBody, App_MessageBlockManager::instance(), &obj_Body_Info);

    if (false == blStateBody)
    {
        return false;
    }
    else
    {
        pPacketParse->SetPacket_Body_Message(pMBBody);
        pPacketParse->SetPacket_Body_Curr_Length(obj_Body_Info.m_u4BodyCurrLen);
    }

    return true;
}

bool Udp_Common_Recv_Stream(ACE_Message_Block* pMbData, CPacketParse* pPacketParse, uint32 u4PacketParseInfoID)
{
    //������������
    _Packet_Info obj_Packet_Info;

    if (PACKET_GET_ENOUGTH == App_PacketParseLoader::instance()->GetPacketParseInfo(u4PacketParseInfoID)->Parse_Packet_Stream(0, pMbData, App_MessageBlockManager::instance(), &obj_Packet_Info))
    {
        pPacketParse->SetPacket_Head_Message(obj_Packet_Info.m_pmbHead);
        pPacketParse->SetPacket_Body_Message(obj_Packet_Info.m_pmbBody);
        pPacketParse->SetPacket_CommandID(obj_Packet_Info.m_u2PacketCommandID);
        pPacketParse->SetPacket_Head_Src_Length(obj_Packet_Info.m_u4HeadSrcLen);
        pPacketParse->SetPacket_Head_Curr_Length(obj_Packet_Info.m_u4HeadCurrLen);
        pPacketParse->SetPacket_Head_Src_Length(obj_Packet_Info.m_u4BodySrcLen);
        pPacketParse->SetPacket_Body_Curr_Length(obj_Packet_Info.m_u4BodyCurrLen);
    }
    else
    {
        OUR_DEBUG((LM_ERROR, "[Udp_Common_Recv_Stream]m_pPacketParse GetPacketStream is error.\n"));
        App_PacketParsePool::instance()->Delete(pPacketParse);
        return false;
    }

    return true;
}

bool Udp_Common_Send_WorkThread(CPacketParse*& pPacketParse, ACE_INET_Addr addrRemote, ACE_INET_Addr addrLocal, ACE_Time_Value& tvCheck)
{
    //��֯���ݰ�
    _MakePacket objMakePacket;
    objMakePacket.m_u4ConnectID = UDP_HANDER_ID;
    objMakePacket.m_pPacketParse = pPacketParse;
    objMakePacket.m_PacketType = PACKET_UDP;
    objMakePacket.m_AddrRemote = addrRemote;
    objMakePacket.m_AddrListen = addrLocal;
    objMakePacket.m_u1Option = PACKET_PARSE;

    //UDP��Ϊ�����������ӵ�
    if (false == App_MakePacket::instance()->PutMessageBlock(&objMakePacket, tvCheck))
    {
        OUR_DEBUG((LM_ERROR, "[Udp_Common_Send_WorkThread]PutMessageBlock is error.\n"));
        App_PacketParsePool::instance()->Delete(pPacketParse);
        return false;
    }

    return true;
}

void Recovery_Common_BuffPacket(bool blDelete, IBuffPacket* pBuffPacket)
{
    if (true == blDelete)
    {
        App_BuffPacketManager::instance()->Delete(pBuffPacket);
    }
}

void Tcp_Common_Send_Message_Error(bool blDelete, IBuffPacket* pBuffPacket)
{
    ACE_Message_Block* pSendMessage = App_MessageBlockManager::instance()->Create(pBuffPacket->GetPacketLen());
    memcpy_safe((char*)pBuffPacket->GetData(), pBuffPacket->GetPacketLen(), (char*)pSendMessage->wr_ptr(), pBuffPacket->GetPacketLen());
    pSendMessage->wr_ptr(pBuffPacket->GetPacketLen());
    ACE_Time_Value tvNow = ACE_OS::gettimeofday();

    if (false == App_MakePacket::instance()->PutSendErrorMessage(0, pSendMessage, tvNow))
    {
        OUR_DEBUG((LM_INFO, "Tcp_Common_Send_Message_Error]PutSendErrorMessage error.\n"));
    }

    Recovery_Common_BuffPacket(blDelete, pBuffPacket);
}

uint8 Tcp_Common_Recv_Stream(uint32 u4ConnectID, ACE_Message_Block* pMbData, CPacketParse* pPacketParse, uint32 u4PacketParseInfoID)
{
    _Packet_Info obj_Packet_Info;
    uint8 n1Ret = App_PacketParseLoader::instance()->GetPacketParseInfo(u4PacketParseInfoID)->Parse_Packet_Stream(u4ConnectID, pMbData, dynamic_cast<IMessageBlockManager*>(App_MessageBlockManager::instance()), &obj_Packet_Info);

    if (PACKET_GET_ENOUGTH == n1Ret)
    {
        pPacketParse->SetPacket_Head_Message(obj_Packet_Info.m_pmbHead);
        pPacketParse->SetPacket_Body_Message(obj_Packet_Info.m_pmbBody);
        pPacketParse->SetPacket_CommandID(obj_Packet_Info.m_u2PacketCommandID);
        pPacketParse->SetPacket_Head_Src_Length(obj_Packet_Info.m_u4HeadSrcLen);
        pPacketParse->SetPacket_Head_Curr_Length(obj_Packet_Info.m_u4HeadCurrLen);
        pPacketParse->SetPacket_Body_Src_Length(obj_Packet_Info.m_u4BodySrcLen);
        pPacketParse->SetPacket_Body_Curr_Length(obj_Packet_Info.m_u4BodyCurrLen);
    }

    return n1Ret;
}

void Send_MakePacket_Queue(uint32 u4ConnectID, uint32 u4PacketParseID, CPacketParse* m_pPacketParse, uint8 u1Option, ACE_INET_Addr& addrRemote, const char* pLocalIP, uint32 u4LocalPort)
{
    //��Ҫ�ص����ͳɹ���ִ
    _MakePacket objMakePacket;

    objMakePacket.m_u4ConnectID = u4ConnectID;
    objMakePacket.m_pPacketParse = m_pPacketParse;
    objMakePacket.m_u1Option = u1Option;
    objMakePacket.m_AddrRemote = addrRemote;
    objMakePacket.m_u4PacketParseID = u4PacketParseID;

    if (ACE_OS::strcmp("INADDR_ANY", pLocalIP) == 0)
    {
        objMakePacket.m_AddrListen.set(u4LocalPort);
    }
    else
    {
        objMakePacket.m_AddrListen.set(u4LocalPort, pLocalIP);
    }

    //���Ϳͻ������ӶϿ���Ϣ��
    ACE_Time_Value tvNow = ACE_OS::gettimeofday();

    if (false == App_MakePacket::instance()->PutMessageBlock(&objMakePacket, tvNow))
    {
        OUR_DEBUG((LM_ERROR, "[Send_MakePacket_Queue] ConnectID = %d, PACKET_CONNECT is error.\n", u4ConnectID));
    }
}

bool Tcp_Common_File_Message(_File_Message_Param obj_File_Message_Param, IBuffPacket*& pBuffPacket, const char* pConnectName)
{
    char szLog[10] = { '\0' };
    uint32 u4DebugSize = 0;
    bool blblMore = false;

    if (pBuffPacket->GetPacketLen() >= obj_File_Message_Param.m_u4PacketDebugSize)
    {
        u4DebugSize = obj_File_Message_Param.m_u4PacketDebugSize - 1;
        blblMore = true;
    }
    else
    {
        u4DebugSize = (int)pBuffPacket->GetPacketLen();
    }

    char* pData = (char*)pBuffPacket->GetData();

    for (uint32 i = 0; i < u4DebugSize; i++)
    {
        sprintf_safe(szLog, 10, "0x%02X ", (unsigned char)pData[i]);
        sprintf_safe(obj_File_Message_Param.m_pPacketDebugData + 5 * i, MAX_BUFF_1024 - 5 * i, "0x%02X ", (unsigned char)pData[i]);
    }

    obj_File_Message_Param.m_pPacketDebugData[5 * u4DebugSize] = '\0';

    if (blblMore == true)
    {
        AppLogManager::instance()->WriteLog(LOG_SYSTEM_DEBUG_CLIENTSEND, "[(%s)%s:%d]%s.(���ݰ�����)", pConnectName, obj_File_Message_Param.m_addrRemote.get_host_addr(), obj_File_Message_Param.m_addrRemote.get_port_number(), obj_File_Message_Param.m_pPacketDebugData);
    }
    else
    {
        AppLogManager::instance()->WriteLog(LOG_SYSTEM_DEBUG_CLIENTSEND, "[(%s)%s:%d]%s.", pConnectName, obj_File_Message_Param.m_addrRemote.get_host_addr(), obj_File_Message_Param.m_addrRemote.get_port_number(), obj_File_Message_Param.m_pPacketDebugData);
    }

    //�ص������ļ�����ӿ�
    if (NULL != obj_File_Message_Param.m_pFileTest)
    {
        obj_File_Message_Param.m_pFileTest->HandlerServerResponse(obj_File_Message_Param.m_u4ConnectID);
    }

    //��������Ķ���
    Recovery_Common_BuffPacket(obj_File_Message_Param.m_blDelete, pBuffPacket);

    return true;
}

_ClientConnectInfo Tcp_Common_ClientInfo(_ClientConnectInfo_Param obj_ClientConnectInfo_Param)
{
    _ClientConnectInfo ClientConnectInfo;

    ClientConnectInfo.m_blValid = true;
    ClientConnectInfo.m_u4ConnectID = obj_ClientConnectInfo_Param.m_u4ConnectID;
    ClientConnectInfo.m_addrRemote = obj_ClientConnectInfo_Param.m_addrRemote;
    ClientConnectInfo.m_u4RecvCount = obj_ClientConnectInfo_Param.m_u4AllRecvCount;
    ClientConnectInfo.m_u4SendCount = obj_ClientConnectInfo_Param.m_u4AllSendCount;
    ClientConnectInfo.m_u4AllRecvSize = obj_ClientConnectInfo_Param.m_u4AllRecvSize;
    ClientConnectInfo.m_u4AllSendSize = obj_ClientConnectInfo_Param.m_u4AllSendSize;
    ClientConnectInfo.m_u4BeginTime = (uint32)obj_ClientConnectInfo_Param.m_atvConnect.sec();
    ClientConnectInfo.m_u4AliveTime = (uint32)(ACE_OS::gettimeofday().sec() - obj_ClientConnectInfo_Param.m_atvConnect.sec());
    ClientConnectInfo.m_u4RecvQueueCount = obj_ClientConnectInfo_Param.m_u4RecvQueueCount;
    ClientConnectInfo.m_u8RecvQueueTimeCost = obj_ClientConnectInfo_Param.m_u8RecvQueueTimeCost;
    ClientConnectInfo.m_u8SendQueueTimeCost = obj_ClientConnectInfo_Param.m_u8SendQueueTimeCost;

    return ClientConnectInfo;
}

bool Tcp_Common_Send_Input_To_Cache(uint32 u4ConnectID, uint32 u4PacketParseInfoID, uint32 u4SendMaxBuffSize, ACE_Message_Block* pBlockMessage, uint8 u1SendType, uint32& u4PacketSize, uint16 u2CommandID, bool blDelete, IBuffPacket*& pBuffPacket)
{
    //���ж�Ҫ���͵����ݳ��ȣ������Ƿ���Է��뻺�壬�����Ƿ��Ѿ�������
    uint32 u4SendPacketSize = 0;

    if (u1SendType == SENDMESSAGE_NOMAL)
    {
        u4SendPacketSize = App_PacketParseLoader::instance()->GetPacketParseInfo(u4PacketParseInfoID)->Make_Send_Packet_Length(u4ConnectID, pBuffPacket->GetPacketLen(), u2CommandID);
    }
    else
    {
        u4SendPacketSize = (uint32)pBlockMessage->length();
    }

    u4PacketSize = u4SendPacketSize;

    if (u4SendPacketSize + (uint32)pBlockMessage->length() >= u4SendMaxBuffSize)
    {
        OUR_DEBUG((LM_DEBUG, "[Tcp_Common_Send_Input_To_Cache] Connectid=[%d] m_pBlockMessage is not enougth.\n", u4ConnectID));
        //������Ӳ������ˣ������ﷵ��ʧ�ܣ��ص���ҵ���߼�ȥ����
        ACE_Message_Block* pSendMessage = App_MessageBlockManager::instance()->Create(pBuffPacket->GetPacketLen());
        memcpy_safe((char*)pBuffPacket->GetData(), pBuffPacket->GetPacketLen(), (char*)pSendMessage->wr_ptr(), pBuffPacket->GetPacketLen());
        pSendMessage->wr_ptr(pBuffPacket->GetPacketLen());
        ACE_Time_Value tvNow = ACE_OS::gettimeofday();
        App_MakePacket::instance()->PutSendErrorMessage(0, pSendMessage, tvNow);

        Recovery_Common_BuffPacket(blDelete, pBuffPacket);

        return false;
    }
    else
    {
        //��ӽ�������
        //SENDMESSAGE_NOMAL����Ҫ��ͷ��ʱ�򣬷��򣬲����ֱ�ӷ���
        if (u1SendType == SENDMESSAGE_NOMAL)
        {
            //������ɷ������ݰ�
            App_PacketParseLoader::instance()->GetPacketParseInfo(u4PacketParseInfoID)->Make_Send_Packet(u4ConnectID, pBuffPacket->GetData(), pBuffPacket->GetPacketLen(), pBlockMessage, u2CommandID);
        }
        else
        {
            //�������SENDMESSAGE_NOMAL����ֱ�����
            memcpy_safe((char*)pBuffPacket->GetData(), pBuffPacket->GetPacketLen(), (char*)pBlockMessage->wr_ptr(), pBuffPacket->GetPacketLen());
            pBlockMessage->wr_ptr(pBuffPacket->GetPacketLen());
        }
    }

    Recovery_Common_BuffPacket(blDelete, pBuffPacket);

    return true;
}

bool Tcp_Common_Make_Send_Packet(uint32 u4ConnectID, uint8 u1SendType, uint32 u4PacketParseInfoID, uint32 u4SendMaxBuffSize, IBuffPacket*& pBuffPacket, uint16 u2CommandID, bool blDelete, ACE_Message_Block* pBlockMessage)
{
    uint32 u4SendPacketSize = 0;

    if (u1SendType == SENDMESSAGE_NOMAL)
    {
        u4SendPacketSize = App_PacketParseLoader::instance()->GetPacketParseInfo(u4PacketParseInfoID)->Make_Send_Packet_Length(u4ConnectID, pBuffPacket->GetPacketLen(), u2CommandID);

        if (u4SendPacketSize >= u4SendMaxBuffSize)
        {
            OUR_DEBUG((LM_DEBUG, "[Tcp_Common_Make_Send_Packet](%d) u4SendPacketSize is more than(%d)(%d).\n", u4ConnectID, u4SendPacketSize, u4SendMaxBuffSize));

            Recovery_Common_BuffPacket(blDelete, pBuffPacket);

            return false;
        }

        App_PacketParseLoader::instance()->GetPacketParseInfo(u4PacketParseInfoID)->Make_Send_Packet(u4ConnectID, pBuffPacket->GetData(), pBuffPacket->GetPacketLen(), pBlockMessage, u2CommandID);
        //����MakePacket�Ѿ��������ݳ��ȣ����������ﲻ��׷��
    }
    else
    {
        u4SendPacketSize = (uint32)pBuffPacket->GetPacketLen();

        if (u4SendPacketSize >= u4SendMaxBuffSize)
        {
            OUR_DEBUG((LM_DEBUG, "[CProConnectHandle::SendMessage](%d) u4SendPacketSize is more than(%d)(%d).\n", u4ConnectID, u4SendPacketSize, u4SendMaxBuffSize));
            //������Ӳ������ˣ������ﷵ��ʧ�ܣ��ص���ҵ���߼�ȥ����
            ACE_Message_Block* pSendMessage = App_MessageBlockManager::instance()->Create(pBuffPacket->GetPacketLen());
            memcpy_safe((char*)pBuffPacket->GetData(), pBuffPacket->GetPacketLen(), (char*)pSendMessage->wr_ptr(), pBuffPacket->GetPacketLen());
            pSendMessage->wr_ptr(pBuffPacket->GetPacketLen());
            ACE_Time_Value tvNow = ACE_OS::gettimeofday();
            App_MakePacket::instance()->PutSendErrorMessage(0, pSendMessage, tvNow);

            Recovery_Common_BuffPacket(blDelete, pBuffPacket);

            return false;
        }

        memcpy_safe((char*)pBuffPacket->GetData(), pBuffPacket->GetPacketLen(), (char*)pBlockMessage->wr_ptr(), pBuffPacket->GetPacketLen());
        pBlockMessage->wr_ptr(pBuffPacket->GetPacketLen());
    }

    return true;
}

bool Tcp_Common_CloseConnect_By_Queue(uint32 u4ConnectID, CSendMessagePool& objSendMessagePool, uint32 u4SendQueuePutTime, ACE_Task<ACE_MT_SYNCH>* pTask)
{
    //���뷢�Ͷ���
    _SendMessage* pSendMessage = objSendMessagePool.Create();

    if (NULL == pSendMessage)
    {
        OUR_DEBUG((LM_ERROR, "[Tcp_Common_CloseConnect_By_Queue] new _SendMessage is error.\n"));
        return false;
    }

    ACE_Message_Block* mb = pSendMessage->GetQueueMessage();

    if (NULL != mb)
    {
        //��װ�ر�����ָ��
        pSendMessage->m_u4ConnectID = u4ConnectID;
        pSendMessage->m_pBuffPacket = NULL;
        pSendMessage->m_nEvents = 0;
        pSendMessage->m_u2CommandID = 0;
        pSendMessage->m_u1SendState = 0;
        pSendMessage->m_blDelete = false;
        pSendMessage->m_nMessageID = 0;
        pSendMessage->m_u1Type = 1;
        pSendMessage->m_tvSend = ACE_OS::gettimeofday();

        //�ж϶����Ƿ����Ѿ����
        int nQueueCount = (int)pTask->msg_queue()->message_count();

        if (nQueueCount >= (int)MAX_MSG_THREADQUEUE)
        {
            OUR_DEBUG((LM_ERROR, "[Tcp_Common_CloseConnect_By_Queue] Queue is Full nQueueCount = [%d].\n", nQueueCount));
            objSendMessagePool.Delete(pSendMessage);
            return false;
        }

        ACE_Time_Value xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, u4SendQueuePutTime);

        if (pTask->putq(mb, &xtime) == -1)
        {
            OUR_DEBUG((LM_ERROR, "[Tcp_Common_CloseConnect_By_Queue] Queue putq  error nQueueCount = [%d] errno = [%d].\n", nQueueCount, errno));
            objSendMessagePool.Delete(pSendMessage);
            return false;
        }
    }
    else
    {
        OUR_DEBUG((LM_ERROR, "[Tcp_Common_CloseConnect_By_Queue] mb new error.\n"));
        objSendMessagePool.Delete(pSendMessage);
        return false;
    }

    return true;
}

bool Tcp_Common_Manager_Post_Message(uint32 u4ConnectID, IBuffPacket* pBuffPacket, uint8 u1SendType, uint16 u2CommandID, uint8 u1SendState, bool blDelete, int nMessageID, CSendMessagePool& objSendMessagePool, uint16 u2SendQueueMax, uint32 u4SendQueuePutTime, ACE_Task<ACE_MT_SYNCH>* pTask)
{
    if (NULL == pBuffPacket)
    {
        OUR_DEBUG((LM_ERROR, "[CProConnectManager::PutMessage] pBuffPacket is NULL.\n"));
        return false;
    }

    //���뷢�Ͷ���
    _SendMessage* pSendMessage = objSendMessagePool.Create();

    if (NULL == pSendMessage)
    {
        OUR_DEBUG((LM_ERROR, "[CProConnectManager::PutMessage] new _SendMessage is error.\n"));

        if (true == blDelete)
        {
            App_BuffPacketManager::instance()->Delete(pBuffPacket);
        }

        return false;
    }

    ACE_Message_Block* mb = pSendMessage->GetQueueMessage();

    if (NULL != mb)
    {
        pSendMessage->m_u4ConnectID = u4ConnectID;
        pSendMessage->m_pBuffPacket = pBuffPacket;
        pSendMessage->m_nEvents     = u1SendType;
        pSendMessage->m_u2CommandID = u2CommandID;
        pSendMessage->m_u1SendState = u1SendState;
        pSendMessage->m_blDelete    = blDelete;
        pSendMessage->m_nMessageID  = nMessageID;
        pSendMessage->m_u1Type      = 0;
        pSendMessage->m_tvSend      = ACE_OS::gettimeofday();

        //�ж϶����Ƿ����Ѿ����
        int nQueueCount = (int)pTask->msg_queue()->message_count();

        if (nQueueCount >= (int)u2SendQueueMax)
        {
            OUR_DEBUG((LM_ERROR, "[CProConnectManager::PutMessage] Queue is Full nQueueCount = [%d].\n", nQueueCount));

            if (true == blDelete)
            {
                App_BuffPacketManager::instance()->Delete(pBuffPacket);
            }

            objSendMessagePool.Delete(pSendMessage);
            return false;
        }

        ACE_Time_Value xtime = ACE_OS::gettimeofday() + ACE_Time_Value(0, u4SendQueuePutTime);

        if (pTask->putq(mb, &xtime) == -1)
        {
            OUR_DEBUG((LM_ERROR, "[CProConnectManager::PutMessage] Queue putq  error nQueueCount = [%d] errno = [%d].\n", nQueueCount, errno));

            if (true == blDelete)
            {
                App_BuffPacketManager::instance()->Delete(pBuffPacket);
            }

            objSendMessagePool.Delete(pSendMessage);
            return false;
        }
    }
    else
    {
        OUR_DEBUG((LM_ERROR, "[CMessageService::PutMessage] mb new error.\n"));

        if (true == blDelete)
        {
            App_BuffPacketManager::instance()->Delete(pBuffPacket);
        }

        objSendMessagePool.Delete(pSendMessage);
        return false;
    }

    return true;
}

void Tcp_Common_Manager_Timeout_CheckInfo(int nActiveConnectCount)
{
    //������������Ƿ�Խ��ط�ֵ
    if (App_MainConfig::instance()->GetConnectAlert()->m_u4ConnectAlert > 0
        && nActiveConnectCount > (int)App_MainConfig::instance()->GetConnectAlert()->m_u4ConnectAlert)
    {
        AppLogManager::instance()->WriteToMail(LOG_SYSTEM_CONNECT,
                                               App_MainConfig::instance()->GetConnectAlert()->m_u4MailID,
                                               (char*)"Alert",
                                               "[Tcp_Common_Manager_Timeout_CheckInfo]active ConnectCount is more than limit(%d > %d).",
                                               nActiveConnectCount,
                                               App_MainConfig::instance()->GetConnectAlert()->m_u4ConnectAlert);
    }

    //��ⵥλʱ���������Ƿ�Խ��ֵ
    int nCheckRet = App_ConnectAccount::instance()->CheckConnectCount();

    if (nCheckRet == 1)
    {
        AppLogManager::instance()->WriteToMail(LOG_SYSTEM_CONNECT,
                                               App_MainConfig::instance()->GetConnectAlert()->m_u4MailID,
                                               "Alert",
                                               "[Tcp_Common_Manager_Timeout_CheckInfo]CheckConnectCount is more than limit(%d > %d).",
                                               App_ConnectAccount::instance()->GetCurrConnect(),
                                               App_ConnectAccount::instance()->GetConnectMax());
    }
    else if (nCheckRet == 2)
    {
        AppLogManager::instance()->WriteToMail(LOG_SYSTEM_CONNECT,
                                               App_MainConfig::instance()->GetConnectAlert()->m_u4MailID,
                                               "Alert",
                                               "[Tcp_Common_Manager_Timeout_CheckInfo]CheckConnectCount is little than limit(%d < %d).",
                                               App_ConnectAccount::instance()->GetCurrConnect(),
                                               App_ConnectAccount::instance()->Get4ConnectMin());
    }

    //��ⵥλʱ�����ӶϿ����Ƿ�Խ��ֵ
    nCheckRet = App_ConnectAccount::instance()->CheckDisConnectCount();

    if (nCheckRet == 1)
    {
        AppLogManager::instance()->WriteToMail(LOG_SYSTEM_CONNECT,
                                               App_MainConfig::instance()->GetConnectAlert()->m_u4MailID,
                                               "Alert",
                                               "[Tcp_Common_Manager_Timeout_CheckInfo]CheckDisConnectCount is more than limit(%d > %d).",
                                               App_ConnectAccount::instance()->GetCurrConnect(),
                                               App_ConnectAccount::instance()->GetDisConnectMax());
    }
    else if (nCheckRet == 2)
    {
        AppLogManager::instance()->WriteToMail(LOG_SYSTEM_CONNECT,
                                               App_MainConfig::instance()->GetConnectAlert()->m_u4MailID,
                                               "Alert",
                                               "[Tcp_Common_Manager_Timeout_CheckInfo]CheckDisConnectCount is little than limit(%d < %d).",
                                               App_ConnectAccount::instance()->GetCurrConnect(),
                                               App_ConnectAccount::instance()->GetDisConnectMin());
    }

}

_ClientNameInfo Tcp_Common_ClientNameInfo(uint32 u4ConnectID, const char* pConnectName, const char* pClientIP, int nClientPort, bool IsLog)
{
    _ClientNameInfo ClientNameInfo;
    ClientNameInfo.m_nConnectID = (int)u4ConnectID;
    sprintf_safe(ClientNameInfo.m_szName, MAX_BUFF_100, "%s", pConnectName);
    sprintf_safe(ClientNameInfo.m_szClientIP, MAX_BUFF_50, "%s", pClientIP);
    ClientNameInfo.m_nPort = nClientPort;

    if (IsLog == true)
    {
        ClientNameInfo.m_nLog = 1;
    }
    else
    {
        ClientNameInfo.m_nLog = 0;
    }

    return ClientNameInfo;
}

void Tcp_Common_Manager_Init(uint16 u2Index, CCommandAccount& objCommandAccount, uint16& u2SendQueueMax, CSendCacheManager& objSendCacheManager)
{
    //�����̳߳�ʼ��ͳ��ģ�������
    char szName[MAX_BUFF_50] = { '\0' };
    sprintf_safe(szName, MAX_BUFF_50, "�����߳�(%d)", u2Index);
    objCommandAccount.InitName(szName, App_MainConfig::instance()->GetMaxCommandCount());

    //��ʼ��ͳ��ģ�鹦��
    objCommandAccount.Init(App_MainConfig::instance()->GetCommandAccount(),
                           App_MainConfig::instance()->GetCommandFlow(),
                           App_MainConfig::instance()->GetPacketTimeOut());

    //��ʼ����������ͻ�������
    u2SendQueueMax = App_MainConfig::instance()->GetSendQueueMax();

    //��ʼ�����ͻ����
    objSendCacheManager.Init(App_MainConfig::instance()->GetBlockCount(), App_MainConfig::instance()->GetBlockSize());
}
