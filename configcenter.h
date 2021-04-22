#ifndef CONFIGCENTER_H
#define CONFIGCENTER_H

#include "scheduler_global.h"
#include <QObject>
#include <QSettings>
#include <QHostAddress>
#include <QGeoCoordinate>
#include <QScreen>

class SCHEDULERSHARED_EXPORT ConfigCenter : public QObject
{
    Q_OBJECT
public:
    static ConfigCenter *instance();
    static int ddsID();
    static QScreen *screen();
    static QString elevationPath();
    static QGeoCoordinate homePosition();
private:
    explicit ConfigCenter(QObject *parent = nullptr);

private:
    static QSettings      m_settings;
};

#endif // CONFIGCENTER_H
