#ifndef MODULE_H
#define MODULE_H
#include "scheduler_global.h"

#include "core.h"
#include <QObject>
#include <QUdpSocket>

/*!
 * \brief 模块
 * \details 作为Core类的核心组成;内部默认集成一个UDP套接字
 */
class SCHEDULERSHARED_EXPORT Module : public QObject
{
    Q_OBJECT
    friend Core;
public:
    explicit Module(QObject *parent = nullptr);

protected:
    /// 为当前模块注册UDP监听事件
    /// 参见 \see Core::registerUDP
    bool registerUDP(int identity);

    /// 获取内置UDP套接字
    inline QUdpSocket *udpSocket() {return m_udp;}
    /// 使用内置UDP发送广播数据 \see Module::udpSocket
    void sendBroadcast(const char *data, qint64 size, quint16 port);
    /// 使用内置UDP发送点对点或者广播数据 \see Module::udpSocket
    void sendDatagram(const char *data, qint64 size, const QHostAddress &address, quint16 port);;

protected:
    /// 设置当前模块归属的Core(子类不因该调用)
    inline void setCore(Core *core) {m_core = core;}
    /// 获取当前模块归属的Core
    inline Core *core() {return m_core;}

protected:
    /// 加载前处理
    virtual void preLoad() {};
    /// 加载处理
    virtual void load() {};
    /// 加载后处理
    virtual void postLoad() {};

    /// 卸载前处理
    virtual void preUnload() {};
    /// 卸载处理
    virtual void unload() {};
    /// 卸载后处理
    virtual void postUnload() {};

    /// UDP数据事件,仅监听的数据才会触发该事件
    virtual void UDPEvent(int identity, const QByteArray &datagram){};

    /// 更新事件
    /// \param deltaMS 上一次更新距离本次更新的毫秒差
    virtual void updateEvent(int deltaMS) {};

private:
    Core       *m_core;
    QUdpSocket *m_udp;
};

#endif // MODULE_H
