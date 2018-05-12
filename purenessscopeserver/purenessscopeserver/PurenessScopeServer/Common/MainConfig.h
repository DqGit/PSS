#ifndef _MAINCONFIG_H
#define _MAINCONFIG_H

#include "define.h"
#include "XmlOpeation.h"

#include "ace/Singleton.h"
#include <vector>

#include "PacketParse.h"
#include "IpCheck.h"

//main.xml����ö��

/*1.ö��ֵ�Ը���s�����Ŀ��ܸ������ж��������Ϊ����Ԫ��
  2.��Ҫ����ö�ٱ���� MIN��MAX���䷶Χ���
  3.ͬ�ļ���ö�ٱ�������*/

enum XmlStart
{
	MIN = 0
};

enum XmlConfig
{
/*******************main.xml************************/

	/********��������********/
	RecvInfo = MIN,
	SendInfo,
	NetWorkMode,
	/********������Ϣ********/
	ConnectServer,
	ClientInfo,
	/********ģ������********/
	ModuleInfos,
	ModuleMangager,
	/********�����Ϣ********/
	Monitor,
	ThreadInfoAI,
	ThreadInfo,
	Console,
	ConsoleKey,
	/********�ļ���¼********/
	AceDebug,
	CommandAccount,
	CoreSetting,
	/********����������********/
	ServerType,
	ServerID,
	ServerName,
	ServerVersion,
	/********��Ϣ����********/
	PacketParses,
	BuffPacket,
	Message,

/****************alert.xml*****************/
	AlertConnect,
	IP,
	ClientData,
	CommandInfo,
	Mail,
	WorkThreadChart,
	ConnectChart,
	CommandChart,

	/*ö�ٽ���*/
	End
};

enum XmlEnd
{
	MAX = XmlConfig::End
};

class IConfigOpeation;

//MianConfig����
class XMainConfig
{
public:
	bool Init();
	template<class T>
	T* GetXmlConfig(XmlConfig config) { return dynamic_cast<T*>(IConfigOpeation::_array[config]); }
private:
	bool Init(const char* pFileName, XmlConfig start, XmlConfig end);
	CXmlOpeation m_XmlOpeation;
};

typedef ACE_Singleton<XMainConfig, ACE_Null_Mutex> App_XmlConfig;


class IConfigOpeation
{
	friend class XMainConfig;
protected:
	IConfigOpeation(XmlConfig config) { _array[config] = this;}
	virtual bool Init(CXmlOpeation* pXmlOpeation) = 0;
private:
	static IConfigOpeation* _array[XmlEnd::MAX];
};


//�������ݰ������������
class xmlRecvInfo : public IConfigOpeation
{
public:
	uint32 RecvBuffSize;
	uint16 RecvQueueTimeout;
	xmlRecvInfo() : IConfigOpeation(RecvInfo), RecvBuffSize(0), RecvQueueTimeout(0){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

//�������ݰ������������
class xmlSendInfo : public IConfigOpeation
{
public:
	uint16 SendQueueMax;//���Ͷ�����������ݰ�����
	uint16 TcpNodelay;
	uint32 MaxBlockSize;
	uint32 SendDatamark;
	uint32 BlockCount;
	uint16 SendTimeout;
	uint16 SendQueueTimeout;
	uint16 SendQueueCount;
	uint16 PutQueueTimeout;
	xmlSendInfo() : IConfigOpeation(SendInfo), SendQueueMax(0), TcpNodelay(TCP_NODELAY_OFF), MaxBlockSize(0),
		SendDatamark(0), BlockCount(0), SendTimeout(0), SendQueueTimeout(0), SendQueueCount(0), PutQueueTimeout(0){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

//����ģ��
class xmlNetWorkMode : public IConfigOpeation
{
public:
	uint8 Mode;
	uint16 BackLog;
	uint8 ByteOrder;
	xmlNetWorkMode() : IConfigOpeation(NetWorkMode), Mode(0), BackLog(0), ByteOrder(false){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlConnectServer : public IConfigOpeation
{
public:
	uint32 TimeInterval;
	uint32 Recvbuff;
	uint8 RunType;
	uint16 TimeCheck;
	uint16 Count;
	xmlConnectServer() : IConfigOpeation(ConnectServer), TimeInterval(500), Recvbuff(1024), RunType(0), Count(100){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlClientInfo : public IConfigOpeation
{
public:
	uint32 HandlerCount;
	uint32 MaxHandlerCount;
	uint16 MaxConnectTime;
	uint16 CheckAliveTime;
	uint32 MaxBuffRecv;
	xmlClientInfo() : IConfigOpeation(ClientInfo), HandlerCount(5000), MaxHandlerCount(10000), MaxConnectTime(120), 
		CheckAliveTime(60), MaxBuffRecv(1024){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlModuleInfos : public IConfigOpeation
{
public:
	class _ModuleInfo
	{
	public:
		std::string szModuleName;
		std::string szModulePath;
		std::string szModuleParam;
		_ModuleInfo() : szModuleName("TcpTest.dll"), szModulePath("./"), szModuleParam(""){}
	};
	std::vector<_ModuleInfo> vec;
	xmlModuleInfos() : IConfigOpeation(ModuleInfos) {}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlModuleMangager : public IConfigOpeation
{
public:
	uint32 MaxCount;
	xmlModuleMangager() : IConfigOpeation(ModuleMangager), MaxCount(100) {}
	bool Init(CXmlOpeation* pXmlOpeation);
};
class xmlMonitor : public IConfigOpeation
{
public:
	uint8 CpuAndMemory;
	uint16 CpuMax;
	uint32 MemoryMax;
	xmlMonitor() : IConfigOpeation(Monitor), CpuAndMemory(1), CpuMax(50), MemoryMax(1000){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlThreadInfoAI : public IConfigOpeation
{
public:
	uint8 AI;
	uint8 ReturnDataType;
	uint16 CheckTime;
	uint16 TimeoutCount;
	uint16 StopTime;
	std::string ReturnData;
	xmlThreadInfoAI() : IConfigOpeation(ThreadInfoAI), AI(0), ReturnDataType(1), CheckTime(30), 
		TimeoutCount(1), StopTime(30), ReturnData("ff ff"){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlThreadInfo : public IConfigOpeation
{
public:
	uint16 ThreadTimeout;
	uint16 ThreadTimeCheck;
	uint16 DisposeTimeout;
	uint16 PutQueueTimeout;
	xmlThreadInfo() : IConfigOpeation(ThreadInfo), ThreadTimeout(30), ThreadTimeCheck(60), 
		DisposeTimeout(40), PutQueueTimeout(100){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlConsole : public IConfigOpeation
{
public:
	uint8 support;
	std::string sip;
	uint16 sport;
	xmlConsole() : IConfigOpeation(Console), support(1), sip("INADDR_ANY"), sport(10010){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlConsoleKeys : public IConfigOpeation
{
public:
	class _ConsoleKey
	{
	public:
		std::string Key;
		_ConsoleKey() : Key("not"){}
	};
	std::vector<_ConsoleKey> vec;
	xmlConsoleKeys() : IConfigOpeation(ConsoleKey) {}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlAceDebug : public IConfigOpeation
{
public:
	uint8 TrunOn;
	std::string DebugName;
	uint16 ChkInterval;
	uint32 LogFileMaxSize;
	uint8 LogFileMaxCnt;
	std::string Level;
	xmlAceDebug() : IConfigOpeation(AceDebug), TrunOn(0), DebugName("./serverdebug.log"), ChkInterval(600), 
		LogFileMaxSize(10240), LogFileMaxCnt(3), Level(""){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlCommandAccount : public IConfigOpeation
{
public:
	uint8 Account;
	uint8 FlowAccount;
	uint32 MaxCommandCount;
	xmlCommandAccount() : IConfigOpeation(CommandAccount), Account(1), FlowAccount(1), MaxCommandCount(2000) {}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlCoreSetting : public IConfigOpeation
{
public:
	uint8 CoreNeed;
	std::string Script;
	xmlCoreSetting() : IConfigOpeation(CoreSetting), CoreNeed(0), Script(""){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlServerType : public IConfigOpeation
{
public:
	uint8 Type;
	std::string name;
	std::string displayname;
	uint8 Debug;
	uint32 DebugSize;
	uint8 IsClose;
	xmlServerType() : IConfigOpeation(ServerType), Type(0), name("Pss Service"),
		displayname("PssService"), Debug(0), DebugSize(1000), IsClose(0) {}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlServerID : public IConfigOpeation
{
public:
	uint32 id;
	xmlServerID() : IConfigOpeation(ServerID), id(1001) {}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlServerName : public IConfigOpeation
{
public:
	std::string name;
	xmlServerName() : IConfigOpeation(ServerName), name("Freeeyes") {}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlServerVersion : public IConfigOpeation
{
public:
	std::string Version;
	xmlServerVersion() : IConfigOpeation(ServerVersion), Version("1.00") {}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlPacketParses : public IConfigOpeation
{
public:
	class _PacketParse
	{
	public:
		uint16 uParseID;
		std::string szModulePath;
		std::string szModuleName;
		std::string szType;
		uint16 uOrgLength;
		_PacketParse() : uParseID(1), szModulePath("./"), 
			szModuleName("PacketParse_Interface.dll"), szType("STREAM"), uOrgLength(40){}
	};
	std::vector<_PacketParse> _vec;
	xmlPacketParses() : IConfigOpeation(PacketParses){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlBuffPacket : public IConfigOpeation
{
public:
	uint32 Count;
	xmlBuffPacket() : IConfigOpeation(BuffPacket), Count(5000) {}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlMessage : public IConfigOpeation
{
public:
	uint32 Msg_High_mark;
	uint32 Msg_Low_mark;
	uint32 Msg_Buff_Max_Size;
	uint16 Msg_Thread;
	uint32 Msg_MaxQueue;
	xmlMessage() : IConfigOpeation(Message), Msg_High_mark(64000), Msg_Low_mark(64000), Msg_Buff_Max_Size(20480),
		Msg_Thread(5), Msg_MaxQueue(10000){}
	bool Init(CXmlOpeation* pXmlOpeation);
};



/****************************alert.xml*****************************/
class xmlAlertConnect : public IConfigOpeation
{
public:
	uint8 ConnectMin;
	uint32 ConnectMax;
	uint16 DisConnectMin;
	uint32 DisConnectMax;
	uint32 ConnectAlert;
	uint32 MailID;
	xmlAlertConnect() : IConfigOpeation(AlertConnect), ConnectMin(0), ConnectMax(0), DisConnectMin(0),
		DisConnectMax(0), ConnectAlert(20000), MailID(0){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlIP : public IConfigOpeation
{
public:
	uint8 IPMax;
	uint32 Timeout;
	xmlIP() : IConfigOpeation(IP), IPMax(0), Timeout(300){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlClientData : public IConfigOpeation
{
public:
	uint16 RecvPacketCount;
	uint32 RecvDataMax;
	uint16 SendPacketCount;
	uint32 SendDataMax;
	uint32 MailID;
	xmlClientData() : IConfigOpeation(ClientData), RecvPacketCount(0), RecvDataMax(0), SendPacketCount(0),
		SendDataMax(0), MailID(0){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlCommandInfo : public IConfigOpeation
{
public:
	uint32 CommandID;
	uint32 CommandCount;
	uint32 MailID;
	xmlCommandInfo() : IConfigOpeation(CommandInfo), CommandID(4096), CommandCount(0), MailID(0){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlMail : public IConfigOpeation
{
public:
	uint16 MailID;
	std::string fromMailAddr;
	std::string toMailAddr;
	uint32 MailPass;
	std::string MailUrl;
	uint16 MailPort;
	xmlMail() : IConfigOpeation(Mail), MailID(1), fromMailAddr("local@163.com"), toMailAddr("freeeyes@163.com"),
		MailPass(123456), MailUrl("smtp.163.com"), MailPort(25){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlWorkThreadChart : public IConfigOpeation
{
public:
	uint16 JsonOutput;
	uint32 Count;
	std::string File;
	xmlWorkThreadChart() : IConfigOpeation(WorkThreadChart), JsonOutput(1), Count(10),
		File("./Log/WorkThread.json"){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlConnectChart : public IConfigOpeation
{
public:
	uint16 JsonOutput;
	uint32 Count;
	std::string File;
	xmlConnectChart() : IConfigOpeation(ConnectChart), JsonOutput(1), Count(10),
		File("./Log/ConnnectInfo.json"){}
	bool Init(CXmlOpeation* pXmlOpeation);
};

class xmlCommandChart : public IConfigOpeation
{
public:
	uint16 JsonOutput;
	uint32 Count;
	uint32 CommandID;
	std::string File;
	xmlCommandChart() : IConfigOpeation(CommandChart), JsonOutput(1), Count(10), CommandID(4096),
		File("./Log/Command4096.json"){}
	bool Init(CXmlOpeation* pXmlOpeation);
};


/*********************************����Ϊ�ɵ�********************************/

//PacketParse�����Ϣ
class _PacketParseInfo
{
public:
    uint32 m_u4PacketID;
    uint32 m_u4OrgLength;
    uint8  m_u1Type;
    char   m_szPacketParsePath[MAX_BUFF_200];
    char   m_szPacketParseName[MAX_BUFF_100];

    _PacketParseInfo()
    {
        m_u4PacketID           = 0;
        m_szPacketParsePath[0] = '\0';
        m_szPacketParseName[0] = '\0';
        m_u1Type               = (uint8)PACKET_WITHHEAD;
        m_u4OrgLength          = 0;
    }
};

//��������Ϣ
//���Ӷ�IPv4��IPv6��֧��
class _ServerInfo
{
public:
    uint32 m_u4PacketParseInfoID;
    int32  m_nPort;
    uint8  m_u1IPType;
    char   m_szServerIP[MAX_BUFF_50];

    _ServerInfo()
    {
        m_szServerIP[0]       = '\0';
        m_nPort               = 0;
        m_u1IPType            = TYPE_IPV4;
        m_u4PacketParseInfoID = 0;
    }
};

//����������������Ϣ
class _ModuleConfig
{
public:
    char m_szModuleName[MAX_BUFF_100];
    char m_szModulePath[MAX_BUFF_200];
    char m_szModuleParam[MAX_BUFF_200];

    _ModuleConfig()
    {
        m_szModuleName[0]  = '\0';
        m_szModulePath[0]  = '\0';
        m_szModuleParam[0] = '\0';
    }
};

typedef vector<_ModuleConfig> vecModuleConfig;

//Զ�̹�����֧��
//��¼����Զ��ά���ӿڽ��������key�����ݡ�
class _ConsoleKey
{
public:
    char m_szKey[MAX_BUFF_100];

    _ConsoleKey()
    {
        m_szKey[0] = '\0';
    }
};

typedef vector<_ConsoleKey> vecConsoleKey;

//Console�����̨����IP��Ϣ
class _ConsoleClientIP
{
public:
    char m_szServerIP[MAX_BUFF_20];

    _ConsoleClientIP()
    {
        m_szServerIP[0] =  '\0';
    }
};

//���Ӹ澯��ֵ�������
class _ConnectAlert
{
public:
    uint32 m_u4ConnectMin;
    uint32 m_u4ConnectMax;
    uint32 m_u4DisConnectMin;
    uint32 m_u4DisConnectMax;
    uint32 m_u4ConnectAlert;
    uint32 m_u4MailID;

    _ConnectAlert()
    {
        m_u4ConnectMin    = 0;
        m_u4ConnectMax    = 0;
        m_u4DisConnectMin = 0;
        m_u4DisConnectMax = 0;
        m_u4ConnectAlert  = 0;
        m_u4MailID        = 0;
    }
};

//����IP�澯��ֵ�������
class _IPAlert
{
public:
    uint32 m_u4IPMaxCount;
    uint32 m_u4IPTimeout;
    uint32 m_u4MailID;

    _IPAlert()
    {
        m_u4IPMaxCount = 0;
        m_u4IPTimeout  = 0;
        m_u4MailID     = 0;
    }
};

//�����Ӹ澯��ֵ�������
class _ClientDataAlert
{
public:
    uint32 m_u4RecvPacketCount;
    uint32 m_u4RecvDataMax;
    uint32 m_u4SendPacketCount;
    uint32 m_u4SendDataMax;
    uint32 m_u4MailID;

    _ClientDataAlert()
    {
        m_u4RecvPacketCount = 0;
        m_u4RecvDataMax     = 0;
        m_u4SendPacketCount = 0;
        m_u4SendDataMax     = 0;
        m_u4MailID          = 0;
    }
};

//�����и澯��ֵ����
class _CommandAlert
{
public:
    uint32 m_u4CommandCount;
    uint32 m_u4MailID;
    uint16 m_u2CommandID;

    _CommandAlert()
    {
        m_u2CommandID    = 0;
        m_u4CommandCount = 0;
        m_u4MailID       = 0;
    }
};
typedef vector<_CommandAlert> vecCommandAlert;

//�ʼ����������Ϣ
class _MailAlert
{
public:
    uint32 m_u4MailPort;
    uint32 m_u4MailID;
    char   m_szFromMailAddr[MAX_BUFF_200];
    char   m_szToMailAddr[MAX_BUFF_200];
    char   m_szMailPass[MAX_BUFF_200];
    char   m_szMailUrl[MAX_BUFF_200];

    _MailAlert()
    {
        m_u4MailID          = 0;
        m_szFromMailAddr[0] = '\0';
        m_szToMailAddr[0]   = '\0';
        m_szMailPass[0]     = '\0';
        m_szMailUrl[0]      = '\0';
        m_u4MailPort        = 0;
    }
};
typedef vector<_MailAlert> vecMailAlert;

//��Ⱥ���������Ϣ
class _GroupListenInfo
{
public:
    uint32 m_u4GroupPort;
    uint8  m_u1GroupNeed;
    uint8  m_u1IPType;
    char   m_szGroupIP[MAX_BUFF_50];

    _GroupListenInfo()
    {
        m_u1GroupNeed  = 0;
        m_szGroupIP[0] = '\0';
        m_u4GroupPort  = 0;
        m_u1IPType     = TYPE_IPV4;
    }
};

//���ͼ����Ϣ
class _ChartInfo
{
public:
    bool   m_blJsonOutput;
    uint16 m_u2Count;
    uint32 m_u4CommandID;
    char   m_szJsonFile[MAX_BUFF_200];

    _ChartInfo()
    {
        m_blJsonOutput  = false;
        m_u2Count       = 0;
        m_u4CommandID   = 0;
        m_szJsonFile[0] = '\0';
    }
};

enum ENUM_CHAR_ORDER
{
    SYSTEM_LITTLE_ORDER = 0,   //С������
    SYSTEM_BIG_ORDER,          //�������
};

class CMainConfig
{
public:
    CMainConfig(void);
    ~CMainConfig(void);

    bool Init();
    bool Init_Alert(const char* szConfigPath);
    bool Init_Main(const char* szConfigPath);
    void Display();
    bool CheckAllIP();
    const char* GetError();

    const char* GetServerName();
    const char* GetServerVersion();
    const char* GetPacketVersion();
    const char* GetWindowsServiceName();
    const char* GetDisplayServiceName();
    uint16 GetServerID();
    uint16 GetServerPortCount();
    _ServerInfo* GetServerPort(int32 nIndex);

    uint32 GetMgsHighMark();
    uint32 GetMsgLowMark();
    uint32 GetMsgMaxBuffSize();
    uint32 GetThreadCount();
    uint8  GetProcessCount();
    uint32 GetMsgMaxQueue();
    uint16 GetHandleCount();

    int32 GetEncryptFlag();
    const char* GetEncryptPass();
    int32 GetEncryptOutFlag();

    uint32 GetSendTimeout();

    uint32 GetRecvBuffSize();
    uint16 GetSendQueueMax();
    uint16 GetThreadTimuOut();
    uint16 GetThreadTimeCheck();
    uint16 GetPacketTimeOut();
    uint16 GetCheckAliveTime();
    uint16 GetMaxHandlerCount();
    void   SetMaxHandlerCount(uint16 u2MaxHandlerCount);
    uint16 GetMaxConnectTime();
    uint8  GetConsoleSupport();
    int32  GetConsolePort();
    uint8  GetConsoleIPType();
    const char* GetConsoleIP();
    vecConsoleKey* GetConsoleKey();
    uint16 GetRecvQueueTimeout();
    uint16 GetSendQueueTimeout();
    uint16 GetSendQueueCount();

    bool CompareConsoleClinetIP(const char* pConsoleClientIP);

    _ServerInfo* GetUDPServerPort(int32 nIndex);

    uint16 GetUDPServerPortCount();
    uint32 GetReactorCount();
    uint8  GetCommandAccount();
    uint32 GetConnectServerTimeout();
    uint16 GetConnectServerCheck();
    uint8  GetConnectServerRunType();
    uint16 GetSendQueuePutTime();
    uint16 GetWorkQueuePutTime();
    uint8  GetServerType();
    uint8  GetDebug();
    uint32 GetDebugSize();
    void   SetDebug(uint8 u1Debug);
    uint8  GetNetworkMode();
    uint32 GetConnectServerRecvBuffer();
    uint8  GetMonitor();
    uint32 GetServerRecvBuff();
    uint8  GetCommandFlow();
    uint32 GetSendDataMask();
    uint32 GetCoreFileSize();
    uint16 GetTcpNodelay();
    uint16 GetBacklog();
    uint16 GetTrackIPCount();
    ENUM_CHAR_ORDER GetCharOrder();
    uint32 GetCpuMax();
    uint32 GetMemoryMax();
    uint8  GetWTAI();
    uint32 GetWTCheckTime();
    uint32 GetWTTimeoutCount();
    uint32 GetWTStopTime();
    uint8  GetWTReturnDataType();
    char*  GetWTReturnData();
    bool   GetByteOrder();
    uint8  GetDebugTrunOn();
    char*  GetDebugFileName();
    uint32 GetChkInterval();
    uint32 GetLogFileMaxSize();
    uint32 GetLogFileMaxCnt();
    char*  GetDebugLevel();
    uint32 GetBlockSize();
    uint32 GetBlockCount();
    uint8  GetServerClose();
    uint32 GetMaxCommandCount();
    uint32 GetServerConnectCount();
    uint16 GetMaxModuleCount();

    uint16 GetModuleInfoCount();
    _ModuleConfig* GetModuleInfo(uint16 u2Index);

    _ConnectAlert*    GetConnectAlert();
    _IPAlert*         GetIPAlert();
    _ClientDataAlert* GetClientDataAlert();
    uint32            GetCommandAlertCount();
    _CommandAlert*    GetCommandAlert(int32 nIndex);
    _MailAlert*       GetMailAlert(uint32 u4MailID);
    _GroupListenInfo* GetGroupListenInfo();
    _PacketParseInfo* GetPacketParseInfo(uint8 u1Index = 0);
    uint8             GetPacketParseCount();
    char*             GetCoreScript();
    _ChartInfo*       GetWorkThreadChart();
    _ChartInfo*       GetConnectChart();
    uint32            GetCommandChartCount();
    _ChartInfo*       GetCommandChart(uint32 u4Index);
    uint32            GetBuffPacketPoolCount();

private:
    uint32     m_u4MsgHighMark;                        //��Ϣ�ĸ�ˮλ��ֵ
    uint32     m_u4MsgLowMark;                         //��Ϣ�ĵ�ˮλ��ֵ
    uint32     m_u4MsgMaxBuffSize;                     //��Ϣ������С
    uint32     m_u4MsgThreadCount;                     //����Ĺ����̸߳���
    uint32     m_u4MsgMaxQueue;                        //��Ϣ���е�������
    uint32     m_u4DebugSize;                          //���õ�ǰ��¼���ݰ����ȵ���󻺳��С
    uint32     m_u4SendTimeout;                        //���ͳ�ʱʱ��
    uint32     m_u4RecvBuffSize;                       //�������ݻ���صĴ�С
    uint32     m_u4ServerConnectCount;                 //�����������ӻ���������
    uint32     m_u4MaxCommandCount;                    //��ǰ��������������
    uint32     m_u4ReactorCount;                       //ϵͳ�������ķ�Ӧ���ĸ���
    uint32     m_u4ConnectServerTimerout;              //����Զ�̷��������ʱ��
    uint32     m_u4ConnectServerRecvBuff;              //������������ݰ����ջ����С
    uint32     m_u4ServerRecvBuff;                     //���մӿͻ��˵�������ݿ������С��ֻ��PacketPrase��ģʽ�Ż���Ч
    uint32     m_u4SendDatamark;                       //���Ͳ�ֵ��ˮλ�꣨Ŀǰֻ��Proactorģʽ�������
    uint32     m_u4BlockSize;                          //���ͻ�����С����
    uint32     m_u4CoreFileSize;                       //Core�ļ��ĳߴ��С
    uint32     m_u4TrackIPCount;                       //���IP�������ʷ��¼��
    uint32     m_u4SendBlockCount;                     //��ʼ�����ͻ������
    uint32     m_u4MaxCpu;                             //���CPU����߷�ֵ
    uint32     m_u4MaxMemory;                          //����ڴ�ķ�ֵ
    uint32     m_u4WTCheckTime;                        //�����̳߳�ʱ����ʱ�䷶Χ����λ����
    uint32     m_u4WTTimeoutCount;                     //�����̳߳�ʱ���ĵ�λʱ���ڵĳ�ʱ��������
    uint32     m_u4WTStopTime;                         //ֹͣ����������ʱ��
    uint32     m_u4ChkInterval;                        //����ļ�ʱ��
    uint32     m_u4LogFileMaxSize;                     //����ļ����ߴ�
    uint32     m_u4LogFileMaxCnt;                      //����ļ������������ﵽ�������Զ�ѭ��
    uint32     m_u4BuffPacketPoolCount;                //BuffPacket�ڴ�ش�С
    int32      m_nServerID;                            //������ID
    int32      m_nEncryptFlag;                         //0�����ܷ�ʽ�رգ�1Ϊ���ܷ�ʽ����
    int32      m_nEncryptOutFlag;                      //��Ӧ���ݰ���0��Ϊ�����ܣ�1Ϊ����
    int32      m_nConsolePort;                         //Console�������Ķ˿�
    uint16     m_u2SendQueueMax;                       //���Ͷ�����������ݰ�����
    uint16     m_u2ThreadTimuOut;                      //�̳߳�ʱʱ���ж�
    uint16     m_u2ThreadTimeCheck;                    //�߳��Լ�ʱ��
    uint16     m_u2PacketTimeOut;                      //�������ݳ�ʱʱ��
    uint16     m_u2SendAliveTime;                      //���ʹ�����ʱ��
    uint16     m_u2HandleCount;                        //handle����صĸ���
    uint16     m_u2MaxHanderCount;                     //���ͬʱ����Handler������
    uint16     m_u2MaxConnectTime;                     //��ȴ���������ʱ�䣨��ʱ���ڣ�������պͷ��Ͷ�û�з��������ɷ������ر�������ӣ�
    uint16     m_u2RecvQueueTimeout;                   //���ն��д���ʱʱ���޶�
    uint16     m_u2SendQueueTimeout;                   //���Ͷ��д���ʱʱ���޶�
    uint16     m_u2SendQueueCount;                     //��ܷ����߳���
    uint16     m_u2SendQueuePutTime;                   //���÷��Ͷ��е���ӳ�ʱʱ��
    uint16     m_u2WorkQueuePutTime;                   //���ù������е���ӳ�ʱʱ��
    uint16     m_u2MaxModuleCount;                     //��ǰ�����������ģ������
    uint16     m_u2ConnectServerCheck;                 //�����������ӵ�λ���ʱ��
    uint16     m_u2TcpNodelay;                         //TCP��Nagle�㷨���أ�0Ϊ�򿪣�1Ϊ�ر�
    uint16     m_u2Backlog;                            //���õ�Backlogֵ
    bool       m_blByteOrder;                          //��ǰ���ʹ������falseΪ������trueΪ������
    uint8      m_u1MsgProcessCount;                    //��ǰ�Ķ��������(��Linux֧��)
    uint8      m_u1Debug;                              //�Ƿ���Debugģʽ��1�ǿ�����0�ǹر�
    uint8      m_u1ServerClose;                        //�������Ƿ�����Զ�̹ر�
    uint8      m_u1CommandAccount;                     //�Ƿ���Ҫͳ������������������Ϣ��0�ǹرգ�1�Ǵ򿪡��򿪺��������Ӧ�ı���
    uint8      m_u1ServerType;                         //���÷���������״̬
    uint8      m_u1ConsoleSupport;                     //�Ƿ�֧��Console���������1����֧�֣�0�ǲ�֧��
    uint8      m_u1ConsoleIPType;                      //Console��IPType
    uint8      m_u1CommandFlow;                        //�������ͳ�ƣ�0Ϊ��ͳ�ƣ�1Ϊͳ��
    uint8      m_u1ConnectServerRunType;               //�������䷵�ذ�����ģʽ��0Ϊͬ����1Ϊ�첽
    uint8      m_u1NetworkMode;                        //��ǰ�������õ�����ģʽ
    uint8      m_u1Monitor;                            //���õ�ǰ�ļ�ؿ����Ƿ�򿪣�0�ǹرգ�1�Ǵ�
    uint8      m_u1WTAI;                               //�����߳�AI���أ�0Ϊ�رգ�1Ϊ��
    uint8      m_u1WTReturnDataType;                   //���ش������ݵ����ͣ�1Ϊ�����ƣ�2Ϊ�ı�
    uint8      m_u1DebugTrunOn;                        //ACE_DEBUG�ļ�������أ�0Ϊ�رգ�1Ϊ��


    char       m_szError[MAX_BUFF_500];
    char       m_szServerName[MAX_BUFF_20];            //����������
    char       m_szServerVersion[MAX_BUFF_20];         //�������汾
    char       m_szWindowsServiceName[MAX_BUFF_50];    //windows��������
    char       m_szDisplayServiceName[MAX_BUFF_50];    //windows������ʾ����
    char       m_szPacketVersion[MAX_BUFF_20];         //���ݽ�����ģ��İ汾��
    char       m_szEncryptPass[MAX_BUFF_9];            //�8λ�ļ������룬3DES�㷨
    char       m_szConsoleIP[MAX_BUFF_100];            //Console������IP
    char       m_szWTReturnData[MAX_BUFF_1024];        //���ص������壬���1K
    char       m_szDeubgFileName[MAX_BUFF_100];        //����ļ���
    char       m_szDebugLevel[MAX_BUFF_100];           //����ļ�����
    char       m_szCoreScript[MAX_BUFF_200];           //������Core����ʱ�򣬵��õ�ִ�нű�λ��

    CXmlOpeation     m_MainConfig;
    _ConnectAlert    m_ConnectAlert;                   //���Ӹ澯���������Ϣ
    _IPAlert         m_IPAlert;                        //IP�澯��ֵ�������
    _ClientDataAlert m_ClientDataAlert;                //�����ӿͻ��˸澯��ֵ�������
    _GroupListenInfo m_GroupListenInfo;                //��Ⱥ��ط�������ַ����
    _ChartInfo       m_WorkThreadChart;                //�����߳�ͼ����Ϣ
    _ChartInfo       m_ConnectChart;                   //�ͻ�������ͼ����Ϣ

    typedef vector<_ChartInfo> vecCommandChart;       //����ͼ����Ϣ
    vecCommandChart m_vecCommandChart;                //����ͼ����Ϣ

    typedef vector<_PacketParseInfo> vecPacketParseInfo;
    vecPacketParseInfo m_vecPacketParseInfo;

    ENUM_CHAR_ORDER m_u1CharOrder;                 //��ǰ�ֽ���

    typedef vector<_ServerInfo> vecServerInfo;
    vecServerInfo m_vecServerInfo;
    vecServerInfo m_vecUDPServerInfo;

    typedef vector<_ConsoleClientIP> vecConsoleClientIP;
    vecConsoleClientIP m_vecConsoleClientIP;                  //��������̨�����IP
    vecConsoleKey      m_vecConsoleKey;                       //�����������keyֵ
    vecCommandAlert    m_vecCommandAlert;                     //�����и澯��ֵ�������
    vecMailAlert       m_vecMailAlert;                        //�����ʼ��������
    vecModuleConfig    m_vecModuleConfig;                     //����ģ����������Ϣ
};

typedef ACE_Singleton<CMainConfig, ACE_Null_Mutex> App_MainConfig;

#endif


