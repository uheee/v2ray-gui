#ifndef CONFIGSERVICE_H
#define CONFIGSERVICE_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QJsonArray>

#define V2RAY_CONFIG_AUTO_CONNECT_KEY           "autoConnect"
#define V2RAY_CONFIG_CORE_PATH_KEY              "corePath"
#define V2RAY_CONFIG_CURRENT_INSTANCE_KEY       "currentWorkInstance"
#define V2RAY_CONFIG_INSTANCE_KEY               "workInstances"
#define V2RAY_CONFIG_INSTANCE_TAG_KEY           "tag"
#define V2RAY_CONFIG_INSTANCE_CONFIG_PATH_KEY   "configPath"

typedef QMap<QString, QString> WorkInstance;
typedef QList<WorkInstance*> WorkInstanceList;

class ConfigService : QObject
{
    Q_OBJECT

public:
    static ConfigService *getInstance(QObject *parent = nullptr);
    bool loadFromJson(QString filePath, QString &errDes);
    bool releaseDefaultJson();
    bool getAutoConnect() const;
    const QString &getCorePath() const;
    const QString &getCurrentWorkInstance() const;
    const WorkInstanceList &getWorkInstances();

private:
    explicit ConfigService(QObject *parent = nullptr);
    ~ConfigService();
    void clearWorkInstances();

    static ConfigService *singleInstance;
    bool autoConnect;
    QString corePath;
    QString currentWorkInstance;
    QJsonArray workInstanceArray;
    WorkInstanceList workInstances;
};

Q_DECLARE_METATYPE(WorkInstance*)

#endif // CONFIGSERVICE_H
