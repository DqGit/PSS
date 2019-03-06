
#pragma once

#include "MyACELoggingStrategy.h"
#include <string>

#define LOG_CONFIG_ARGV_COUNT   6

//�޸�ACE_Logging_Strategy��һ��BUG
//����ڶ��߳���˫д���µ�tellp()�����̰߳�ȫ������
//�����޸�ACEԴ�룬�������һ������

class Logging_Config_Param
{
public:
    Logging_Config_Param();
    ~Logging_Config_Param();

    Logging_Config_Param(const Logging_Config_Param& ar);

    Logging_Config_Param& operator = (const Logging_Config_Param& ar)
    {
        ACE_UNUSED_ARG(ar);

        return *this;
    }


    //�ļ���С���ʱ��(Secs)
    int m_iChkInterval;

    //ÿ����־�ļ�����С(KB)
    int m_iLogFileMaxSize;

    //��־�ļ�������
    int m_iLogFileMaxCnt;

    //�Ƿ����ն˷���
    int m_bSendTerminal;

    //��־�ȼ�����
    char m_strLogLevel[128];

    //��־�ļ���ȫ·��
    char m_strLogFile[256];
};

class Frame_Logging_Strategy
{
public:
    Frame_Logging_Strategy();
    ~Frame_Logging_Strategy();

    Frame_Logging_Strategy(const Frame_Logging_Strategy& ar);

    Frame_Logging_Strategy& operator = (const Frame_Logging_Strategy& ar)
    {
        ACE_UNUSED_ARG(ar);

        return *this;
    }

    //��־����
    std::string GetLogLevel(const std::string& strLogLevel);

    int InitLogStrategy();

    //��ʼ����־����
    int InitLogStrategy(Logging_Config_Param& ConfigParam);

    //��������
    int EndLogStrategy();

    //�õ�pLogStrategyָ��
    My_ACE_Logging_Strategy* GetStrategy();

private:
    ACE_Reactor* pLogStraReactor;
    My_ACE_Logging_Strategy* pLogStrategy;
};

