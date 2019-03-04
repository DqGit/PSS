#include "ControlListen.h"

CControlListen::CControlListen()
{
}

CControlListen::~CControlListen()
{
}

bool CControlListen::AddListen( const char* pListenIP, uint32 u4Port, uint8 u1IPType, int nPacketParseID)
{
    bool blState = App_ConnectAcceptorManager::instance()->CheckIPInfo(pListenIP, u4Port);

    if(true == blState)
    {
        //��ǰ�����Ѿ����ڣ��������ظ�����
        OUR_DEBUG((LM_INFO, "[CServerManager::AddListen](%s:%d) is exist.\n", pListenIP, u4Port));
        return false;
    }

    ACE_INET_Addr listenAddr;

    //�ж�IPv4����IPv6
    int nErr = 0;

    if (u1IPType == TYPE_IPV4)
    {
        nErr = listenAddr.set(u4Port, pListenIP);
    }
    else
    {
        nErr = listenAddr.set(u4Port, pListenIP, 1, PF_INET6);
    }

    if (nErr != 0)
    {
        OUR_DEBUG((LM_INFO, "[CControlListen::AddListen](%s:%d)set_address error[%d].\n", pListenIP, u4Port, errno));
        return false;
    }

    //�õ�������
    ConnectAcceptor* pConnectAcceptor = App_ConnectAcceptorManager::instance()->GetNewConnectAcceptor();

    if (NULL == pConnectAcceptor)
    {
        OUR_DEBUG((LM_INFO, "[CControlListen::AddListen](%s:%d)pConnectAcceptor is NULL.\n", pListenIP, u4Port));
        return false;
    }

    pConnectAcceptor->SetPacketParseInfoID(nPacketParseID);

    int nRet = pConnectAcceptor->open2(listenAddr,
                                       App_ReactorManager::instance()->GetAce_Reactor(REACTOR_CLIENTDEFINE),
                                       ACE_NONBLOCK,
                                       (int)GetXmlConfigAttribute(xmlNetWorkMode)->BackLog);

    if (-1 == nRet)
    {
        OUR_DEBUG((LM_INFO, "[CControlListen::AddListen] Listen from [%s:%d] error(%d).\n",
                   listenAddr.get_host_addr(),
                   listenAddr.get_port_number(), errno));
        return false;
    }

    OUR_DEBUG((LM_INFO, "[CControlListen::AddListen] Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));

    return true;
}

bool CControlListen::DelListen( const char* pListenIP, uint32 u4Port )
{
    bool blState = App_ConnectAcceptorManager::instance()->CheckIPInfo(pListenIP, u4Port);

    if(false == blState)
    {
        //��ǰ�����Ѿ����ڣ��������ظ�����
        OUR_DEBUG((LM_INFO, "[CControlListen::AddListen](%s:%d) is exist.\n", pListenIP, u4Port));
        return false;
    }

    return App_ConnectAcceptorManager::instance()->Close(pListenIP, u4Port);
}

uint32 CControlListen::GetListenCount()
{
    m_vecListenList.clear();

    if (0 == App_ConnectAcceptorManager::instance()->GetCount())
    {
        //�����δ��������Ҫ�������ļ��л�ȡ
        int nServerPortCount = (int)GetXmlConfigAttribute(xmlTCPServerIPs)->vec.size();

        for (int i = 0; i < nServerPortCount; i++)
        {
            _ControlInfo objInfo;

            sprintf_safe(objInfo.m_szListenIP,
                         MAX_BUFF_20, "%s", GetXmlConfigAttribute(xmlTCPServerIPs)->vec[i].ip.c_str());
            objInfo.m_u4Port = GetXmlConfigAttribute(xmlTCPServerIPs)->vec[i].port;
            m_vecListenList.push_back(objInfo);
        }
    }
    else
    {
        for (int i = 0; i < App_ConnectAcceptorManager::instance()->GetCount(); i++)
        {
            ConnectAcceptor* pConnectAcceptor = App_ConnectAcceptorManager::instance()->GetConnectAcceptor(i);

            if (NULL != pConnectAcceptor)
            {
                _ControlInfo objInfo;
                sprintf_safe(objInfo.m_szListenIP,
                             MAX_BUFF_20, "%s", pConnectAcceptor->GetListenIP());
                objInfo.m_u4Port = pConnectAcceptor->GetListenPort();
                m_vecListenList.push_back(objInfo);
            }
        }
    }

    return (uint32)m_vecListenList.size();
}

bool CControlListen::ShowListen(uint32 u4Index, _ControlInfo& objControlInfo)
{
    if (u4Index >= m_vecListenList.size())
    {
        return false;
    }

    objControlInfo = m_vecListenList[u4Index];
    return true;
}

uint32 CControlListen::GetServerID()
{
    return GetXmlConfigAttribute(xmlServerID)->id;
}
