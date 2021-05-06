#ifndef PACKETUTILITYV2_H
#define PACKETUTILITYV2_H

#include "scheduler_global.h"
#include <QObject>
extern quint8 g_loopCount;

inline quint8 CRC8(const char *buf, int length)
{
    quint8 sum = 0;
    unsigned int nCrc = 0;
    for (int i = 0; i < length; i++) {
        nCrc += (unsigned char)buf[i];
    }
    sum = nCrc & 0xFF;
    return sum;
}

/*!
 * \brief 定长组包工具 移植张松ZrDDSNetwork的算法
 * \details 组包方式为Header + FixedStruct + Tail
 * \note 该类多了一个红蓝方接口
 */
class SCHEDULERSHARED_EXPORT PacketEncoderFixedV2{
public:
    PacketEncoderFixedV2();
    char *data();                      ///<获取内存数据
    unsigned int size();               ///<数据长度
    void setUAVID(int id);             ///<设置无人机ID
    void setUAVType(int type);         ///<设置无人机类型
    void setRedOrBlue(quint8 type);    ///<设置红蓝方
    template<class T>
    void setFixed(T *st);              ///<压入一个结构体

private:
    QByteArray m_buf;            ///<数据首地址
    int m_id;
    int m_type;
    quint8 m_redOrBlue;
};

/*!
 * \brief 定长解包工具 移植张松ZrDDSNetwork的算法
 * \details 解包方式为Header + FixedStruct + Tail
 * \note 该类多了一个红蓝方接口
 */
class SCHEDULERSHARED_EXPORT PacketDecoderFixedV2{
public:
    PacketDecoderFixedV2(const char* buf);
    int UAVID();             ///<获取无人机ID
    int UAVType();           ///<获取无人机类型
    quint8 redOrBlue();      ///<红蓝方
    template<class T>
    const T *getFixed(); ///<取出一个结构体
private:
    const char *m_buf;  ///<数据首地址
};

/*!
 * \brief 变长组包工具 移植张松ZrDDSNetwork的算法
 * \details 组包方式为Header + ArrayHeader + FixedStruct + StructArray[N] + Tail
 * \warning 如果不是使用变长数据，请不要使用该类，该类认定@setVariable为开始组包标志
 * \note 该类多了一个红蓝方接口
 */
class SCHEDULERSHARED_EXPORT PacketEncoderVariableV2{
public:
    PacketEncoderVariableV2();
    char *data();                        ///<获取内存数据
    unsigned int size();                 ///<数据长度
    void setUAVID(int id);               ///<设置无人机ID
    void setUAVType(int type);           ///<设置无人机类型
    void setRedOrBlue(quint8 type);      ///<设置红蓝方
    template<class T>
    void setFixed(T *st);                ///<压入一个定长结构体
    template<class T>
    void setVariable(T st[], int count); ///<压入一个结构体数组

private:
    QByteArray m_buf;            ///<数据首地址
    int m_id;
    int m_type;
    quint8 m_redOrBlue;
};

/*!
 * \brief 变长解包工具 移植张松ZrDDSNetwork的算法
 * \details 解包方式为Header + ArrayHeader + FixedStruct + StructArray[N] + Tail
 * \note 该类多了一个红蓝方接口
 */
class SCHEDULERSHARED_EXPORT PacketDecoderVariableV2{
public:
    PacketDecoderVariableV2(const char* buf);
    int UAVID();         ///<获取无人机ID
    int UAVType();       ///<获取无人机类型
    quint8 redOrBlue();  ///<红蓝方
    template<class T>
    const T *getFixed();    ///<取出一个结构体
    template<class T>
    const QPair<int, const T*> getVariable(); ///<取出一个结构体数组
private:
    const char *m_buf;            ///<数据首地址
};


template<class T>
void PacketEncoderFixedV2::setFixed(T *st)
{
    // Header(9byte) = fixValue(0) + fixValue(1) + pktLen(2) + loopCount(4) + UAVID(5) + UAVType(6) + RedOrBlue(7) + pktType(8)
    // Tail(1byte) = crcSum(1)
    auto pktLen = 9 + sizeof (T) + 1;
    // Header
    m_buf.append(9, 0);
    m_buf[0] = 0xAA;          //fixValue
    m_buf[1] = 0x55;          //fixValue
    m_buf[2] = pktLen;        //pktLen
    m_buf[4] = 0; //loopCount
    m_buf[5] = m_id;          //UAVID
    m_buf[6] = m_type;        //UAVType
    m_buf[7] = m_redOrBlue;   //RedOrBlue
    m_buf[8] = 1;             //pktType
    // T
    m_buf.append(st, sizeof(T));
    // Tail
    m_buf.append(CRC8(m_buf.data(), m_buf.size() - 1));
}

template<class T>
const T *PacketDecoderFixedV2::getFixed()
{
    return reinterpret_cast<const T*>(m_buf+9);
}

template<class T>
void PacketEncoderVariableV2::setFixed(T *st)
{
    m_buf.append(9, 0); //Header
    m_buf.append(5, 0); //ArrayHeader
    // T
    m_buf.append(st, sizeof (T));
}

template<class T>
void PacketEncoderVariableV2::setVariable(T st[], int count)
{
    int fixedSize = m_buf.size() - 9 - 5;
    auto pktLen = m_buf.size() + count * sizeof (T) + 1;

    // T[]
    m_buf.append(st, count + sizeof (T));

    // Header(8byte) = fixValue(0) + fixValue(1) + pktLen(2) + loopCount(4) + UAVID(5) + UAVType(6) + RedOrBlue(7) + pktType(8)
    // ArrayHeader(5byte) = publiLen(0) + onArrayLen(1) + arrayCountAll(2) + arrayCountInPkt(3) + startArrayIndexInPkt(4);
    //Header
    m_buf[0] = 0xAA;          //fixValue
    m_buf[1] = 0x55;          //fixValue
    m_buf[2] = pktLen;        //pktLen
    m_buf[4] = 0; //loopCount
    m_buf[5] = m_id;          //UAVID
    m_buf[6] = m_redOrBlue;   //RedOrBlue
    m_buf[7] = m_type;        //UAVType
    m_buf[8] = 1;             //pktType
    //ArrayHeader
    m_buf[9] = fixedSize;     //publicLen
    m_buf[10] = sizeof (T);    //oneArrayLen
    m_buf[11] = count;        //arrayCountAll
    m_buf[12] = count;        //arrayCountInPkt
    m_buf[13] = 0;            //startArrayIndexInPkt
    //Tail
    m_buf.append(CRC8(m_buf.data(), m_buf.size() - 1));
}

template<class T>
const T *PacketDecoderVariableV2::getFixed()
{
    if(*reinterpret_cast<const quint8*>(m_buf+8) == 0)
        return nullptr;
    else
        return reinterpret_cast<const T*>(m_buf+8+5);
}

template<class T>
const QPair<int, const T *> PacketDecoderVariableV2::getVariable()
{
    int publicLen = *reinterpret_cast<const quint8*>(m_buf+8);
    QPair<int, const T*> ret;
    ret.first = *reinterpret_cast<const quint8*>(m_buf+8+2);
    ret.second = reinterpret_cast<const T*>(m_buf+8+5+publicLen);
    return ret;
}
#endif // PACKETUTILITYV2_H
