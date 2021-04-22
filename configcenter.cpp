#include "configcenter.h"
#include <QApplication>

QSettings ConfigCenter::m_settings("Config.ini", QSettings::IniFormat);
ConfigCenter *ConfigCenter::instance()
{
    static ConfigCenter *ins;
    return ins;
}

int ConfigCenter::ddsID()
{
    m_settings.beginGroup("DDS");
    int id = m_settings.value("ID", "1").toInt();
    m_settings.setValue("ID", id);
    m_settings.endGroup();
    return id;
}

QScreen *ConfigCenter::screen()
{
    m_settings.beginGroup("Screen");
    int screenNumber = m_settings.value("Number", "1").toInt();
    m_settings.setValue("Number", screenNumber);
    m_settings.endGroup();
    auto screens = qApp->screens();
    screenNumber = screenNumber <= screens.size() ? screenNumber : 0;
    screenNumber = screenNumber > 0 ? screenNumber : 1;
    return screens[screenNumber-1];
}

QString ConfigCenter::elevationPath()
{
    m_settings.beginGroup("DEM");
    QString demPath = m_settings.value("DEMPath", "E:/SRTM_DEM_China").toString();
    m_settings.setValue("DEMPath", demPath);
    m_settings.endGroup();
    return demPath;
}

QGeoCoordinate ConfigCenter::homePosition()
{
    m_settings.beginGroup("MapHome");
    double lat = m_settings.value("Latitude", 40.4015).toDouble();
    double lon = m_settings.value("Longitude", 99.7895).toDouble();
    double alt = m_settings.value("Altitude", 9000).toDouble();
    m_settings.setValue("Latitude", lat);
    m_settings.setValue("Longitude", lon);
    m_settings.setValue("Altitude", alt);
    m_settings.endGroup();
    return QGeoCoordinate(lat, lon, alt);
}

ConfigCenter::ConfigCenter(QObject *parent) : QObject(parent)
{
}
