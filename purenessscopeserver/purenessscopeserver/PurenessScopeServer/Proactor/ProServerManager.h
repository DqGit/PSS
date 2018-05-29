#ifndef _PROSERVERMANAGER_H
#define _PROSERVERMANAGER_H

#include "BaseServerManager.h"
#include "ProConnectAccept.h"
#include "ProConsoleAccept.h"
#include "AceProactorManager.h"
#include "ClientProConnectManager.h"
#include "ProUDPManager.h"
#include "XmlConfig.h"

//��ӶԷ��������Ƶ�֧�֣�Consoleģ������֧�������Է������Ŀ���
//add by freeeyes

class Frame_Logging_Strategy;

class CProServerManager : public IServerManager
{
public:
    CProServerManager(void);
    ~CProServerManager(void);

    bool Init();
    bool Start();
    bool Close();

private:
    CProConsoleConnectAcceptor m_ProConsoleConnectAcceptor;      //���ڹ�������������
    Frame_Logging_Strategy*    m_pFrameLoggingStrategy;          //�������
    CFrameCommand              m_objFrameCommand;                //�������
};

typedef ACE_Singleton<CProServerManager, ACE_Null_Mutex> App_ProServerManager;
#endif
