
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "ConfigService.h"

#define V2RAY_RESOURCE_SETTINGS_PATH            ":/config/config/settings.json"

ConfigService::ConfigService(QObject *parent) : QObject(parent)
{

}

ConfigService::~ConfigService() = default;

void ConfigService::clearWorkInstances()
{
    foreach (WorkInstance *item, workInstances)
    {
        delete item;
    }
}

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
        currentWorkInstance = jsonObj.take(V2RAY_CONFIG_CURRENT_INSTANCE_KEY).toString();
        workInstanceArray = jsonObj.take(V2RAY_CONFIG_INSTANCE_KEY).toArray();
    }
    catch (...)
    {
        errDes = QString("%1 %2").arg(filePath, tr("format error."));
        return false;
    }
    return true;
}

bool ConfigService::releaseDefaultJson()
{
    return QFile::copy(V2RAY_RESOURCE_SETTINGS_PATH, "./settings.json");
}

bool ConfigService::getAutoConnect() const
{
    return autoConnect;
}

const QString &ConfigService::getCorePath() const
{
    return corePath;
}

const QString &ConfigService::getCurrentWorkInstance() const
{
    return currentWorkInstance;
}

const WorkInstanceList &ConfigService::getWorkInstances()
{
    clearWorkInstances();
    foreach (QJsonValue item, workInstanceArray)
    {
        try
        {
            QJsonObject instanceObject = item.toObject();
            QString tag = instanceObject.take(V2RAY_CONFIG_INSTANCE_TAG_KEY).toString();
            QString configPath = instanceObject.take(V2RAY_CONFIG_INSTANCE_CONFIG_PATH_KEY).toString();
            WorkInstance *workInstance = new WorkInstance();
            workInstance->insert(V2RAY_CONFIG_INSTANCE_TAG_KEY, tag);
            workInstance->insert(V2RAY_CONFIG_INSTANCE_CONFIG_PATH_KEY, configPath);
            workInstances.append(workInstance);
        }
        catch (...)
        {
            continue;
        }
    }
    return workInstances;
}
