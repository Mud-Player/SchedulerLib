#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "scheduler_global.h"
#include <QThread>
#include <QMap>
#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include "ddsnetwork.h"
#include "dtunetwork.h"

class Scheduler;
///数据处理回调类，多个回调通过调度器的机制可以互相访问
///\note Callback必须设置唯一的objectName
class SCHEDULERSHARED_EXPORT Callback : public QObject{
    Q_OBJECT
    friend Scheduler;
public:
    Callback(Scheduler *scheduler);
    Scheduler *scheduler();
    ~Callback();
private:
    virtual void apply(const DDSDatagram      &datagram); ///<DDS数据
    virtual void apply(const QByteArray       &datagram); ///<UDP数据
    virtual void apply(const DTUDatagram      &datagram); ///<DTU数据
protected:
    void registerDDS(QString name);
    void registerUDP(int identity);
    void registerDTU();
    void unregisterDDS(QString name);
    void unregisterUDP(int identity);
    void unregisterDTU();
protected:
    Scheduler  *m_sched;
    QStringList m_topicList;
    QList<int>  m_identityList;
};

///调度器，调度整套系统运行
class SCHEDULERSHARED_EXPORT Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(QObject *parent = nullptr);
    /// 开始监听DDS(暂时不支持多个域)
    void startDDS(int ddsID);
    /// 开始监听UDP(多次调用可以同时监听多个端口)
    void startUDP(int bindPort);
    /*!
     * \brief connectToDTU
     * \param server 串口服务器地址
     * \param portMap <远程端口，本地端口>的表
     */
    void startDTU(const QHostAddress &server, const QMap<int, int> &portMap);
    /*!
     * \brief quit
     * \todo 当前函数还没有想好实现,可能需要分开为几个单独的stop函数，以对应startXXX函数
     */
    void quit();                                       ///<结束
    DDSNetwork *getDDSNetwork();
    QUdpSocket *getUDPNetwork();
    DTUNetwork *getDTUNetwork();

    /*!
     * \brief 注册Callback类处理数据
     * \param callback 处理数据的回调类
     */
    void regDDSCallback(Callback *callback, const QString &ddsTopic);     ///<DDS
    void unregDDSCallback(Callback *callback, const QString &ddsTopic);
    void regUDPCallback(Callback *callback, const int &identity);         ///<UDP
    void unregUDPCallback(Callback *callback, const int &identity);
    void regDTUCallback(Callback *callback);                              ///<DTU
    void unregDTUCallback(Callback *callback);
    /*!
     * \brief 查找已注册的回调类
     */
    Callback *findCallback(const QString &name) const;
    ~Scheduler();

private:
    void readDDSPendingDatagrams();
    void readUDPPendingDatagrams();
    void readDTUPendingDatagrams();

private:
    //
    DDSNetwork         m_ddsNetwork;
    QUdpSocket         m_udpSender;    ///< 只发送数据，不负责接收
    QList<QUdpSocket*> m_udpReceivers; ///< 只发送接收，不负责发送
    DTUNetwork         m_dtuNetwork;
    //
    QMultiHash<QString,Callback*>  m_ddsCallbacks;  ///<用来处理DDS数据的回调<topic,callback>
    QMultiHash<int,Callback*>      m_udpCallbacks;  ///<用来处理UDP数据的回调<identity,callback>
    QList<Callback*>               m_dtuCallbacks;  ///<用来处理DTU数据的回调
};

#endif // SCHEDULER_H
