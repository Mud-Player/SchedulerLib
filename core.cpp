#include "core.h"
#include "module.h"
#include <QMapIterator>
#include <QUdpSocket>

Core::Core(QObject *parent) : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &Core::handleUpdateEvent, Qt::DirectConnection);
}

void Core::run()
{
    for(auto module : qAsConst(m_modules)) {
        module->preLoad();
    }
    for(auto module : qAsConst(m_modules)) {
        module->load();
    }
    for(auto module : qAsConst(m_modules)) {
        module->postLoad();
    }
}

void Core::stop()
{
    for(auto module : qAsConst(m_modules)) {
        module->preUnload();
    }
    for(auto module : qAsConst(m_modules)) {
        module->unload();
    }
    for(auto module : qAsConst(m_modules)) {
        module->postUnload();
    }
}

void Core::setUpdateInterval(int msec)
{
    if(msec == 0)
        m_timer.stop();
    else {
        m_timer.start(msec);
    }
}

void Core::addModule(Module *module, int priority)
{
    m_modules.insert(priority, module);
    module->setParent(this);
}

bool Core::addUDP(int port)
{
    auto udp = new QUdpSocket(this);
    connect(udp, &QUdpSocket::readyRead, this, &Core::readUDPPendingDatagrams, Qt::DirectConnection);  //really a Qt::DirectConnection
    if(!udp->bind(QHostAddress::Any, port, QAbstractSocket::ReuseAddressHint | QAbstractSocket::ShareAddress)) {
        qWarning() << "Can't bind to UDP with port:" << port;
        delete udp;
        return false;
    }

    return true;
}

bool Core::registerUDP(Module *module, int identity)
{
    if(!module)
        return false;
    if(m_udpModules.contains(identity, module)) {
        return false;
    }

    m_udpModules.insert(identity, module);
    return true;
}

void Core::readUDPPendingDatagrams()
{
    auto udp = dynamic_cast<QUdpSocket*>(this->sender());
    QByteArray datagram;
    while (udp->hasPendingDatagrams()) {    //retreive all pending data
        datagram.resize(udp->pendingDatagramSize());
        udp->readDatagram(datagram.data(), datagram.size());
        int identity= *reinterpret_cast<int*>(datagram.data());   //标识
        auto callbacks = m_udpModules.values(identity);
        for(auto item : callbacks) {
            item->UDPEvent(identity, datagram);
        }
    }
}

void Core::handleUpdateEvent()
{
    for(auto module : qAsConst(m_modules)) {
        module->updateEvent(m_timer.interval());
    }
}
