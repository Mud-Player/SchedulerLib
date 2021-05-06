#include "packetutilityv2.h"

quint8 g_loopCount = 0;

PacketEncoderFixed::PacketEncoderFixed()
{
    m_buf.reserve(128);
}

char *PacketEncoderFixed::data()
{
    return m_buf.data();
}

unsigned int PacketEncoderFixed::size()
{
    return m_buf.size();
}

void PacketEncoderFixed::setUAVID(int id)
{
    m_id = id;
}

void PacketEncoderFixed::setUAVType(int type)
{
    m_type = type;
}

PacketDecoderFixed::PacketDecoderFixed(const char *buf)
{
    m_buf = buf;
}

int PacketDecoderFixed::UAVID()
{
    return *reinterpret_cast<const quint8*>(m_buf+5);
}

int PacketDecoderFixed::UAVType()
{
    return *reinterpret_cast<const quint8*>(m_buf+6);
}

PacketEncoderVariable::PacketEncoderVariable()
{
    m_buf.reserve(1024);
}

unsigned int PacketEncoderVariable::size()
{
    return m_buf.size();
}

void PacketEncoderVariable::setUAVID(int id)
{
    m_id = id;
}

void PacketEncoderVariable::setUAVType(int type)
{
    m_type = type;
}

PacketDecoderVariable::PacketDecoderVariable(const char *buf)
{
    m_buf = buf;
}

int PacketDecoderVariable::UAVID()
{
    return *reinterpret_cast<const quint8*>(m_buf+5);
}

int PacketDecoderVariable::UAVType()
{
    return *reinterpret_cast<const quint8*>(m_buf+6);
}
