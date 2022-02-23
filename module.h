#ifndef MODULE_H
#define MODULE_H
#include "scheduler_global.h"

#include "core.h"
#include <QObject>

/*!
 * \brief 模块
 */
class SCHEDULERSHARED_EXPORT Module : public QObject
{
    Q_OBJECT
    friend Core;
public:
    explicit Module();
    Core *core();

protected:
    /// 为当前模块注册UDP监听事件
    bool registerUDP(int identity);

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

};


#endif // MODULE_H
