#ifndef CORE_H
#define CORE_H
#include "scheduler_global.h"

#include <QObject>
#include <QMultiMap>
#include <QTimer>

class Module;

/*!
 * \brief 内核
 * \details 该类解析UDP数据的第一个int字节，作为标志位调用需要该数据的模块UDP处理事件
 */
class SCHEDULERSHARED_EXPORT Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(QObject *parent = nullptr);

    /// 开始运行
    void run();
    /// 结束运行
    void stop();
    /// 设置更新事件间隔(0代表关闭更新事件)
    void setUpdateInterval(int msec);

    /// 添加模块
    /// \param priority代表当前模块加载优先顺序,值越小越先加载
    void addModule(Module *module, int priority);
    /// 卸载模块
    void removeModule(Module *module);
    /// 查询模块
    template<class T>
    T *findModule();

    /// 添加UDP监听端口
    bool addUDP(int port);
    /// 给模块注册UDP监听事件
    bool registerUDP(Module *module, int identity);

private:
    void readUDPPendingDatagrams();
    void handleUpdateEvent();

private:
    //
    bool   m_isRun;
    QTimer m_timer;

    //
    QMultiMap<int, Module*> m_modules;
    QMultiMap<int, Module*> m_udpModules;
};

template<class T>
T *Core::findModule()
{
    for(auto item : m_modules) {
        auto module = qobject_cast<T*>(item);
        if(module)
            return module;
    }
    return nullptr;
}

#endif // CORE_H
