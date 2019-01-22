#include "ReTTyHandler.h"

CReTTyHandler::CReTTyHandler() : m_blState(false), m_blPause(false), m_pTTyMessage(NULL)
{
    m_szName[0]   = 0;
    m_pmbReadBuff = new ACE_Message_Block(MAX_BUFF_1024);
}

CReTTyHandler::~CReTTyHandler()
{
    OUR_DEBUG((LM_INFO, "[CReTTyHandler::~CReTTyHandler].\n"));
    Close();

    //������Դ
    SAFE_DELETE(m_pTTyMessage);
    SAFE_DELETE(m_pmbReadBuff);
}

bool CReTTyHandler::ConnectTTy()
{
    if (true == m_blState)
    {
        //����豸�Ѿ�������״̬��ʲôҲ����
        return true;
    }

    //�����豸������
    if (m_ReConnector.connect(m_ReTtyio, ACE_DEV_Addr(m_szName), 0, ACE_Addr::sap_any, 0, O_RDWR | FILE_FLAG_OVERLAPPED) == -1)
    {
        OUR_DEBUG((LM_INFO, "[CReTTyHandler::ConnectTTy]m_Connector.connect(%s) fail.\n", m_szName));
        return false;
    }

    //�����豸����
    if (m_ReTtyio.control(ACE_TTY_IO::SETPARAMS, &m_ObjParams) == -1)
    {
        OUR_DEBUG((LM_INFO, "[CReTTyHandler::ConnectTTy]m_Ttyio SETPARAMS(%s) fail.\n", m_szName));
        return false;
    }

    if (this->reactor()->register_handler(m_ReTtyio.get_handle(), this, ACE_Event_Handler::READ_MASK) == -1)
    {
        OUR_DEBUG((LM_ERROR, "[CReTTyHandler::ConnectTTy](%s) Could not register with reactor for %p\n", m_szName));
        return false;
    }

    m_blState = true;
    return true;
}

void CReTTyHandler::Close()
{
    if (true == m_blState)
    {
        m_ReTtyio.close();
        m_blState = false;
    }
}

bool CReTTyHandler::Init(uint32 u4ConnectID, const char* pName, ACE_TTY_IO::Serial_Params inParams, ITTyMessage* pTTyMessage)
{
    m_u4ConnectID = u4ConnectID;
    sprintf_safe(m_szName, MAX_BUFF_100, "%s", pName);
    m_pTTyMessage = pTTyMessage;
    m_ObjParams = inParams;

    //��ʼ�������豸
    return ConnectTTy();
}

bool CReTTyHandler::GetConnectState()
{
    return m_blState;
}

ACE_VERSIONED_NAMESPACE_NAME::ACE_TTY_IO::Serial_Params CReTTyHandler::GetParams()
{
    return m_ObjParams;
}

void CReTTyHandler::SetPause(bool blPause)
{
    m_blPause = blPause;
}

bool CReTTyHandler::GetPause()
{
    return m_blPause;
}

int CReTTyHandler::handle_input(ACE_HANDLE handle)
{
    ACE_UNUSED_ARG(handle);

    m_pmbReadBuff->reset();

    ssize_t bytes_read = m_ReTtyio.recv((void*)m_pmbReadBuff->wr_ptr(), MAX_BUFF_1024);

    if (bytes_read <= 0)
    {
        //�����豸�����쳣
        OUR_DEBUG((LM_ERROR, "[CReTTyHandler::handle_input]Error:%d.\n", (int)errno));

        //֪ͨ�ϲ�Ӧ��
        m_pTTyMessage->ReportMessage(m_u4ConnectID, (uint32)errno, EM_TTY_EVENT_RW_ERROR);

        //�Ͽ���ǰ�豸
        Close();
    }
    else
    {
        if (NULL != m_pTTyMessage && true == m_blPause)
        {
            //�ص��������ݺ���
            m_pTTyMessage->RecvData(m_u4ConnectID, m_pmbReadBuff->rd_ptr(), (uint32)bytes_read);
        }
    }

    return 0;
}

int CReTTyHandler::handle_signal(int signum, siginfo_t* psinfo, ucontext_t* context)
{
    ACE_UNUSED_ARG(psinfo);
    ACE_UNUSED_ARG(context);

    OUR_DEBUG((LM_ERROR, "[CReTTyHandler::handle_signal]Error:%d.\n", (int)signum));
    return 0;
}

int CReTTyHandler::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
    ACE_UNUSED_ARG(handle);
    ACE_UNUSED_ARG(close_mask);

    OUR_DEBUG((LM_ERROR, "[CReTTyHandler::handle_close]Error:%d.\n", (int)errno));
    return 0;
}

int CReTTyHandler::handle_timeout(const ACE_Time_Value& tvNow, const void*  p)
{
    ACE_UNUSED_ARG(tvNow);
    ACE_UNUSED_ARG(p);

    OUR_DEBUG((LM_ERROR, "[CReTTyHandler::handle_timeout]Run.\n"));
    return 0;
}

bool CReTTyHandler::Send_Data(const char* pData, ssize_t nLen)
{
    //��������ѶϿ������ﳢ������
    ConnectTTy();

    if (true == m_blState && true == m_blPause)
    {
        if (nLen != m_ReTtyio.send_n(pData, nLen))
        {
            //��������ʧ��
            m_pTTyMessage->ReportMessage(m_u4ConnectID, (uint32)errno, EM_TTY_EVENT_RW_ERROR);

            //�ж��豸
            Close();
        }

        return true;
    }
    else
    {
        //��ǰ�����жϣ��޷���������
        return false;
    }
}

