#ifndef _PROTTYCLIENTMANAGER_H
#define _PROTTYCLIENTMANAGER_H

//TTy�豸��Proactorʵ��
//add by freeeyes

#include "ITTyClientManager.h"
#include "ReTTyHandler.h"
#include "HashTable.h"
#include "XmlConfig.h"

class CReTTyClientManager : public ITTyClientManager
{
public:
    CReTTyClientManager();
    virtual ~CReTTyClientManager();

    bool Init(ACE_Reactor* pReactor, uint16 u2MaxTTyCount);                                            //��ʼ��������
    void Close();                                                                                        //�ر���������

    virtual int Connect(uint16 u2ConnectID, const char* pName, _TTyDevParam& inParam, ITTyMessage* pMessageRecv);  // ���ӣ��򿪣��˿�

    virtual bool GetClientDevInfo(uint16 u2ConnectID, _TTyDevParam& outParam);                           // ��ȡ����������Ϣ
    virtual bool IsConnect(uint16 u2ConnectID);                                                          // �Ƿ����ӣ��򿪣�״̬

    virtual bool Close(uint16 u2ConnectID);                                                              // �رն˿�
    virtual bool Pause(uint16 u2ConnectID);                                                              // ��ͣ�˿�
    virtual bool Resume(uint16 u2ConnectID);                                                             // �ָ���ͣ�˿�

    virtual bool SendMessage(uint16 u2ConnectID, char*& pMessage, uint32 u4Len);                         // ��������
private:
    CHashTable<CReTTyHandler> m_objTTyClientHandlerList;            //�����豸�б�
    ACE_Recursive_Thread_Mutex m_ThreadWritrLock;                    //�߳���
    ACE_Reactor*               m_pReactor;                          //��Ӧ�����
    uint16                     m_u2MaxListCount;                     //����豸����
};

typedef ACE_Singleton<CReTTyClientManager, ACE_Recursive_Thread_Mutex> App_ReTTyClientManager;


#endif
