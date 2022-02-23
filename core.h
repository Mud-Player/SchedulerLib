#ifndef CORE_H
#define CORE_H
#include "scheduler_global.h"

#include <QObject>
#include <QMultiMap>
class Module;

/*!
 * \brief 内核
 */
class SCHEDULERSHARED_EXPORT Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(QObject *parent = nullptr);

    /// 添加模块
    /// \param priority代表当前模块加载优先顺序,值越小越先加载
    void addModule(Module *module, int priority);
    /// 卸载模块
    void removeModule(Module *module);

    /// 查询模块
    template<class T>
    T *findModule();

    /// 开始运行
    void run();
    /// 停止运行
    void stop();

private:
    //
    bool m_isRun;

    //
    QMultiMap<int, Module*> m_modules;
};

template<class T>
T *Core::findModule()
{
    for(auto module : m_modules) {
        auto tModule = qobject_cast<T*>(module);
        if(tModule)
            return tModule;
    }
    return nullptr;
}

#endif // CORE_H
