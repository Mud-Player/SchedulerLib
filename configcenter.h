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
    static QPair<int,int> DTUBeginPort();
    static QHostAddress DTUServerAddress();
    bool saveDatabase() const;
private:
    explicit ConfigCenter(QObject *parent = nullptr);

private:
    static QSettings      m_settings;
    bool m_saveDatabase;    ///< 是否保存数据
};

#endif // CONFIGCENTER_H
