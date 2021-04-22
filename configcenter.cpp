#include "configcenter.h"

QSettings ConfigCenter::m_settings("Config.ini", QSettings::IniFormat);
ConfigCenter *ConfigCenter::instance()
{
    static ConfigCenter *ins;
    return ins;
}

QString ConfigCenter::elevationPath()
{
    m_settings.beginGroup("DEM");
    QString demPath = m_settings.value("DEMPath", "E:/SRTM_DEM_China").toString();
    m_settings.setValue("DEMPath", demPath);
    m_settings.endGroup();
    return demPath;
}

QPair<int, int> ConfigCenter::DTUBeginPort()
{
    m_settings.beginGroup("DTU");
    int servelPort = m_settings.value("ServerPort", 10001).toInt();
    int localPort = m_settings.value("LocalPort", 20001).toInt();
    m_settings.setValue("ServerPort", servelPort);
    m_settings.setValue("LocalPort", localPort);
    m_settings.endGroup();
    return qMakePair(servelPort, localPort);
}

QString ConfigCenter::DTUServerAddress()
{
    m_settings.beginGroup("DTU");
    QString addr = m_settings.value("ServerAddr", QString("192.168.0.233")).toString();
    m_settings.setValue("ServerAddr", addr);
    m_settings.endGroup();
    return addr;
}

ConfigCenter::ConfigCenter(QObject *parent) : QObject(parent)
{
}
