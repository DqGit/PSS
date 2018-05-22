#include "ProServerManager.h"
#include "Frame_Logging_Strategy.h"

CProServerManager::CProServerManager(void)
{
    m_pFrameLoggingStrategy = NULL;
}

CProServerManager::~CProServerManager(void)
{
}

bool CProServerManager::Init()
{
    //�Ƿ��ACE_DEBUG�ļ��洢
    Server_Manager_Common_FrameLogging(m_pFrameLoggingStrategy);

    int nServerPortCount    = App_MainConfig::instance()->GetServerPortCount();
    int nReactorCount       = App_MainConfig::instance()->GetReactorCount();

    bool blState = false;

    //��ʼ��ģ��������ز���
    App_MessageManager::instance()->Init(App_MainConfig::instance()->GetMaxModuleCount(), App_MainConfig::instance()->GetMaxCommandCount());

    //��ʼ������ģ�����Ϣ
    App_ModuleLoader::instance()->Init(App_MainConfig::instance()->GetMaxModuleCount());

    //��ʼ����ֹIP�б�
    App_ForbiddenIP::instance()->Init(FORBIDDENIP_FILE);

    //��ʼ����־ϵͳ�߳�
    if (false == Server_Manager_Common_LogSystem())
    {
        return false;
    }

    //��ʼ�����ֶ����
    Server_Manager_Common_Pool();

    //��ʼ��ProConnectHandler�����
    if(App_MainConfig::instance()->GetMaxHandlerCount() <= 0)
    {
        //��ʼ��PacketParse�����
        App_ProConnectHandlerPool::instance()->Init(MAX_HANDLE_POOL);
    }
    else
    {
        //��ʼ��PacketParse�����
        App_ProConnectHandlerPool::instance()->Init(App_MainConfig::instance()->GetMaxHandlerCount());
    }

    //��ʼ�����ӹ�����
    App_ProConnectManager::instance()->Init(App_MainConfig::instance()->GetSendQueueCount());

    //��ʼ��������Ķ���ӿ�
    IConnectManager* pConnectManager       = dynamic_cast<IConnectManager*>(App_ProConnectManager::instance());
    IClientManager*  pClientManager        = dynamic_cast<IClientManager*>(App_ClientProConnectManager::instance());
    IUDPConnectManager* pUDPConnectManager = dynamic_cast<IUDPConnectManager*>(App_ProUDPManager::instance());
    IFrameCommand* pFrameCommand           = dynamic_cast<IFrameCommand*>(&m_objFrameCommand);
    IServerManager* pServerManager         = dynamic_cast<IServerManager*>(this);
    Server_Manager_Common_IObject(pConnectManager,
                                  pClientManager,
                                  pUDPConnectManager,
                                  pFrameCommand,
                                  pServerManager);

    //��ʼ��ģ����أ���Ϊ������ܰ������м���������Ӽ���
    if (false == Server_Manager_Common_Module())
    {
        return false;
    }

    //�����е��߳̿�ͬ������
    App_MessageServiceGroup::instance()->CopyMessageManagerList();

    //��ʼ��������
    uint32 u4ClientProactorCount = (uint32)nReactorCount - 3;

    if (!App_ProConnectAcceptManager::instance()->InitConnectAcceptor(nServerPortCount, u4ClientProactorCount))
    {
        OUR_DEBUG((LM_INFO, "[CProServerManager::Init]%s.\n", App_ProConnectAcceptManager::instance()->GetError()));
        return false;
    }

    //��ʼ����Ӧ������
    App_ProactorManager::instance()->Init((uint16)nReactorCount);

    //��ʼ����Ӧ��
    for (int i = 0; i < nReactorCount; i++)
    {
        OUR_DEBUG((LM_INFO, "[CProServerManager::Init()]... i=[%d].\n", i));

        if (App_MainConfig::instance()->GetNetworkMode() == NETWORKMODE_PRO_IOCP)
        {
            blState = App_ProactorManager::instance()->AddNewProactor(i, Proactor_WIN32, 1);
            OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewProactor NETWORKMODE = Proactor_WIN32.\n"));
        }
        else
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewProactor NETWORKMODE Error.\n"));
            return false;
        }

        if (!blState)
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Init]AddNewProactor [%d] Error.\n", i));
            return false;
        }
    }

    return true;
}

bool CProServerManager::Start()
{
    //����TCP����
    int nServerPortCount = App_MainConfig::instance()->GetServerPortCount();

    //��ʼ������Զ������
    for(int i = 0 ; i < nServerPortCount; i++)
    {
        ACE_INET_Addr listenAddr;

        _ServerInfo* pServerInfo = App_MainConfig::instance()->GetServerPort(i);

        if (false == Server_Manager_Common_Addr(pServerInfo, listenAddr))
        {
            return false;
        }

        //�õ�������
        ProConnectAcceptor* pConnectAcceptor = App_ProConnectAcceptManager::instance()->GetConnectAcceptor(i);

        if(NULL == pConnectAcceptor)
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start]pConnectAcceptor[%d] is NULL.\n", i));
            return false;
        }

        //���ü���IP��Ϣ
        pConnectAcceptor->SetPacketParseInfoID(pServerInfo->m_u4PacketParseInfoID);
        pConnectAcceptor->SetListenInfo(pServerInfo->m_szServerIP, (uint32)pServerInfo->m_nPort);

        ACE_Proactor* pProactor = App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE);

        if(NULL == pProactor)
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE) is NULL.\n"));
            return false;
        }

        int nRet = pConnectAcceptor->open(listenAddr, 0, 1, App_MainConfig::instance()->GetBacklog(), 1, pProactor);

        if(-1 == nRet)
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start] pConnectAcceptor->open[%d] is error.\n", i));
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start] Listen from [%s:%d] error(%d).\n",listenAddr.get_host_addr(), listenAddr.get_port_number(), errno));
            return false;
        }

        OUR_DEBUG((LM_INFO, "[CProServerManager::Start] Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));
    }

    //����UDP����
    int nUDPServerPortCount = App_MainConfig::instance()->GetUDPServerPortCount();

    for(int i = 0 ; i < nUDPServerPortCount; i++)
    {
        ACE_INET_Addr listenAddr;

        _ServerInfo* pServerInfo = App_MainConfig::instance()->GetUDPServerPort(i);

        CProactorUDPHandler* pProactorUDPHandler = App_ProUDPManager::instance()->Create();

        if(NULL == pProactorUDPHandler)
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start] pProactorUDPHandler is NULL[%d] is error.\n", i));
            return false;
        }
        else
        {
            pProactorUDPHandler->SetPacketParseInfoID(pServerInfo->m_u4PacketParseInfoID);
            int nErr = 0;

            if (false == Server_Manager_Common_Addr(pServerInfo, listenAddr))
            {
                return false;
            }

            ACE_Proactor* pProactor = App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE);

            if(NULL == pProactor)
            {
                OUR_DEBUG((LM_INFO, "[CProServerManager::Start]UDP App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE) is NULL.\n"));
                return false;
            }

            pProactorUDPHandler->SetRecvSize(pServerInfo->m_u4MaxRecvSize);

            if(0 != pProactorUDPHandler->OpenAddress(listenAddr, pProactor))
            {
                OUR_DEBUG((LM_INFO, "[CProServerManager::Start] UDP Listen from [%s:%d] error(%d).\n",listenAddr.get_host_addr(), listenAddr.get_port_number(), errno));
                return false;
            }

            OUR_DEBUG((LM_INFO, "[CProServerManager::Start] UDP Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));
        }
    }

    //������̨����˿ڼ���
    if(App_MainConfig::instance()->GetConsoleSupport() == CONSOLE_ENABLE)
    {
        ACE_INET_Addr listenConsoleAddr;

        int nErr = 0;

        if(App_MainConfig::instance()->GetConsoleIPType() == TYPE_IPV4)
        {
            if(ACE_OS::strcmp(App_MainConfig::instance()->GetConsoleIP(), "INADDR_ANY") == 0)
            {
                nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(),
                                             (uint32)INADDR_ANY);
            }
            else
            {
                nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(),
                                             App_MainConfig::instance()->GetConsoleIP());
            }
        }
        else
        {
            if(ACE_OS::strcmp(App_MainConfig::instance()->GetConsoleIP(), "INADDR_ANY") == 0)
            {
                nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(),
                                             (uint32)INADDR_ANY);
            }
            else
            {
                nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(),
                                             App_MainConfig::instance()->GetConsoleIP(), 1, PF_INET6);
            }
        }

        if(nErr != 0)
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start]listenConsoleAddr set_address error[%d].\n", errno));
            return false;
        }

        ACE_Proactor* pProactor = App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE);

        if(NULL == pProactor)
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->GetAce_Proactor(REACTOR_CLIENTDEFINE) is NULL.\n"));
            return false;
        }

        int nRet = m_ProConsoleConnectAcceptor.open(listenConsoleAddr, 0, 1, MAX_ASYNCH_BACKLOG, 1, pProactor, true);

        if(-1 == nRet)
        {
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start] m_ProConsoleConnectAcceptor.open is error.\n"));
            OUR_DEBUG((LM_INFO, "[CProServerManager::Start] Listen from [%s:%d] error(%d).\n",listenConsoleAddr.get_host_addr(), listenConsoleAddr.get_port_number(), errno));
            return false;
        }
    }

    //������־�����߳�
    if(0 != AppLogManager::instance()->Start())
    {
        AppLogManager::instance()->WriteLog(LOG_SYSTEM, "[CProServerManager::Init]AppLogManager is ERROR.");
    }
    else
    {
        AppLogManager::instance()->WriteLog(LOG_SYSTEM, "[CProServerManager::Init]AppLogManager is OK.");
    }

    //������ʱ��
    if(0 != App_TimerManager::instance()->activate())
    {
        OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_TimerManager::instance()->Start() is error.\n"));
        return false;
    }

    //������Ӧ��(�����ķ�Ӧ������Ϊ�����������Ҫ)
    if(!App_ProactorManager::instance()->StartOtherProactor())
    {
        OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->StartOtherProactor is error.\n"));
        return false;
    }

    //��ʼ����������ͨѶ��
    App_ClientProConnectManager::instance()->Init(App_ProactorManager::instance()->GetAce_Proactor(REACTOR_POSTDEFINE));

    //�����м���������ӹ�������ʱ��
    App_ClientProConnectManager::instance()->StartConnectTask(App_MainConfig::instance()->GetConnectServerCheck());

    //�������еĲ����ʼ������
    if (false == App_ModuleLoader::instance()->InitModule())
    {
        OUR_DEBUG((LM_INFO, "[CServerManager::Run]App_ModuleLoader::instance()->InitModule() is error.\n"));
        return false;
    }

    //��ʼ��Ϣ�����߳�
    App_MessageServiceGroup::instance()->Start();

    if(App_MainConfig::instance()->GetConnectServerRunType() == 1)
    {
        //�����첽�������������Ϣ���Ĺ���
        App_ServerMessageTask::instance()->Start();
    }

    //��ʼ�������ӷ��Ͷ�ʱ��
    App_ProConnectManager::instance()->StartTimer();


    //��բ���ÿͻ������ݽ���
    if (!App_ProactorManager::instance()->StartClientProactor())
    {
        OUR_DEBUG((LM_INFO, "[CProServerManager::Start]App_ProactorManager::instance()->StartClientProactor is error.\n"));
        return false;
    }

    return true;
}

bool CProServerManager::Close()
{
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close begin....\n"));
    App_ProConnectAcceptManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_ProConnectAcceptManager OK.\n"));

    m_ProConsoleConnectAcceptor.cancel();
    App_TimerManager::instance()->deactivate();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_TimerManager OK.\n"));

    App_ProUDPManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_ProUDPManager OK.\n"));

    App_ClientProConnectManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_ClientProConnectManager OK.\n"));

    App_ModuleLoader::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_ModuleLoader OK.\n"));

    App_ServerMessageTask::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_ServerMessageTask OK.\n"));

    App_MessageServiceGroup::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_MessageServiceGroup OK.\n"));

    App_ProConnectManager::instance()->CloseAll();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_ProConnectManager OK.\n"));

    AppLogManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close AppLogManager OK\n"));

    App_MessageManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_MessageManager OK.\n"));

    App_BuffPacketManager::instance()->Close_Object(CBuffPacketManager::Close_Callback);
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_BuffPacketManager OK\n"));

    App_ProactorManager::instance()->StopProactor();
    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close App_ReactorManager OK.\n"));

    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]Close end....\n"));

    if(NULL != m_pFrameLoggingStrategy)
    {
        m_pFrameLoggingStrategy->EndLogStrategy();
        SAFE_DELETE(m_pFrameLoggingStrategy);
    }

    OUR_DEBUG((LM_INFO, "[CProServerManager::Close]EndLogStrategy end....\n"));

    return true;
}
