#include "Uint_ConnectHandler.h"

#ifdef _CPPUNIT_TEST

CUnit_ConnectHandler::~CUnit_ConnectHandler()
{
    m_pConnectHandler = NULL;
}

void CUnit_ConnectHandler::setUp(void)
{
    m_pConnectHandler = new CConnectHandler();
    m_pConnectHandler->Init(1);
    m_pConnectHandler->SetConnectID(111);
    m_pConnectHandler->SetPacketParseInfoID(1);
}

void CUnit_ConnectHandler::tearDown(void)
{
    delete m_pConnectHandler;
    m_pConnectHandler = NULL;
}

void CUnit_ConnectHandler::Test_ConnectHandler_Stream(void)
{
    //���Դ���ָ���ķ�Ӧ��
    bool blRet = false;

    //ƴװ���Է�������
    char szSendBuffer[MAX_BUFF_200] = { '\0' };
    char szBuff[20] = { '\0' };
    char szSession[32] = { '\0' };
    sprintf_safe(szBuff, 20, "freeeyes");
    sprintf_safe(szSession, 32, "FREEEYES");

    //���Եõ�HashID
    m_pConnectHandler->SetHashID(111);

    if (111 != m_pConnectHandler->GetHashID())
    {
        OUR_DEBUG((LM_INFO, "[Test_ConnectHandler_Stream]GetHashID is fail(%d).\n", m_pConnectHandler->GetHashID()));
        CPPUNIT_ASSERT_MESSAGE("[Test_ConnectHandler_Stream]GetHashID is fail.", true == blRet);
        return;
    }

    //��������ģʽ�����ݰ�
    short sVersion = 1;
    short sCommand = (short)0x1000;
    int nPacketLen = ACE_OS::strlen(szBuff);

    memcpy(szSendBuffer, (char*)&sVersion, sizeof(short));
    memcpy((char*)&szSendBuffer[2], (char*)&sCommand, sizeof(short));
    memcpy((char*)&szSendBuffer[4], (char*)&nPacketLen, sizeof(int));
    memcpy((char*)&szSendBuffer[8], (char*)&szSession, sizeof(char) * 32);
    memcpy((char*)&szSendBuffer[40], (char*)szBuff, sizeof(char) * nPacketLen);
    uint32 u4SendLen = nPacketLen + 40;

    ACE_Message_Block* pmb = App_MessageBlockManager::instance()->Create(u4SendLen);
    memcpy_safe(szSendBuffer, u4SendLen, pmb->wr_ptr(), u4SendLen);
    pmb->wr_ptr(u4SendLen);

    blRet = m_pConnectHandler->Test_Paceket_Parse_Stream(pmb);

    if (false == blRet)
    {
        OUR_DEBUG((LM_INFO, "[Test_ConnectHandler_Stream]Test_Paceket_Parse_Stream is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_ConnectHandler_Stream]Test_Paceket_Parse_Stream is fail.", true == blRet);
    }
}

void CUnit_ConnectHandler::Test_ConnectHandler_CloseMessages(void)
{
    bool blRet = false;

    blRet = m_pConnectHandler->SendCloseMessage();

    if (false == blRet)
    {
        OUR_DEBUG((LM_INFO, "[Test_ConnectHandler_CloseMessages]SendCloseMessage is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_ConnectHandler_CloseMessages]SendCloseMessage is fail.", true == blRet);
    }
}

void CUnit_ConnectHandler::Test_ConnectHandler_Debug(void)
{
    char szText[MAX_BUFF_20] = { '\0' };

    sprintf_safe(szText, MAX_BUFF_20, "freeeyes");

    ACE_Message_Block* pmb = new ACE_Message_Block(20);

    memcpy_safe(szText, ACE_OS::strlen(szText), pmb->wr_ptr(), ACE_OS::strlen(szText));
    pmb->wr_ptr(ACE_OS::strlen(szText));

    m_pConnectHandler->SetIsLog(true);

    m_pConnectHandler->Output_Debug_Data(pmb, LOG_SYSTEM_DEBUG_CLIENTRECV);

    pmb->release();
}

void CUnit_ConnectHandler::Test_ConnectHandler_Close_Queue(void)
{
    bool blRet = false;
    blRet = App_ConnectManager::instance()->CloseConnect(1);

    if (false == blRet)
    {
        OUR_DEBUG((LM_INFO, "[Test_ConnectHandler_Close_Queue]CloseConnect is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_ConnectHandler_Close_Queue]CloseConnect is fail.", true == blRet);
    }
}

void CUnit_ConnectHandler::Test_ConnectHandler_PostMessage(void)
{
    bool blRet = false;
    char szData[10] = { '\0' };
    sprintf_safe(szData, 10, "freeeyes");

    IBuffPacket* pBuffPacket = App_BuffPacketManager::instance()->Create(__FILE__, __LINE__);

    (*pBuffPacket) << (uint32)1;

    //���ԶϿ�����
    App_ConnectManager::instance()->Close(1);

    App_ConnectManager::instance()->CloseUnLock(1);

    App_ConnectManager::instance()->SetRecvQueueTimeCost(1, 1000);

    //����Ⱥ������
    const char* ptrReturnData = reinterpret_cast<const char*>(szData);
    uint32 u4SendLen = (uint32)ACE_OS::strlen(szData);
    App_ConnectManager::instance()->PostMessageAll(ptrReturnData, u4SendLen, SENDMESSAGE_NOMAL, 0, true, false, 0);

    App_ConnectManager::instance()->PostMessageAll(ptrReturnData, u4SendLen, SENDMESSAGE_NOMAL, 0, false, false, 0);

    App_ConnectManager::instance()->PostMessageAll(pBuffPacket, SENDMESSAGE_NOMAL, 0, true, false, 0);

    App_ConnectManager::instance()->SetConnectName(1, "freeeyes");

    App_ConnectManager::instance()->SetIsLog(1, false);

    if (CLIENT_CONNECT_NO_EXIST != App_ConnectManager::instance()->GetConnectState(1))
    {
        OUR_DEBUG((LM_INFO, "[Test_ConnectHandler_PostMessage]GetConnectState is fail.\n"));
        CPPUNIT_ASSERT_MESSAGE("[Test_ConnectHandler_PostMessage]GetConnectState is fail.", true == blRet);
    }

    App_BuffPacketManager::instance()->Delete(pBuffPacket);
}

#endif

