#ifndef CONFIGCENTER_H
#define CONFIGCENTER_H

#include "scheduler_global.h"
#include <QObject>
#include <QSettings>
#include <QHostAddress>

class SCHEDULERSHARED_EXPORT ConfigCenter : public QObject
{
    Q_OBJECT
public:
    static ConfigCenter *instance();
    static QString elevationPath();
    static QPair<int,int> DTUBeginPort();
    static QString DTUServerAddress();
private:
    explicit ConfigCenter(QObject *parent = nullptr);

private:
    static QSettings      m_settings;
};

#endif // CONFIGCENTER_H
