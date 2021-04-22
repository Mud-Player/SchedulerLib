#include "dtunetwork.h"
#include <QNetworkDatagram>
#include <QDebug>
#include "configcenter.h"

int DTUDatagram::localPort() const
{
    return m_localPort;
}

void DTUDatagram::setLocalPort(int port)
{
    m_localPort = port;
}

const int &DTUDatagram::peerPort() const
{
    return m_peerPort;
}

void DTUDatagram::setPeerPort(int localPort)
{
    m_peerPort = localPort;
}

const QByteArray &DTUDatagram::data() const
{
    return m_data;
}

void DTUDatagram::setData(const QByteArray &payload)
{
    m_data = payload;
}

DTUSocket::DTUSocket(QHostAddress &peerHost, int peerPort, int localPort, DTUNetwork *parent):QObject(parent),
    m_boss(parent),
    m_parseFunc(&DTUSocket::parseHead1),
    m_subsequence(0),
    m_xorChecksum(0),
    m_sendSequence(0)
{
    connect(&m_socket, &QUdpSocket::readyRead, this, &DTUSocket::readPendingDatagrams);
    if(!m_socket.bind(QHostAddress::Any, localPort)) {
        qWarning("Can't bind to DTU of Port %d", localPort);
    }
    m_socket.connectToHost(peerHost, peerPort);
}

void DTUSocket::write(QByteArray &payload)
{
    char *data = payload.data();
    // 1.head
    payload[0] = (char)0xAE;
    payload[1] = (char)0x46;
    // 2.length
    *reinterpret_cast<quint16*>(data+2) = payload.size();
    // 3.send subsequence
    *reinterpret_cast<quint8*>(data+4) = m_sendSequence++;
    // 4.xor
    *(payload.rbegin()) = 0x00;
    quint8 xorChksum = 0;
    for(const char &byte : payload) {
        xorChksum ^= byte;
    }
    *(payload.rbegin()) = xorChksum;

    // send
    m_socket.write(payload);
}

void DTUSocket::readPendingDatagrams()
{
    while (m_socket.hasPendingDatagrams()) {
        QByteArray payload = m_socket.readAll();
        if(payload.isEmpty())
            continue;
        pushSerialData(payload);
    }
}

void DTUSocket::pushSerialData(QByteArray &data)
{
    for(const char &byte : data) {
        bool ok = (this->*(m_parseFunc))(byte);
        //注意不要疏忽这一种情况：如果某一字节校验失败，而这一字节正好是帧头第一个字节，那么接下来需要从第二个字节开始校验
        if(!ok && byte == (char)0xAE) {
            m_parseFunc = &DTUSocket::parseHead2;
        }
    }
}

inline bool DTUSocket::parseHead1(const char &byte)
{
    //reset
    m_xorChecksum = 0;
    m_xorChecksum ^= byte;
    m_payload.clear();
    //
    m_recvLen = 1;
    if(byte == (char)0xAE) {
        m_parseFunc = &DTUSocket::parseHead2;
        m_payload.append(byte);
        return true;
    }
    return false;
}

inline bool DTUSocket::parseHead2(const char &byte)
{
    m_xorChecksum ^= byte;
    m_recvLen = 2;
    if((quint8)byte == (quint8)0x46) {
        m_parseFunc = &DTUSocket::parseLength1;
        m_payload.append(byte);
        return true;
    }
    return false;
}

inline bool DTUSocket::parseLength1(const char &byte)
{
    m_xorChecksum ^= byte;
    m_recvLen = 3;
    *reinterpret_cast<quint8*>(&m_payloadLen) = (quint8)byte;   //front 1 byte of quint16
    m_parseFunc = &DTUSocket::parseLength2;
    m_payload.append(byte);
    return true;
}

inline bool DTUSocket::parseLength2(const char &byte)
{
    m_xorChecksum ^= byte;
    m_recvLen = 4;
    *(reinterpret_cast<quint8*>(&m_payloadLen)+1) = (quint8)byte;   //last 1 byte of quint16
    m_parseFunc = &DTUSocket::parseSubsequence;
    m_payload.append(byte);
    return true;
}

inline bool DTUSocket::parseSubsequence(const char &byte)
{
    m_xorChecksum ^= byte;
    m_recvLen = 5;
    quint8 preSub = m_subsequence;
    m_subsequence = (quint8)byte;
    if(preSub != m_subsequence) {
//        emit framLost(preSub, m_subsequence); //有BUG，暂时不使用
    }
    m_parseFunc = &DTUSocket::parsePayload;
    m_payload.append(byte);
    return true;
}

inline bool DTUSocket::parsePayload(const char &byte)
{
    ++m_recvLen;
    m_xorChecksum ^= byte;
    m_payload.append(byte);
    if(m_recvLen == m_payloadLen-1) {
        m_parseFunc = &DTUSocket::parseChecksum;
        return true;
    }
    return false;
}

inline bool DTUSocket::parseChecksum(const char &byte)
{
    m_parseFunc = &DTUSocket::parseHead1;
    m_payload.append(byte);
    if(m_xorChecksum == quint8(byte)) {
        DTUDatagram datagram;
        datagram.setLocalPort(m_socket.localPort());
        datagram.setPeerPort(m_socket.peerPort());
        datagram.setData(m_payload);
        m_boss->pushDatagram(datagram);
        return true;
    }
    return false;
}

DTUNetwork::DTUNetwork(QObject *parent) : QObject(parent)
{
}

void DTUNetwork::connectToDTU(const QHostAddress &server, const QMap<int, int> &portMap)
{
    if(!m_host.isNull()) {
        qWarning()<<"DTUNetork already connected to " << m_host;
        return;
    }
    //
    m_host = server;
    QMapIterator<int, int> port(portMap);
    while (port.hasNext()) {
        port.next();
        DTUSocket *dtu = new DTUSocket(m_host, port.key(), port.value(), this);
        m_dtuSockets.insert(port.key(), dtu);
    }
}

void DTUNetwork::write(QByteArray &data, int serverPort)
{
    auto iter = m_dtuSockets.find(serverPort);
    if(iter != m_dtuSockets.end()) {
        iter.value()->write(data);
    }
    else {
        qWarning()<<"Unable to find DTU Port: " << serverPort;
    }
}

bool DTUNetwork::hasPendingDatagrams() const
{
    return !m_datagramQueue.isEmpty();
}

DTUDatagram DTUNetwork::receiveDatagram()
{
    return m_datagramQueue.dequeue();
}

void DTUNetwork::pushDatagram(DTUDatagram &datagram)
{
    if(m_datagramQueue.isEmpty()) {
        // only the condition of Queue's size change from 0 to 1 emitting the signal
        m_datagramQueue.enqueue(datagram);
        emit readyRead();
    } else  {
        m_datagramQueue.enqueue(datagram);
    }
}
