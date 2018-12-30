#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QVariant>
#include "MainService.h"
#include <QDebug>

#define V2RAY_SYS_TRAY_ENABLED_ICON_PATH ":/images/enabled.png"
#define V2RAY_SYS_TRAY_DISABLED_ICON_PATH ":/images/disabled.png"
#define V2RAY_SYS_TRAY_ERROR_ICON_PATH ":/images/error.png"

MainService::MainService(QWidget *parent) :
    QWidget(parent),
    configuration(ConfigService::getInstance(this)),
    v2rayCore(new QProcess(this)),
    trayIcon(new QSystemTrayIcon(this)),
    mainMenu(new QMenu(this)),
    switchAction(new QAction(mainMenu)),
    workInstanceMenu(new QMenu(mainMenu)),
    workInstanceGroup(new QActionGroup(mainMenu)),
    configAction(new QAction(mainMenu)),
    monitorAction(new QAction(mainMenu)),
    aboutAction(new QAction(mainMenu)),
    exitAction(new QAction(mainMenu))
{
    // Start with error status flag
    setSysStatus(AppStatus::Error);

    // Initialize System Tray
    trayIcon->setContextMenu(mainMenu);
    trayIcon->show();

    // Initialize Main Menu
    initMainMenu();

    // Load Configuration and flag status
    loadConfiguration();

    // Binding signals and slots
    connect(v2rayCore, &QProcess::started, this, &MainService::processStartSlot);
    connect(v2rayCore, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &MainService::processFinishSlot);
    connect(v2rayCore, &QProcess::errorOccurred, this, &MainService::processErrorSlot);
}

MainService::~MainService()
{

}

void MainService::setSysStatus(MainService::AppStatus status)
{
    this->status = status;
    switch (status)
    {
    case AppStatus::Enabled:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_ENABLED_ICON_PATH));
        trayIcon->setToolTip(tr("Enabled"));
        switchAction->setText(tr("Disonnect"));
        switchAction->setEnabled(true);
        workInstanceMenu->setEnabled(true);
        break;
    case AppStatus::Disabled:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_DISABLED_ICON_PATH));
        trayIcon->setToolTip(tr("Disabled"));
        switchAction->setText(tr("Connect"));
        switchAction->setEnabled(true);
        workInstanceMenu->setEnabled(true);
        break;
    default:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_ERROR_ICON_PATH));
        trayIcon->setToolTip(tr("Error"));
        switchAction->setText(tr("Connect"));
        switchAction->setEnabled(false);
        workInstanceMenu->setEnabled(false);
        break;
    }
}

MainService::AppStatus MainService::getSysStatus()
{
    return status;
}

void MainService::initMainMenu()
{
    // Connect/Disconnect
    mainMenu->addAction(switchAction);
    connect(switchAction, &QAction::triggered, this, &MainService::switchSlot);

    mainMenu->addSeparator();

    // Work instances

    mainMenu->addMenu(workInstanceMenu);
    workInstanceMenu->setTitle(tr("Work Instances"));
    workInstanceGroup->setExclusive(true);

    mainMenu->addSeparator();

    // Config
    mainMenu->addAction(configAction);
    configAction->setText(tr("Configure..."));
    connect(configAction, &QAction::triggered, this, &MainService::configSlot);

    mainMenu->addSeparator();

    // Monitor
    mainMenu->addAction(monitorAction);
    monitorAction->setText(tr("Monitor..."));
    connect(monitorAction, &QAction::triggered, this, &MainService::monitorSlot);

    // About
    mainMenu->addAction(aboutAction);
    aboutAction->setText(tr("About..."));
    connect(aboutAction, &QAction::triggered, this, &MainService::aboutSlot);

    mainMenu->addSeparator();

    // Close
    mainMenu->addAction(exitAction);
    exitAction->setText(tr("Exit"));
    connect(exitAction, &QAction::triggered, this, &MainService::exitSlot);
}

void MainService::loadConfiguration()
{
    // Check if settings.json file exists
    QFileInfo settingsInfo("settings.json");
    if(settingsInfo.isFile())
    {
        // Load config file
        QString loadJsonErr;
        if(!configuration->loadFromJson("settings.json", loadJsonErr))
        {
            if(QMessageBox::Yes ==
            QMessageBox::critical(this, tr("Fatal Error"),
                    QString("%1\n%2")
                    .arg(loadJsonErr,
                            tr("Do you want to start with the default configuration?")),
                            QMessageBox::Yes|QMessageBox::No))
            {
                // Release resource json file
                if(configuration->releaseDefaultJson())
                {
                    if(QMessageBox::Yes == QMessageBox::information(this, tr("Reload"),
                            QString("%1\n%2")
                            .arg(tr("Default configuration has been released."),
                                    tr("Do you want to reload with it?")),
                                    QMessageBox::Yes|QMessageBox::No))
                    {
                        loadConfiguration();
                        return;
                    }
                }
            }
        }
        else
        {
            // Add work instances to menu
            loadWorkInstanceMenu();
            setSysStatus(AppStatus::Disabled);
            if(checkAvailable())
            {
                switchAction->setEnabled(true);
                qDebug() << configuration->getAutoConnect();
                if(configuration->getAutoConnect())
                {
                    qDebug() << "Got!";
                    switchSlot();
                }
            }
            else
            {
                switchAction->setEnabled(false);
            }
        }
    }
    else
    {
        if(QMessageBox::Yes ==
        QMessageBox::critical(this, tr("Fatal Error"),
                QString("%1\n%2")
                .arg(tr("No configuration file found."),
                     tr("Do you want to start with the default configuration?")),
                        QMessageBox::Yes|QMessageBox::No))
        {
            // Release resource json file
            if(configuration->releaseDefaultJson())
            {
                if(QMessageBox::Yes == QMessageBox::information(this, tr("Reload"),
                        QString("%1\n%2")
                        .arg(tr("Default configuration has been released."),
                                tr("Do you want to reload with it?")),
                                QMessageBox::Yes|QMessageBox::No))
                {
                    loadConfiguration();
                    return;
                }
            }
        }

    }
}

void MainService::clearWorkInstanceMenu()
{
    foreach (QAction *action, workInstanceMenu->actions())
    {
        delete action;
        action = nullptr;
    }
    workInstanceMenu->clear();
}

void MainService::loadWorkInstanceMenu()
{
    WorkInstanceList workInstances = configuration->getWorkInstances();
    currentWorkInstance = nullptr;
    QAction *configLevel = nullptr, *nameLevel = nullptr, *uniqueLevel = nullptr;
    clearWorkInstanceMenu();
    foreach (WorkInstance *item, workInstances)
    {
        QString tag = item->take(V2RAY_CONFIG_INSTANCE_TAG_KEY);
        QString configPath = item->take(V2RAY_CONFIG_INSTANCE_CONFIG_PATH_KEY);
        QVariant workInstanceVariant = QVariant::fromValue(item);
        QAction *workInstanceAction = workInstanceMenu->addAction(tag);
        workInstanceGroup->addAction(workInstanceAction);
        workInstanceAction->setCheckable(true);
        workInstanceAction->setToolTip(configPath);
        workInstanceAction->setData(workInstanceVariant);
        connect(workInstanceAction, &QAction::toggled, this, &MainService::selectWorkInstanceSlot);
        // Get each level's default work instance
        if(tag == configuration->getCurrentWorkInstance())
        {
            configLevel = workInstanceAction;
        }
        if(tag == "default")
        {
            nameLevel = workInstanceAction;
        }
    }
    if(workInstances.count() == 1)
    {
        uniqueLevel = workInstanceMenu->actions().at(0);
    }
    // Select each level's default work instance
    (configLevel == nullptr ? (nameLevel == nullptr ? uniqueLevel : nameLevel) : configLevel)->setChecked(true);
}

bool MainService::checkAvailable()
{
    return currentWorkInstance != nullptr;
}

void MainService::startConnect()
{
    qDebug() << "Got!";    v2rayCore->close();
    v2rayCore->setProgram(configuration->getCorePath());
    QStringList args;
    args << currentWorkInstance->take(V2RAY_CONFIG_CURRENT_INSTANCE_KEY);
    qDebug() << configuration->getCorePath() << args;
    v2rayCore->setArguments(args);
    v2rayCore->setProcessChannelMode(QProcess :: MergedChannels);
    v2rayCore->start();
    v2rayCore->closeWriteChannel();
}

void MainService::closeConnect()
{
    v2rayCore->close();
}

/******************************************** SLOTS ********************************************/

void MainService::switchSlot()
{
    if(status == AppStatus::Disabled)
    {
        startConnect();
    }
    else
    {
        closeConnect();
    }
}

void MainService::configSlot()
{
    notReady();
}

void MainService::monitorSlot()
{
    notReady();
}

void MainService::aboutSlot()
{
    notReady();
}

void MainService::exitSlot()
{
    v2rayCore->close();
    v2rayCore->waitForFinished();
    QApplication::quit();
}

void MainService::restartSlot()
{

}

void MainService::selectWorkInstanceSlot(bool checked)
{
    if(checked)
    {
        QAction *act=qobject_cast<QAction*>(sender());
        currentWorkInstance = act->data().value<WorkInstance*>();
        // restart v2ray core if it is running
        if(status == AppStatus::Enabled) startConnect();
    }
}

void MainService::processStartSlot()
{
    setSysStatus(AppStatus::Enabled);
}

void MainService::processFinishSlot(int exitCode, QProcess::ExitStatus exitStatus)
{
    setSysStatus(AppStatus::Disabled);
}

void MainService::processErrorSlot(QProcess::ProcessError error)
{
    qDebug() << v2rayCore->exitCode();
    QString errDes;
    switch (error)
    {
    case QProcess::FailedToStart:
        errDes = tr("V2Ray core failed to start.");
        break;
//    case QProcess::Crashed:
//        errDes = tr("V2Ray core crashed.");
//        break;
//    default:
//        errDes = tr("Unknown error.");
    }
    if(!errDes.isEmpty())
    {
        QMessageBox::critical(this, tr("V2Ray Aborted"), errDes);
    }
}

void MainService::notReady()
{
    QMessageBox::information(this, tr("Sorry"), tr("This feature is under Developing."));
}
