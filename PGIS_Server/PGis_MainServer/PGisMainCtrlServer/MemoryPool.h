/*********************************************************************
* Copyright (c) 2008, HYT
* All rights reserved.
* 
* �ļ����ƣ�AudioCodec.h
* �ļ���ʶ��interface for the AudioCodec class.
* ժ    Ҫ���ڴ��/FIFO�ķ�װ��
*
* ��ʷ�޸ļ�¼��
*   ����            ����            �汾            ����
*
*********************************************************************/

#ifndef __MEMORYPOOL_H_INCLUDED_
#define __MEMORYPOOL_H_INCLUDED_

//////////////////////////////////////////////////////////////////////
// Macro/Struct definition
//////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
//���ζ��ж���
struct BUFFER_TYPE 
{
	unsigned int buffer_size; //���������ֽ�Ϊ��λ�Ĵ�С
	char *pointer;     //�������ڴ��׵�ַ
	unsigned short message_num; //��Ϣ����
	unsigned int head;        //����ͷԪ�ؾ��뻺�����ڴ��׵�ַ���ֽ�ƫ��
	unsigned int tail;        //����βԪ�ؾ��뻺�����ڴ��׵�ַ���ֽ�ƫ��
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////
// Class definition
//////////////////////////////////////////////////////////////////////
class CMemoryPool 
{
public:
    CMemoryPool();
    virtual ~CMemoryPool(){};

    BOOL InitBuffer(unsigned int size );
    void FreeBuf();
    void ClearBuf();
    BOOL SendToBuffer( char * msg, unsigned short length );
    unsigned char InsertToBuffer(  char * msg, unsigned short length );
    unsigned int GetFromBuffer( char *  msg);
    unsigned int GetMsgLenFromBuffer();

    unsigned GetMessageNum();
    unsigned char GetOccupyPercent(); //ȡ��ռ�ðٷֱ�

protected:
    void RoundInc( unsigned int *c, unsigned int round );
    void RoundDec( unsigned int *c, unsigned int round );

protected:
    struct BUFFER_TYPE m_BufPool;
    CRITICAL_SECTION  m_CriSec;   //�û��������ٽ���
};

#endif