#include "packetutilityv3.h"

PacketEncoderFixedV2::PacketEncoderFixedV2()
{
    m_buf.reserve(128);
}

char *PacketEncoderFixedV2::data()
{
    return m_buf.data();
}

void PacketEncoderFixedV2::setUAVID(int id)
{
    m_id = id;
}

void PacketEncoderFixedV2::setUAVType(int type)
{
    m_type = type;
}

void PacketEncoderFixedV2::setRedOrBlue(quint8 type)
{
    m_redOrBlue = type;
}

PacketDecoderFixedV2::PacketDecoderFixedV2(const char *buf)
{
    m_buf = buf;
}

int PacketDecoderFixedV2::UAVID()
{
    return *reinterpret_cast<const quint8*>(m_buf+5);
}

int PacketDecoderFixedV2::UAVType()
{
    return *reinterpret_cast<const quint8*>(m_buf+6);
}

quint8 PacketDecoderFixedV2::redOrBlue()
{
    return *reinterpret_cast<const quint8*>(m_buf+7);
}

PacketEncoderVariableV2::PacketEncoderVariableV2()
{
    m_buf.reserve(1024);
}

void PacketEncoderVariableV2::setUAVID(int id)
{
    m_id = id;
}

void PacketEncoderVariableV2::setUAVType(int type)
{
    m_type = type;
}

void PacketEncoderVariableV2::setRedOrBlue(quint8 type)
{
    m_redOrBlue = type;
}

PacketDecoderVariableV2::PacketDecoderVariableV2(const char *buf)
{
    m_buf = buf;
}

int PacketDecoderVariableV2::UAVID()
{
    return *reinterpret_cast<const quint8*>(m_buf+5);
}

int PacketDecoderVariableV2::UAVType()
{
    return *reinterpret_cast<const quint8*>(m_buf+6);
}

quint8 PacketDecoderVariableV2::redOrBlue()
{
    return *reinterpret_cast<const quint8*>(m_buf+7);
}
