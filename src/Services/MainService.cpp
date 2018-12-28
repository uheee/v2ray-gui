#include <QApplication>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include "MainService.h"

#define V2RAY_SYS_TRAY_ENABLED_ICON_PATH ":/icon/icons/tray_enabled.ico"
#define V2RAY_SYS_TRAY_DISABLED_ICON_PATH ":/icon/icons/tray_disabled.ico"
#define V2RAY_SYS_TRAY_ERROR_ICON_PATH ":/icon/icons/tray_error.ico"

MainService::MainService(QWidget *parent) :
    QWidget(parent),
    configuration(new ConfigService(this)),
    trayIcon(new QSystemTrayIcon(this)),
    mainMenu(new QMenu(this)),
    switchAction(new QAction(this)),
    instanceAction(new QMenu(this)),
    configAction(new QAction(this)),
    monitorAction(new QAction(this)),
    aboutAction(new QAction(this)),
    exitAction(new QAction(this))
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

}

MainService::~MainService() = default;

void MainService::setSysStatus(MainService::AppStatus status)
{
    this->status = status;
    switch (status)
    {
    case AppStatus::Enabled:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_ENABLED_ICON_PATH));
        trayIcon->setToolTip(tr("Enabled"));
        switchAction->setText(tr("Connect"));
        switchAction->setEnabled(true);
        instanceAction->setEnabled(true);
        break;
    case AppStatus::Disabled:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_DISABLED_ICON_PATH));
        trayIcon->setToolTip(tr("Disabled"));
        switchAction->setText(tr("Disconnect"));
        switchAction->setEnabled(true);
        instanceAction->setEnabled(true);
        break;
    default:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_ERROR_ICON_PATH));
        trayIcon->setToolTip(tr("Error"));
        switchAction->setText(tr("Connect"));
        switchAction->setEnabled(false);
        instanceAction->setEnabled(false);
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

    // Instances

    mainMenu->addMenu(instanceAction);
    instanceAction->setTitle(tr("Instances"));

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
            // TODO Add instances to menu

            setSysStatus(AppStatus::Disabled);
            if(configuration->getAutoConnect())
            {
                switchSlot();
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

/******************************************** SLOTS ********************************************/

void MainService::switchSlot()
{

}

void MainService::configSlot()
{

}

void MainService::monitorSlot()
{

}

void MainService::aboutSlot()
{

}

void MainService::exitSlot()
{

}

void MainService::restartSlot()
{

}
