#include "ServerManager.h"

CServerManager::CServerManager(void)
{
    m_pFrameLoggingStrategy = NULL;
}

CServerManager::~CServerManager(void)
{
    OUR_DEBUG((LM_INFO, "[CServerManager::~CServerManager].\n"));
}

bool CServerManager::Init()
{
    //�Ƿ��ACE_DEBUG�ļ��洢
    Server_Manager_Common_FrameLogging(m_pFrameLoggingStrategy);

    int nServerPortCount = App_MainConfig::instance()->GetServerPortCount();
    int nReactorCount = App_MainConfig::instance()->GetReactorCount();

    //��ʼ��ģ��������ز���
    App_MessageManager::instance()->Init(App_MainConfig::instance()->GetMaxModuleCount(), App_MainConfig::instance()->GetMaxCommandCount());

    //��ʼ������ģ�����Ϣ
    App_ModuleLoader::instance()->Init(App_MainConfig::instance()->GetMaxModuleCount());

    //��ʼ����ֹIP�б�
    App_ForbiddenIP::instance()->Init(FORBIDDENIP_FILE);

    OUR_DEBUG((LM_INFO, "[CServerManager::Init]nReactorCount=%d.\n", nReactorCount));

    //Ϊ�������׼�������epoll��epollet��ʼ������������ȥ��,��Ϊ�ڶ������epoll_create�������ӽ�����ȥ����
    if (NETWORKMODE_RE_EPOLL != App_MainConfig::instance()->GetNetworkMode() && NETWORKMODE_RE_EPOLL_ET != App_MainConfig::instance()->GetNetworkMode())
    {
        //��ʼ����Ӧ������
        App_ReactorManager::instance()->Init((uint16)nReactorCount);

        OUR_DEBUG((LM_INFO, "[CServerManager::Init]****1*******.\n"));
        Init_Reactor((uint8)nReactorCount, App_MainConfig::instance()->GetNetworkMode());
    }

    //��ʼ����־ϵͳ�߳�
    if (false == Server_Manager_Common_LogSystem())
    {
        return false;
    }

    //��ʼ�����ֶ����
    Server_Manager_Common_Pool();

    //��ʼ��ConnectHandler�����
    if (App_MainConfig::instance()->GetMaxHandlerCount() <= 0)
    {
        //��ʼ��PacketParse�����
        App_ConnectHandlerPool::instance()->Init(MAX_HANDLE_POOL);
    }
    else
    {
        //��ʼ��PacketParse�����
        App_ConnectHandlerPool::instance()->Init(App_MainConfig::instance()->GetMaxHandlerCount());
    }

    //��ʼ�����ӹ�����
    App_ConnectManager::instance()->Init(App_MainConfig::instance()->GetSendQueueCount());

    //��ʼ��������Ķ���ӿ�
    IConnectManager* pConnectManager       = dynamic_cast<IConnectManager*>(App_ConnectManager::instance());
    IClientManager*  pClientManager        = dynamic_cast<IClientManager*>(App_ClientReConnectManager::instance());
    IUDPConnectManager* pUDPConnectManager = dynamic_cast<IUDPConnectManager*>(App_ReUDPManager::instance());
    IFrameCommand* pFrameCommand           = dynamic_cast<IFrameCommand*>(&m_objFrameCommand);
    IServerManager* pServerManager         = dynamic_cast<IServerManager*>(this);
    Server_Manager_Common_IObject(pConnectManager,
                                  pClientManager,
                                  pUDPConnectManager,
                                  pFrameCommand,
                                  pServerManager);

    //��ʼ����Ϣ�����߳�
    App_MessageServiceGroup::instance()->Init(App_MainConfig::instance()->GetThreadCount(),
            App_MainConfig::instance()->GetMsgMaxQueue(),
            App_MainConfig::instance()->GetMsgLowMark(),
            App_MainConfig::instance()->GetMgsHighMark());

    //��ʼ��ģ����أ���Ϊ������ܰ������м���������Ӽ���
    if (false == Server_Manager_Common_Module())
    {
        return false;
    }

    //�����е��߳̿�ͬ������
    App_MessageServiceGroup::instance()->CopyMessageManagerList();

    //��ʼ��������
    uint32 u4ClientReactorCount = (uint32)nReactorCount - 3;

    if (!App_ConnectAcceptorManager::instance()->InitConnectAcceptor(nServerPortCount, u4ClientReactorCount))
    {
        OUR_DEBUG((LM_INFO, "[CServerManager::Init]%s.\n", App_ConnectAcceptorManager::instance()->GetError()));
        return false;
    }

    return true;
}

bool CServerManager::Start()
{
    //����TCP������ʼ��
    bool blRet = false;
    blRet = Start_Tcp_Listen();

    if (false == blRet)
    {
        return blRet;
    }

    //����UDP����
    blRet = Start_Udp_Listen();

    if (false == blRet)
    {
        return blRet;
    }

    //������̨����˿ڼ���
    blRet = Start_Console_Tcp_Listen();

    if (false == blRet)
    {
        return blRet;
    }

    if (App_MainConfig::instance()->GetProcessCount() > 1)
    {
        Multiple_Process_Start();
    }
    else
    {
        if (false == Run())
        {
            OUR_DEBUG((LM_INFO, "child Run failure.\n"));
            return false;
        }
    }

    return true;
}

bool CServerManager::Init_Reactor(uint8 u1ReactorCount, uint8 u1NetMode)
{
    bool blState = true;

    //��ʼ����Ӧ��
    for (uint8 i = 0; i < u1ReactorCount; i++)
    {
        OUR_DEBUG((LM_INFO, "[CServerManager::Init_Reactor]... i=[%d].\n", i));

        if (u1NetMode == NETWORKMODE_RE_SELECT)
        {
            blState = App_ReactorManager::instance()->AddNewReactor(i, Reactor_Select, 1);
            OUR_DEBUG((LM_INFO, "[CServerManager::Init_Reactor]AddNewReactor REACTOR_CLIENTDEFINE = Reactor_Select.\n"));
        }
        else if (u1NetMode == NETWORKMODE_RE_TPSELECT)
        {
            blState = App_ReactorManager::instance()->AddNewReactor(i, Reactor_TP, 1);
            OUR_DEBUG((LM_INFO, "[CServerManager::Init_Reactor]AddNewReactor REACTOR_CLIENTDEFINE = Reactor_TP.\n"));
        }
        else if (u1NetMode == NETWORKMODE_RE_EPOLL)
        {
            blState = App_ReactorManager::instance()->AddNewReactor(i, Reactor_DEV_POLL, 1, App_MainConfig::instance()->GetMaxHandlerCount());
            OUR_DEBUG((LM_INFO, "[CServerManager::Init_Reactor]AddNewReactor REACTOR_CLIENTDEFINE = Reactor_DEV_POLL.\n"));
        }
        else if (u1NetMode == NETWORKMODE_RE_EPOLL_ET)
        {
            blState = App_ReactorManager::instance()->AddNewReactor(i, Reactor_DEV_POLL_ET, 1, App_MainConfig::instance()->GetMaxHandlerCount());
            OUR_DEBUG((LM_INFO, "[CServerManager::Init_Reactor]AddNewReactor REACTOR_CLIENTDEFINE = Reactor_DEV_POLL_ET.\n"));
        }
        else
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Init_Reactor]AddNewProactor NETWORKMODE Error.\n"));
            return false;
        }

        if (!blState)
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Init_Reactor]AddNewReactor [%d] Error.\n", i));
            OUR_DEBUG((LM_INFO, "[CServerManager::Init_Reactor]Error=%s.\n", App_ReactorManager::instance()->GetError()));
            return false;
        }
    }

    return blState;
}

bool CServerManager::Run()
{
    //��Ӧ����̣�epoll�������ӽ�������г�ʼ��
    if (NETWORKMODE_RE_EPOLL == App_MainConfig::instance()->GetNetworkMode() || NETWORKMODE_RE_EPOLL_ET == App_MainConfig::instance()->GetNetworkMode())
    {
        //��ʼ����Ӧ������
        App_ReactorManager::instance()->Init((uint16)App_MainConfig::instance()->GetReactorCount());

        if (false == Init_Reactor((uint8)App_MainConfig::instance()->GetReactorCount(), App_MainConfig::instance()->GetNetworkMode()))
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Run]Init_Reactor Error.\n"));
            return false;
        }
    }

    //�����м���������ӹ�����
    App_ClientReConnectManager::instance()->Init(App_ReactorManager::instance()->GetAce_Reactor(REACTOR_POSTDEFINE));

    int nServerPortCount = App_MainConfig::instance()->GetServerPortCount();

    //����������TCP��Ӧ��
    for (int i = 0; i < nServerPortCount; i++)
    {
        //�õ�������
        ConnectAcceptor* pConnectAcceptor = App_ConnectAcceptorManager::instance()->GetConnectAcceptor(i);

        //�򿪼�����Ӧ�¼�
        pConnectAcceptor->Run_Open(App_ReactorManager::instance()->GetAce_Reactor(REACTOR_CLIENTDEFINE));
    }

    m_ConnectConsoleAcceptor.Run_Open(App_ReactorManager::instance()->GetAce_Reactor(REACTOR_CLIENTDEFINE));

    //����������UDP��Ӧ��
    uint16 u2UDPServerPortCount = App_MainConfig::instance()->GetUDPServerPortCount();

    for (uint16 i = 0; i < u2UDPServerPortCount; i++)
    {
        CReactorUDPHander* pReactorUDPHandler = App_ReUDPManager::instance()->GetUDPHandle((uint8)i);

        if (NULL != pReactorUDPHandler)
        {
            pReactorUDPHandler->Run_Open(App_ReactorManager::instance()->GetAce_Reactor(REACTOR_CLIENTDEFINE));
        }
    }

    //������־�����߳�
    if (0 != AppLogManager::instance()->Start())
    {
        AppLogManager::instance()->WriteLog(LOG_SYSTEM, "[CServerManager::Init]AppLogManager is ERROR.");
    }
    else
    {
        AppLogManager::instance()->WriteLog(LOG_SYSTEM, "[CServerManager::Init]AppLogManager is OK.");
    }

    //���������������߳�
    if (false == App_ClientReConnectManager::instance()->StartConnectTask(App_MainConfig::instance()->GetConnectServerCheck()))
    {
        OUR_DEBUG((LM_INFO, "[CServerManager::Run]StartConnectTask error.\n"));
        return false;
    }

    //������ʱ��
    if (0 != App_TimerManager::instance()->activate())
    {
        OUR_DEBUG((LM_INFO, "[CServerManager::Run]App_TimerManager::instance()->Start() is error.\n"));
        return false;
    }

    //�������з�Ӧ��(���ǿͻ��˽��յķ�Ӧ���������ﲻ������)
    if (!App_ReactorManager::instance()->StartOtherReactor())
    {
        OUR_DEBUG((LM_INFO, "[CServerManager::Run]App_ReactorManager::instance()->StartOtherReactor is error.\n"));
        return false;
    }

    //�������еĲ����ʼ������
    if (false == App_ModuleLoader::instance()->InitModule())
    {
        OUR_DEBUG((LM_INFO, "[CServerManager::Run]App_ModuleLoader::instance()->InitModule() is error.\n"));
        return false;
    }

    //��ʼ��Ϣ�����߳�
    App_MessageServiceGroup::instance()->Start();

    if (App_MainConfig::instance()->GetConnectServerRunType() == 1)
    {
        //�����첽�������������Ϣ���Ĺ���
        App_ServerMessageTask::instance()->Start();
    }

    //��ʼ�������ӷ��Ͷ�ʱ��
    App_ConnectManager::instance()->StartTimer();

    //���բ�������ͻ��˷�Ӧ�����ÿͻ������ݽ���
    if (!App_ReactorManager::instance()->StartClientReactor())
    {
        OUR_DEBUG((LM_INFO, "[CServerManager::Run]App_ReactorManager::instance()->StartClientReactor is error.\n"));
        return false;
    }

    ACE_Thread_Manager::instance()->wait();

    return true;
}

bool CServerManager::Start_Tcp_Listen()
{
    int nServerPortCount = App_MainConfig::instance()->GetServerPortCount();

    for (int i = 0; i < nServerPortCount; i++)
    {
        ACE_INET_Addr listenAddr;

        _ServerInfo* pServerInfo = App_MainConfig::instance()->GetServerPort(i);

        if (false == Server_Manager_Common_Addr(pServerInfo, listenAddr))
        {
            return false;
        }

        //�õ�������
        ConnectAcceptor* pConnectAcceptor = App_ConnectAcceptorManager::instance()->GetConnectAcceptor(i);

        if (NULL == pConnectAcceptor)
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Start]pConnectAcceptor[%d] is NULL.\n", i));
            return false;
        }

        pConnectAcceptor->SetPacketParseInfoID(pServerInfo->m_u4PacketParseInfoID);
        int nRet = pConnectAcceptor->Init_Open(listenAddr, 0, 1, 1, (int)App_MainConfig::instance()->GetBacklog());

        if (-1 == nRet)
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Start] pConnectAcceptor->open[%d] is error.\n", i));
            OUR_DEBUG((LM_INFO, "[CServerManager::Start] Listen from [%s:%d] error(%d).\n", listenAddr.get_host_addr(), listenAddr.get_port_number(), errno));
            return false;
        }

        OUR_DEBUG((LM_INFO, "[CServerManager::Start] Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));
    }

    return true;
}

bool CServerManager::Start_Udp_Listen()
{
    int nUDPServerPortCount = App_MainConfig::instance()->GetUDPServerPortCount();

    for (int i = 0; i < nUDPServerPortCount; i++)
    {
        ACE_INET_Addr listenAddr;

        _ServerInfo* pServerInfo = App_MainConfig::instance()->GetUDPServerPort(i);

        if (false == Server_Manager_Common_Addr(pServerInfo, listenAddr))
        {
            return false;
        }

        //�õ�������
        CReactorUDPHander* pReactorUDPHandler = App_ReUDPManager::instance()->Create();

        if (NULL == pReactorUDPHandler)
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Start]UDP pReactorUDPHandler[%d] is NULL.\n", i));
            return false;
        }

        pReactorUDPHandler->SetPacketParseInfoID(pServerInfo->m_u4PacketParseInfoID);
        int nRet = pReactorUDPHandler->OpenAddress(listenAddr);

        if (-1 == nRet)
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Start] UDP Listen from [%s:%d] error(%d).\n", listenAddr.get_host_addr(), listenAddr.get_port_number(), errno));
            return false;
        }

        OUR_DEBUG((LM_INFO, "[CServerManager::Start] UDP Listen from [%s:%d] OK.\n", listenAddr.get_host_addr(), listenAddr.get_port_number()));
    }

    return true;
}

bool CServerManager::Start_Console_Tcp_Listen()
{
    if (App_MainConfig::instance()->GetConsoleSupport() == CONSOLE_ENABLE)
    {
        ACE_INET_Addr listenConsoleAddr;
        int nErr = 0;

        if (App_MainConfig::instance()->GetConsoleIPType() == TYPE_IPV4)
        {
            if (ACE_OS::strcmp(App_MainConfig::instance()->GetConsoleIP(), "INADDR_ANY") == 0)
            {
                nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(), (uint32)INADDR_ANY);
            }
            else
            {
                nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(), App_MainConfig::instance()->GetConsoleIP());
            }
        }
        else
        {
            if (ACE_OS::strcmp(App_MainConfig::instance()->GetConsoleIP(), "INADDR_ANY") == 0)
            {
                nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(), (uint32)INADDR_ANY);
            }
            else
            {
                nErr = listenConsoleAddr.set(App_MainConfig::instance()->GetConsolePort(), App_MainConfig::instance()->GetConsoleIP(), 1, PF_INET6);
            }
        }

        if (nErr != 0)
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Start]listenConsoleAddr set_address error[%d].\n", errno));
            return false;
        }

        int nRet = m_ConnectConsoleAcceptor.Init_Open(listenConsoleAddr);

        if (-1 == nRet)
        {
            OUR_DEBUG((LM_INFO, "[CServerManager::Start] pConnectAcceptor->open is error.\n"));
            OUR_DEBUG((LM_INFO, "[CServerManager::Start] Listen from [%s:%d] error(%d).\n", listenConsoleAddr.get_host_addr(), listenConsoleAddr.get_port_number(), errno));
            return false;
        }
    }

    return true;
}

void CServerManager::Multiple_Process_Start()
{
#ifndef WIN32
    //��ǰ������̸߳���
    int nNumChlid = App_MainConfig::instance()->GetProcessCount();

    //���ʱ��������
    //�����̼��ʱ����������ÿ��5��һ�Σ�
    ACE_Time_Value tvMonitorSleep(5, 0);

    //�ļ���
    int fd_lock = 0;

    int nRet = 0;

    //��õ�ǰ·��
    char szWorkDir[MAX_BUFF_500] = { 0 };

    if (!ACE_OS::getcwd(szWorkDir, sizeof(szWorkDir)))
    {
        exit(1);
    }

    //��Linux�²��ö���̵ķ�ʽ����
    // �򿪣����������ļ�
    char szFileName[200] = { '\0' };
    //memset(szFileName, 0, sizeof(szFileName));
    sprintf(szFileName, "%s/lockwatch.lk", szWorkDir);
    fd_lock = open(szFileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    if (fd_lock < 0)
    {
        printf("open the flock and exit, errno = %d.\n", errno);
        exit(1);
    }

    //�鿴��ǰ�ļ����Ƿ�����
    nRet = SeeLock(fd_lock, 0, sizeof(int));

    if (nRet == -1 || nRet == 2)
    {
        printf("file is already exist!\n");
        exit(1);
    }

    //����ļ���û��������ס��ǰ�ļ���
    if (AcquireWriteLock(fd_lock, 0, sizeof(int)) != 0)
    {
        printf("lock the file failure and exit, idx = 0.\n");
        exit(1);
    }

    //д���ӽ�������Ϣ
    lseek(fd_lock, 0, SEEK_SET);

    for (int nIndex = 0; nIndex <= nNumChlid; nIndex++)
    {
        write(fd_lock, &nIndex, sizeof(nIndex));
    }

    //����������ӽ���
    while (1)
    {
        for (int nChlidIndex = 1; nChlidIndex <= nNumChlid; nChlidIndex++)
        {
            //����100ms
            ACE_Time_Value tvSleep(0, 100000);
            ACE_OS::sleep(tvSleep);

            //����ÿ���ӽ��̵����Ƿ񻹴���
            nRet = SeeLock(fd_lock, nChlidIndex * sizeof(int), sizeof(int));

            if (nRet == -1 || nRet == 2)
            {
                continue;
            }

            //����ļ���û�б������������ļ������������ӽ���
            int npid = ACE_OS::fork();

            if (npid == 0)
            {
                //���ļ���
                if (AcquireWriteLock(fd_lock, nChlidIndex * sizeof(int), sizeof(int)) != 0)
                {
                    printf("child %d AcquireWriteLock failure.\n", nChlidIndex);
                    exit(1);
                }

                //�����ӽ���
                if (false == Run())
                {
                    printf("child %d Run failure.\n", nChlidIndex);
                    exit(1);
                }

                //�ӽ�����ִ�������������˳�ѭ�����ͷ���
                ReleaseLock(fd_lock, nChlidIndex * sizeof(int), sizeof(int));
            }
        }

        //printf("child count(%d) is ok.\n", nNumChlid);
        //�����
        ACE_OS::sleep(tvMonitorSleep);
    }

#endif
}

bool CServerManager::Close()
{
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close begin....\n"));
    App_ConnectAcceptorManager::instance()->Close();
    m_ConnectConsoleAcceptor.close();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_TimerManager OK.\n"));
    App_TimerManager::instance()->deactivate();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ReUDPManager OK.\n"));
    App_ReUDPManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ModuleLoader OK.\n"));
    App_ClientReConnectManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ClientReConnectManager OK.\n"));
    App_ModuleLoader::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_MessageManager OK.\n"));
    App_ServerMessageTask::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ServerMessageTask OK.\n"));
    App_MessageServiceGroup::instance()->Close();
    OUR_DEBUG((LM_INFO, "[App_MessageServiceGroup::Close]Close App_MessageServiceGroup OK.\n"));
    App_ConnectManager::instance()->CloseAll();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ConnectManager OK.\n"));
    AppLogManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]AppLogManager OK\n"));
    App_MessageManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_MessageManager OK.\n"));
    App_BuffPacketManager::instance()->Close();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]BuffPacketManager OK\n"));
    App_ReactorManager::instance()->StopReactor();
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close App_ReactorManager OK.\n"));
    OUR_DEBUG((LM_INFO, "[CServerManager::Close]Close end....\n"));

    if (NULL != m_pFrameLoggingStrategy)
    {
        m_pFrameLoggingStrategy->EndLogStrategy();
        SAFE_DELETE(m_pFrameLoggingStrategy);
    }

    return true;
}


