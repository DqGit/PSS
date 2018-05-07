#include "FileLogger.h"



//********************************************************

CFileLogger::CFileLogger()
{
    m_pLogFileList   = NULL;
    m_nCount         = 0;
    m_u4BlockSize    = 0;
    m_u4PoolCount    = 0;
    m_u4CurrLogLevel = 0;
    m_szLogRoot[0]   = '\0';
}

CFileLogger::~CFileLogger()
{
    OUR_DEBUG((LM_INFO, "[CFileLogger::~CFileLogger].\n"));
    Close();
    OUR_DEBUG((LM_INFO, "[CFileLogger::~CFileLogger]End.\n"));
}

void CFileLogger::Close()
{
    if(NULL != m_pLogFileList)
    {
        for(int i = 0; i < m_nCount; i++)
        {
            if(NULL != m_pLogFileList[i])
            {
                SAFE_DELETE(m_pLogFileList[i]);
            }
        }

        SAFE_DELETE_ARRAY(m_pLogFileList);
        m_nCount = 0;
    }
}

int CFileLogger::DoLog(int nLogType, _LogBlockInfo* pLogBlockInfo)
{
    //����LogTypeȡ�࣬��õ�ǰ��־ӳ��λ��
    int nIndex = nLogType % m_nCount;

    if(NULL != m_pLogFileList[nIndex])
    {
        m_pLogFileList[nIndex]->doLog(pLogBlockInfo);
    }

    return 0;
}

int CFileLogger::GetLogTypeCount()
{
    return m_nCount;
}

bool CFileLogger::Init()
{
    CXmlOpeation objXmlOpeation;

    //Ĭ�ϵ����ļ������8M
    int nData                       = -1;
    uint32 u4FileMaxSize            = MAX_BUFF_500;
    uint16 u2LogID                  = 0;
    uint16 u2LogLevel               = 0;
    uint8  u1FileClass              = 0;
    uint8  u1DisPlay                = 0;
    char szFile[MAX_BUFF_1024]      = {'\0'};
    char szFileName[MAX_BUFF_100]   = {'\0'};
    char szServerName[MAX_BUFF_100] = {'\0'};
    char* pData = NULL;
    vector<_Log_File_Info> objvecLogFileInfo;

    Close();

    sprintf_safe(szFile, MAX_BUFF_1024, "%s", FILELOG_CONFIG);

    if(false == objXmlOpeation.Init(szFile))
    {
        OUR_DEBUG((LM_ERROR,"[CFileLogger::Init] Read Configfile[%s] failed\n", szFile));
        return false;
    }

    //�õ�����������
    objXmlOpeation.Read_XML_Data_Single_String("ServerLogHead", "Text", szServerName, MAX_BUFF_100);
    OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]strServerName=%s\n", szServerName));

    //�õ�������־����С
    objXmlOpeation.Read_XML_Data_Single_Uint32("ServerLogHead", "LogFileMaxSize", u4FileMaxSize);

    //�õ�����·��
    objXmlOpeation.Read_XML_Data_Single_String("LogPath", "Path", m_szLogRoot, MAX_BUFF_100);
    OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]m_strRoot=%s\n", m_szLogRoot));

    //�õ���־��������Ϣ����־��Ĵ�С
    objXmlOpeation.Read_XML_Data_Single_Uint32("LogPool", "BlockSize", m_u4BlockSize);

    //�õ���־��������Ϣ�����������־��ĸ���
    objXmlOpeation.Read_XML_Data_Single_Uint32("LogPool", "PoolCount", m_u4PoolCount);

    //�õ���־���еĵ�ǰ��־����
    //�˹��ܸ�л��/ka�̷� �ĺ��뷨����һ�������ٳɶ�ͻ��ۺ���
    objXmlOpeation.Read_XML_Data_Single_Uint32("LogLevel", "CurrLevel", m_u4CurrLogLevel);

    //�������ĸ���
    TiXmlElement* pNextTiXmlElement        = NULL;
    TiXmlElement* pNextTiXmlElementPos     = NULL;
    TiXmlElement* pNextTiXmlElementIdx     = NULL;
    TiXmlElement* pNextTiXmlElementDisplay = NULL;
    TiXmlElement* pNextTiXmlElementLevel   = NULL;

    while(true)
    {
        //�õ���־id
        if(true == objXmlOpeation.Read_XML_Data_Multiple_Uint16("LogInfo", "logid", u2LogID, pNextTiXmlElementIdx))
        {
            OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]u2LogID=%d\n", u2LogID));
        }
        else
        {
            break;
        }

        //�õ���־����
        objXmlOpeation.Read_XML_Data_Multiple_String("LogInfo", "logname", szFileName, MAX_BUFF_100, pNextTiXmlElement);
        OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]strFileValue=%s\n", szFileName));

        //�õ���־����
        objXmlOpeation.Read_XML_Data_Multiple_Uint8("LogInfo", "logtype", u1FileClass, pNextTiXmlElementPos);
        OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]u1FileClass=%d\n", u1FileClass));

        //�õ���־�����Դ��0Ϊ������ļ���1Ϊ�������Ļ
        objXmlOpeation.Read_XML_Data_Multiple_Uint8("LogInfo", "Display", u1DisPlay, pNextTiXmlElementDisplay);
        OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]u1DisPlay=%d\n", u1DisPlay));

        //�õ���־��ǰ����
        objXmlOpeation.Read_XML_Data_Multiple_Uint16("LogInfo", "Level", u2LogLevel, pNextTiXmlElementLevel);
        OUR_DEBUG((LM_ERROR, "[CFileLogger::readConfig]u4LogLevel=%d\n", u2LogLevel));

        //���뻺��
        _Log_File_Info obj_Log_File_Info;
        obj_Log_File_Info.m_u2LogID     = u2LogID;
        obj_Log_File_Info.m_u1FileClass = u1FileClass;
        obj_Log_File_Info.m_u1DisPlay   = u1DisPlay;
        obj_Log_File_Info.m_u2LogLevel  = u2LogLevel;
        sprintf_safe(obj_Log_File_Info.m_szFileName, 100, "%s", szFileName);

        objvecLogFileInfo.push_back(obj_Log_File_Info);
        m_nCount++;
    }

    //���������б�
    m_pLogFileList = new CLogFile*[m_nCount];
    memset(m_pLogFileList, 0, sizeof(CLogFile*)*m_nCount);

    for(int i = 0; i < (int)objvecLogFileInfo.size(); i++)
    {
        int nPos = objvecLogFileInfo[i].m_u2LogID % m_nCount;
        CLogFile* pLogFile = new CLogFile(m_szLogRoot, m_u4BlockSize, u4FileMaxSize);

        pLogFile->SetLoggerName(objvecLogFileInfo[i].m_szFileName);
        pLogFile->SetLoggerID(objvecLogFileInfo[i].m_u2LogID);
        pLogFile->SetLoggerClass(objvecLogFileInfo[i].m_u1FileClass);
        pLogFile->SetLevel(objvecLogFileInfo[i].m_u2LogLevel);
        pLogFile->SetServerName(szServerName);
        pLogFile->SetDisplay(objvecLogFileInfo[i].m_u1DisPlay);
        pLogFile->Init();

        if (false == pLogFile->Run())
        {
            OUR_DEBUG((LM_INFO, "[CFileLogger::Init]Run error.\n"));
        }

        m_pLogFileList[nPos] = pLogFile;
    }

    return true;
}

bool CFileLogger::ReSet(uint32 u4CurrLogLevel)
{
    //������־�ȼ�
    m_u4CurrLogLevel = u4CurrLogLevel;
    return true;
}

uint32 CFileLogger::GetBlockSize()
{
    return m_u4BlockSize;
}

uint32 CFileLogger::GetPoolCount()
{
    return m_u4PoolCount;
}

uint32 CFileLogger::GetCurrLevel()
{
    return m_u4CurrLogLevel;
}

uint16 CFileLogger::GetLogID(uint16 u2Index)
{
    if(u2Index >= m_nCount)
    {
        return 0;
    }

    return m_pLogFileList[u2Index]->GetLoggerID();
}

char* CFileLogger::GetLogInfoByServerName(uint16 u2LogID)
{
    int nIndex = u2LogID % m_nCount;

    return (char* )m_pLogFileList[nIndex]->GetServerName().c_str();
}

char* CFileLogger::GetLogInfoByLogName(uint16 u2LogID)
{
    int nIndex = u2LogID % m_nCount;

    return (char* )m_pLogFileList[nIndex]->GetLoggerName().c_str();
}

int CFileLogger::GetLogInfoByLogDisplay(uint16 u2LogID)
{
    int nIndex = u2LogID % m_nCount;

    return m_pLogFileList[nIndex]->GetDisPlay();
}

uint16 CFileLogger::GetLogInfoByLogLevel(uint16 u2LogID)
{
    int nIndex = u2LogID % m_nCount;

    return m_pLogFileList[nIndex]->GetLevel();
}


