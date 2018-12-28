#ifndef CONFIGSERVICE_H
#define CONFIGSERVICE_H

#include <QObject>
#include <QString>
#include <QJsonArray>

class ConfigService : QObject
{
    Q_OBJECT

public:
    explicit ConfigService(QObject *parent = nullptr);
    ~ConfigService();
    bool loadFromJson(QString filePath, QString &errDes);
    bool releaseDefaultJson();
    bool getAutoConnect() const;
    const QString &getCorePath() const;
    const QString &getCurrentInstance() const;

private:
    bool autoConnect;
    QString corePath;
    QString currentInstance;
    QJsonArray instances;
};

#endif // CONFIGSERVICE_H
