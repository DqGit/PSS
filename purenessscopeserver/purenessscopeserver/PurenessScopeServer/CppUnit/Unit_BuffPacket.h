#ifndef UNIT_BUFFPACKET_H
#define UNIT_BUFFPACKET_H

#ifdef _CPPUNIT_TEST

#include "Unit_Common.h"
#include "BuffPacket.h"

class CUnit_BuffPacket : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CUnit_BuffPacket);
    CPPUNIT_TEST(Read_Write_BuffPacket);
    CPPUNIT_TEST(Read_Write_String_BuffPacket);
    CPPUNIT_TEST(Read_Write_Binary_BuffPacket);
    CPPUNIT_TEST(Check_Size_BuffPacket);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual ~CUnit_BuffPacket();

    virtual void setUp(void);

    virtual void tearDown(void);

    void Read_Write_BuffPacket(void);          //��д����
    void Read_Write_String_BuffPacket(void);   //��д�ַ�������
    void Read_Write_Binary_BuffPacket(void);   //��д�����Ʋ���
    void Check_Size_BuffPacket(void);          //��ض���ߴ�Ĳ���

private:
    CBuffPacket* m_pBuffPacket;
};

#endif

#endif
