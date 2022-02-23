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

    /// 更新时间
    /// \param deltaMS 上一次更新距离本次更新的毫秒差
    virtual void updateEvent(int deltaMS) {};
};


#endif // MODULE_H
