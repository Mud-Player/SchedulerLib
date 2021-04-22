#ifndef DTUNETWORK_H
#define DTUNETWORK_H

#include "scheduler_global.h"
#include <QObject>
#include <QHash>
#include <QMap>
#include <QUdpSocket>
#include <QQueue>
class DTUNetwork;

class SCHEDULERSHARED_EXPORT DTUDatagram {
public:
    int localPort() const;
    void setLocalPort(int localPort);
    const int &peerPort() const;
    void setPeerPort(int localPort);
    const QByteArray &data() const;
    void setData(const QByteArray &payload);
private:
    int        m_peerPort;
    int        m_localPort;
    QByteArray m_data;
};

class DTUSocket : public QObject {
    Q_OBJECT
    typedef bool (DTUSocket::*ParseFunc)(const char &byte);
public:
    DTUSocket(QHostAddress &peerHost, int peerPort, int localPort, DTUNetwork *parent);
    void write(QByteArray &paylaod);
signals:
    void framLost(int previous, int current);
private:
    void readPendingDatagrams();
    void pushSerialData(QByteArray &data);
    //Parse function
    bool parseHead1(const char &byte);
    bool parseHead2(const char &byte);
    bool parseLength1(const char &byte);
    bool parseLength2(const char &byte);
    bool parseSubsequence(const char &byte);
    bool parsePayload(const char &byte);
    bool parseChecksum(const char &byte);
private:
    DTUNetwork         *m_boss;
    QUdpSocket          m_socket;
    QByteArray          m_payload;
    ParseFunc           m_parseFunc;
    quint8              m_subsequence;
    quint16             m_recvLen;
    quint16             m_payloadLen;
    quint8              m_xorChecksum;
    quint8              m_sendSequence;
};

class SCHEDULERSHARED_EXPORT DTUNetwork : public QObject
{
    Q_OBJECT
    friend DTUSocket;
public:
    explicit DTUNetwork(QObject *parent = nullptr);
    /*!
     * \brief connectToDTU
     * \param server 串口服务器地址
     * \param portMap <远程端口，本地端口>的表
     */
    void connectToDTU(const QHostAddress &server, const QMap<int ,int> &portMap);
    void write(QByteArray &data, int serverPort);
    bool hasPendingDatagrams() const;
    DTUDatagram receiveDatagram();

signals:
    void readyRead();

private:
    void pushDatagram(DTUDatagram &datagram);

private:
    QHash<int,DTUSocket*> m_dtuSockets; ///< (serverPort,DTUSocket)
    QHostAddress          m_host;
    QQueue<DTUDatagram>   m_datagramQueue;
};

#endif // DTUNETWORK_H
