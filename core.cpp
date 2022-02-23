#include "core.h"
#include <QMapIterator>
#include "module.h"

Core::Core(QObject *parent) : QObject(parent)
{
}

void Core::addModule(Module *module, int priority)
{
    m_modules.insert(priority, module);
    module->setParent(this);
}

void Core::run()
{
    QMapIterator<int, Module*> iter(m_modules);
    while (iter.hasNext()) {
        auto module = iter.next().value();
        module->preLoad();
    }
    iter.toFront();
    while (iter.hasNext()) {
        auto module = iter.next().value();
        module->load();
    }
    iter.toFront();
    while (iter.hasNext()) {
        auto module = iter.next().value();
        module->postLoad();
    }
}

void Core::stop()
{
    QMapIterator<int, Module*> iter(m_modules);
    while (iter.hasNext()) {
        auto module = iter.next().value();
        module->preUnload();
    }
    iter.toFront();
    while (iter.hasNext()) {
        auto module = iter.next().value();
        module->unload();
    }
    iter.toFront();
    while (iter.hasNext()) {
        auto module = iter.next().value();
        module->postUnload();
    }
}

