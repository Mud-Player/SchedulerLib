#include <QThread>
#include <QTimer>
#include <QDebug>
#include "scheduler.h"
#include "configcenter.h"

Callback::Callback(Scheduler *scheduler):QObject(scheduler),
    m_sched(scheduler)
{
}

Scheduler *Callback::scheduler()
{
    return m_sched;
}

Callback::~Callback()
{
    for(QString &topic : m_topicList) {
        m_sched->unregDDSCallback(this, topic);
    }
    for(int &identity : m_identityList) {
        m_sched->unregUDPCallback(this, identity);
    }
    m_sched->unregDTUCallback(this);
}

void Callback::apply(const DDSDatagram &datagram)
{
    Q_UNUSED(datagram);
}

void Callback::apply(const QByteArray &datagram)
{
    Q_UNUSED(datagram);
}

void Callback::apply(const DTUDatagram &datagram)
{
    Q_UNUSED(datagram);
}

void Callback::registerDDS(QString name)
{
    m_topicList.append(name);
    m_sched->regDDSCallback(this, name);
}

void Callback::registerUDP(int identity)
{
    m_identityList.append(identity);
    m_sched->regUDPCallback(this, identity);
}

void Callback::registerDTU()
{
    m_sched->regDTUCallback(this);
}

void Callback::unregisterDDS(QString name)
{
    m_sched->unregDDSCallback(this, name);
}

void Callback::unregisterUDP(int identity)
{
    m_sched->unregUDPCallback(this, identity);
}

void Callback::unregisterDTU()
{
    m_sched->unregDTUCallback(this);
}

Scheduler::Scheduler(QObject *parent) :
    QObject(parent)
{
}

void Scheduler::startDDS(int ddsID)
{
    connect(&m_ddsNetwork, &DDSNetwork::readyRead, this, &Scheduler::readDDSPendingDatagrams, Qt::QueuedConnection);
    m_ddsNetwork.start(ddsID);
}

void Scheduler::startUDP(int bindPort)
{
    auto udp = new QUdpSocket(this);
    connect(udp, &QUdpSocket::readyRead, this, &Scheduler::readUDPPendingDatagrams, Qt::AutoConnection);  //really a Qt::DirectConnection
    if(!udp->bind(QHostAddress::Any, bindPort, QAbstractSocket::ReuseAddressHint | QAbstractSocket::ShareAddress)) {
        qWarning() << "Can't bind to UDP with port:" << bindPort;
        delete udp;
    }
    else
        m_udpReceivers.append(udp);
}

void Scheduler::startDTU(const QHostAddress &server, const QMap<int, int> &portMap)
{
    connect(&m_dtuNetwork, &DTUNetwork::readyRead, this, &Scheduler::readDTUPendingDatagrams, Qt::AutoConnection);  //really a Qt::DirectConnection
    m_dtuNetwork.connectToDTU(server, portMap);
}

void Scheduler::quit()
{
    //here need to disconect slot and other sth
}

DDSNetwork *Scheduler::getDDSNetwork()
{
    return &m_ddsNetwork;
}

QUdpSocket *Scheduler::getUDPNetwork()
{
    return &m_udpSender;
}

DTUNetwork *Scheduler::getDTUNetwork()
{
    return &m_dtuNetwork;
}

void Scheduler::regDDSCallback(Callback *callback, const QString &ddsTopic)
{
    if(callback->objectName().isEmpty()) {
        qDebug()<<"Please setObjectName() of Callback before register topic: " << ddsTopic;
        return;
    }
    if(m_ddsCallbacks.contains(ddsTopic, callback)) {
        qDebug()<<"You don't need to register DDS twice: " << ddsTopic << "for " << callback->objectName();
        return;
    }
    // if had ever been registerd, so register it to DDSNetwork
    if(!m_ddsCallbacks.contains(ddsTopic)) {
        m_ddsNetwork.registerRead(ddsTopic.toUtf8());
    }
    m_ddsCallbacks.insert(ddsTopic, callback);
}

void Scheduler::unregDDSCallback(Callback *callback, const QString &ddsTopic)
{
    m_ddsCallbacks.remove(ddsTopic, callback);
    // if empty of one topic, so register dds topic from DDSNetwork as nobody use it
    if(!m_ddsCallbacks.contains(ddsTopic)) {
        m_ddsNetwork.unregisterRead(ddsTopic.toUtf8());
    }
}

void Scheduler::regUDPCallback(Callback *callback, const int &identity)
{
    if(callback->objectName().isEmpty()) {
        qDebug("Please setObjectName() of Callback before register identity: %x",identity);
        return;
    }
    if(m_udpCallbacks.contains(identity, callback)) {
        qDebug()<<"You don't need to register UPD twice: " << identity;
        return;
    }
    m_udpCallbacks.insert(identity, callback);
}

void Scheduler::unregUDPCallback(Callback *callback, const int &identity)
{
    m_udpCallbacks.remove(identity, callback);
}

void Scheduler::regDTUCallback(Callback *callback)
{
    if(m_dtuCallbacks.contains(callback))
        return;
    m_dtuCallbacks.append(callback);
}

void Scheduler::unregDTUCallback(Callback *callback)
{
    m_dtuCallbacks.removeOne(callback);
}

Callback *Scheduler::findCallback(const QString &name) const
{
    return this->findChild<Callback*>(name, Qt::FindDirectChildrenOnly);
}

Scheduler::~Scheduler()
{
    //手动删除所有Callbacks，防止Scheduler被析构后Callbacks手动析构自己造成崩溃（此时Scheduler已经析构完了）
    const QObjectList objList = this->children();
    for(QObject *obj : objList) {
        if(qobject_cast<Callback*>(obj)) {
            delete obj;
        }
    }
}

///runing at main thread
void Scheduler::readDDSPendingDatagrams()
{
    while (m_ddsNetwork.hasPendingDatagrams()) {    //retreive all pending data
        DDSDatagram datagram = m_ddsNetwork.receiveDatagram();
        const QString &topic = datagram.topic();
        auto callbacks = m_ddsCallbacks.values(topic);
        for(Callback* item : callbacks) {
            item->apply(datagram);
        }
    }
}

void Scheduler::readUDPPendingDatagrams()
{
    auto udp = dynamic_cast<QUdpSocket*>(this->sender());
    QByteArray datagram;
    while (udp->hasPendingDatagrams()) {    //retreive all pending data
        datagram.resize(int(udp->pendingDatagramSize()));
        udp->readDatagram(datagram.data(), datagram.size());
        int identity= *reinterpret_cast<int*>(datagram.data());   //标识
        auto callbacks = m_udpCallbacks.values(identity);
        for(Callback* item : callbacks) {
            item->apply(datagram);
        }
    }
}

void Scheduler::readDTUPendingDatagrams()
{
    while (m_dtuNetwork.hasPendingDatagrams()) {
        DTUDatagram datagram = m_dtuNetwork.receiveDatagram();
        for(Callback* item : qAsConst(m_dtuCallbacks)) {
            item->apply(datagram);
        }
    }
}
