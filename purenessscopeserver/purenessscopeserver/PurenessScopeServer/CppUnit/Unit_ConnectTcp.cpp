#include "Unit_ConnectTcp.h"

#ifdef _CPPUNIT_TEST

CUnit_ConnectTcp::~CUnit_ConnectTcp()
{

}

void CUnit_ConnectTcp::setUp(void)
{
    m_nServerID = 1;
}

void CUnit_ConnectTcp::tearDown(void)
{
    m_nServerID = 0;
}

void CUnit_ConnectTcp::Test_Connect_Tcp_Server(void)
{
    bool blRet                 = false;
    CPostServerData objPostServerData;
    objPostServerData.SetServerID(m_nServerID);

    //����Զ�̷�����
    if (false == App_ClientReConnectManager::instance()->Connect(m_nServerID,
            "127.0.0.1",
            10002,
            TYPE_IPV4,
            (IClientMessage*)&objPostServerData))
    {
        OUR_DEBUG((LM_INFO, "[Test_Connect_Tcp_Server]Connect[127.0.0.1:10002]Tcp connect is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_Connect_Tcp_Server]Connect[127.0.0.1:10002]Tcp connect is fail.", true == blRet);
        return;
    }

    ACE_Time_Value tvConnectSleep(0, 10000);
    ACE_OS::sleep(tvConnectSleep);

    //���Զ�ʱִ�г���
    ACE_Time_Value tvNow = ACE_OS::gettimeofday();
    App_ClientReConnectManager::instance()->handle_timeout(tvNow, NULL);

    //��õ�ǰ������Ϣ
    vecClientConnectInfo VecClientConnectInfo;
    App_ClientReConnectManager::instance()->GetConnectInfo(VecClientConnectInfo);

    if (1 != VecClientConnectInfo.size())
    {
        OUR_DEBUG((LM_INFO, "[Test_Connect_Tcp_Server]App_ClientReConnectManager::instance()->GetConnectInfo() is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_Connect_Tcp_Server]App_ClientReConnectManager::instance()->GetConnectInfo() is fail.", true == blRet);
        return;
    }

    //���ServerID������״̬
    EM_Server_Connect_State emConnectState = App_ClientReConnectManager::instance()->GetConnectState(m_nServerID);

    if (emConnectState != SERVER_CONNECT_OK)
    {
        OUR_DEBUG((LM_INFO, "[Test_Connect_Tcp_Server]App_ClientReConnectManager::instance()->GetConnectState is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_Connect_Tcp_Server]App_ClientReConnectManager::instance()->GetConnectState is fail.", true == blRet);
        return;
    }

    //��õ�ǰ�ѽ�˿���Ϣ
    App_ClientReConnectManager::instance()->GetServerAddr(m_nServerID);

    _ClientIPInfo objServerIPInfo;
    App_ClientReConnectManager::instance()->GetServerIPInfo(m_nServerID, objServerIPInfo);

    //����OK
    if (false == App_ClientReConnectManager::instance()->SetServerConnectState(m_nServerID, SERVER_CONNECT_OK))
    {
        OUR_DEBUG((LM_INFO, "[Test_Connect_Tcp_Server]App_ClientReConnectManager::instance()->SetServerConnectState is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_Connect_Tcp_Server]App_ClientReConnectManager::instance()->SetServerConnectState is fail.", true == blRet);
        return;
    }

    //ƴװ���Է�������
    char szSendBuffer[MAX_BUFF_200] = { '\0' };
    char szBuff[20]                 = { '\0' };
    char szSession[32]              = { '\0' };
    sprintf_safe(szBuff, 20, "freeeyes");
    sprintf_safe(szSession, 32, "FREEEYES");

    short sVersion = 1;
    short sCommand = (short)COMMAND_AUTOTEST_HEAD;
    int nPacketLen = ACE_OS::strlen(szBuff);

    memcpy(szSendBuffer, (char*)&sVersion, sizeof(short));
    memcpy((char*)&szSendBuffer[2], (char*)&sCommand, sizeof(short));
    memcpy((char*)&szSendBuffer[4], (char*)&nPacketLen, sizeof(int));
    memcpy((char*)&szSendBuffer[8], (char*)&szSession, sizeof(char) * 32);
    memcpy((char*)&szSendBuffer[40], (char*)szBuff, sizeof(char) * nPacketLen);
    int nSendLen = nPacketLen + 40;

    char* pData = (char* )szSendBuffer;

    if (false == App_ClientReConnectManager::instance()->SendData(m_nServerID, pData, nSendLen, false))
    {
        OUR_DEBUG((LM_INFO, "[Test_Connect_Tcp_Server]Send TCP [127.0.0.1:10002] is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_Connect_Tcp_Server]Send TCP [127.0.0.1:10002] is fail.\n", true == blRet);
    }

    //�ȴ�3�봦��������ݵ�ʱ��
    ACE_Time_Value tvSleep(0, 20*1000);
    ACE_OS::sleep(tvSleep);

    App_ClientReConnectManager::instance()->Close(m_nServerID);
    OUR_DEBUG((LM_INFO, "[Test_Connect_Tcp_Server]Close OK.\n"));
}

#endif


