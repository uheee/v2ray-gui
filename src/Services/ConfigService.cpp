
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "ConfigService.h"

#define V2RAY_RESOURCE_SETTINGS_PATH            ":/config/settings.json"

ConfigService::ConfigService(QObject *parent) : QObject(parent)
{

}

ConfigService::~ConfigService() = default;

ConfigService *ConfigService::getInstance(QObject *parent)
{
    static ConfigService instance(parent);
    return &instance;
}

bool ConfigService::loadFromJson(QString filePath, QString &errDes)
{
    QFile jsonFile(filePath, this);
    if(!jsonFile.open(QIODevice::ReadOnly)) return false;
    QJsonParseError jsonErr{};
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), &jsonErr);
    jsonFile.close();
    if(jsonErr.error != QJsonParseError::NoError)
    {
        errDes = jsonErr.errorString();
        return false;
    }
    try
    {
        QJsonObject jsonObj = jsonDoc.object(); // Root Object
        autoConnect = jsonObj.take(V2RAY_CONFIG_AUTO_CONNECT_KEY).toBool();
        corePath = jsonObj.take(V2RAY_CONFIG_CORE_PATH_KEY).toString();
        currentSection = jsonObj.take(V2RAY_CONFIG_CURRENT_SECTION_KEY).toString();
        sectionArray = jsonObj.take(V2RAY_CONFIG_SECTION_KEY).toArray();
    }
    catch (...)
    {
        errDes = QString("%1 %2").arg(filePath, tr("format error."));
        return false;
    }
    return true;
}

bool ConfigService::releaseDefaultJson(const QString &path)
{
    return QFile::copy(V2RAY_RESOURCE_SETTINGS_PATH, path);
}

bool ConfigService::getAutoConnect() const
{
    return autoConnect;
}

const QString &ConfigService::getCorePath() const
{
    return corePath;
}

const QString &ConfigService::getCurrentSection() const
{
    return currentSection;
}

const SectionList &ConfigService::getSections()
{
    foreach (QJsonValue item, sectionArray)
    {
        try
        {
            QJsonObject sectionObject = item.toObject();
            QString tag = sectionObject.take(V2RAY_CONFIG_SECTION_TAG_KEY).toString();
            QString configPath = sectionObject.take(V2RAY_CONFIG_SECTION_CONFIG_PATH_KEY).toString();
            SectionPtr section(new Section());
            section->insert(V2RAY_CONFIG_SECTION_TAG_KEY, tag);
            section->insert(V2RAY_CONFIG_SECTION_CONFIG_PATH_KEY, configPath);
            sections.append(section);
        }
        catch (...)
        {
            continue;
        }
    }
    return sections;
}
