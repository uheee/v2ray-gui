
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "ConfigService.h"

#define V2RAY_CONFIG_AUTO_CONNECT_KEY           "autoConnect"
#define V2RAY_CONFIG_CORE_PATH_KEY              "corePath"
#define V2RAY_CONFIG_CURRENT_INSTANCE_KEY       "currentInstance"
#define V2RAY_CONFIG_INSTANCE_KEY               "instances"
#define V2RAY_CONFIG_INSTANCE_TAG_KEY           "tag"
#define V2RAY_CONFIG_INSTANCE_CONFIG_PATH_KEY   "configPath"

ConfigService::ConfigService(QObject *parent) : QObject(parent)
{

}

ConfigService::~ConfigService() = default;

bool ConfigService::loadFromJson(QString filePath, QString &errDes)
{
    QFile jsonFile(filePath, this);
    if(!jsonFile.open(QIODevice::ReadOnly)) return false;
    QJsonParseError jsonErr{};
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), &jsonErr);
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
        currentInstance = jsonObj.take(V2RAY_CONFIG_CURRENT_INSTANCE_KEY).toString();
        instances = jsonObj.take(V2RAY_CONFIG_INSTANCE_KEY).toArray();
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

}

bool ConfigService::getAutoConnect() const
{
    return autoConnect;
}

const QString &ConfigService::getCorePath() const
{
    return corePath;
}

const QString &ConfigService::getCurrentInstance() const
{
    return currentInstance;
}
