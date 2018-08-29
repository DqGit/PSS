#ifndef _TCPREDIRECTION_H
#define _TCPREDIRECTION_H

//��������͸��ת�������ݽӿڡ�
//�����������һ����ܹ���
//add by freeeyes

#include "define.h"
#include "XmlConfig.h"
#include "HashTable.h"
#include "BuffPacketManager.h"
#include "IMessageBlockManager.h"
#include "ClientCommand.h"
#include "IClientManager.h"
#include "IConnectManager.h"

class CRedirectionData : public IClientMessage
{
public:
    CRedirectionData() : m_u4ServerID(0), m_u1Mode(0), m_pConnectManager(NULL)
    {
    }

    virtual ~CRedirectionData()
    {
    }

    void SetMode(uint8 u1Mode)
    {
        m_u1Mode = u1Mode;
    }

    void SetConnectManager(IConnectManager* pConnectManager)
    {
        m_pConnectManager = pConnectManager;
    }

    virtual bool Need_Send_Format()
    {
        //�����Ҫ���÷�����װ�����������ﷵ��true
        //����Ҫ�Ļ����ﷵ��false��Send_Format_data�����Ͳ��ᱻ����
        return true;
    }

    //��������ڼ䷢�����ݵ���װ
    virtual bool Send_Format_data(char* pData, uint32 u4Len, IMessageBlockManager* pMessageBlockManager, ACE_Message_Block*& mbSend)
    {
        //�жϻ�����Ƿ���ڣ�����������򷵻�ʧ��
        if (NULL == pMessageBlockManager)
        {
            return false;
        }

        //���뷢�͵��ڴ�
        mbSend = pMessageBlockManager->Create(u4Len);

        if (NULL == mbSend)
        {
            OUR_DEBUG((LM_INFO, "[CRedirectionData::Send_Format_data](%d)Format Create Data NULL(%d) OK.\n", m_u4ServerID, u4Len));
            return false;
        }

        //�����Ҫ��װ���ݣ�������д��Ĵ���
        //����ֻ��������ֻ���򵥵��ڴ濽������ʾ�����Ĳ�������
        memcpy_safe(pData, u4Len, mbSend->wr_ptr(), u4Len);
        mbSend->wr_ptr(u4Len);
        return true;
    }

    //�����ṩ��������ƴ���㷨������������ݰ�������RecvData����
    virtual bool Recv_Format_data(ACE_Message_Block* mbRecv, IMessageBlockManager* pMessageBlockManager, uint16& u2CommandID, ACE_Message_Block*& mbFinishRecv, EM_PACKET_ROUTE& emPacketRoute)
    {
        ACE_UNUSED_ARG(pMessageBlockManager);
        emPacketRoute = PACKET_ROUTE_SELF;

        u2CommandID = 0x1000;

        mbFinishRecv = mbRecv;

        return true;
    }

    virtual bool RecvData(uint16 u2CommandID, ACE_Message_Block* mbRecv, _ClientIPInfo objServerIPInfo)
    {
        //���ݰ��Ѿ���ȫ�������ﴦ������
        ACE_UNUSED_ARG(u2CommandID);

        if (NULL == m_pConnectManager || 1 == m_u1Mode)
        {
            return true;
        }

        IBuffPacket* pBuffPacket = App_BuffPacketManager::instance()->Create(__FILE__, __LINE__);

        pBuffPacket->WriteStream((char* )mbRecv->rd_ptr(), (uint32)mbRecv->length());

        //������ת����ָ����ConnectID
        m_pConnectManager->PostMessage(m_u4ServerID,
                                       pBuffPacket,
                                       SENDMESSAGE_JAMPNOMAL,
                                       0,
                                       PACKET_SEND_IMMEDIATLY,
                                       PACKET_IS_FRAMEWORK_RECYC,
                                       0);
        return true;
    }

    virtual void ReConnect(int nServerID)
    {
        OUR_DEBUG((LM_INFO, "[CRedirectionData::ReConnect]nServerID=%d.\n", nServerID));
    }

    virtual bool ConnectError(int nError, _ClientIPInfo objServerIPInfo)
    {
        ACE_UNUSED_ARG(objServerIPInfo);
        OUR_DEBUG((LM_INFO, "[CRedirectionData::ConnectError]nServerID=%d, nError=%d.\n", m_u4ServerID, nError));
        return true;
    }

    void SetServerID(uint32 u4ServerID)
    {
        m_u4ServerID = u4ServerID;
    }

private:
    uint32                     m_u4ServerID;
    uint8                      m_u1Mode;            //0 ����Ӧ����, 1 ��Ӧ����
    IConnectManager*           m_pConnectManager;

};

class CTcpRedirection
{
public:
    CTcpRedirection();
    ~CTcpRedirection();

    void Close();

    void Init(xmlTcpRedirection& objCXmlTcpRedirection, uint32 u4MaxHandlerCount, IClientManager* pClientManager, IConnectManager* pConnectManager);

    void ConnectRedirect(uint32 u4SrcPort, uint32 u4ConnectID);

    void DataRedirect(uint32 u4ConnectID, ACE_Message_Block* mb);

    void CloseRedirect(uint32 u4ConnectID);

private:
    CHashTable<xmlTcpRedirection::_RedirectionInfo> m_objRedirectList;          //ת���������ӿ��б�
    CHashTable<CRedirectionData>                    m_objRedirectConnectList;   //ת����������Ӧ���ӽṹ
    uint16                                          m_u2Count;
    IClientManager*                                 m_pClientManager;
    IConnectManager*                                m_pConnectManager;
};

#endif
