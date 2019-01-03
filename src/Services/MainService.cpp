#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <QVariant>
#include <QLockFile>
#include "MainService.h"

#define V2RAY_SETTINGS_FILE_PATH                applicationDirPath() + "/settings.json"
#define V2RAY_SYS_TRAY_ENABLED_ICON_PATH        ":/images/enabled.png"
#define V2RAY_SYS_TRAY_DISABLED_ICON_PATH       ":/images/disabled.png"
#define V2RAY_SYS_TRAY_ERROR_ICON_PATH          ":/images/error.png"
#define V2RAY_ERR_MSG_START_WITH_DEFAULT        "Do you want to start with the default configuration?"
#define V2RAY_ERR_MSG_NO_CONFIG_FILE            "No configuration file found."
#define V2RAY_QUES_MSG_DEFAULT_CONFIG_RELEASED  "Default configuration has been released"
#define V2RAY_QUES_MSG_WHETHER_RELOAD           "Do you want to reload with it?"
#define V2RAY_INFO_MSG_FEATURE_UNDER_DEV        "Sorry, this feature is under Developing"
#define V2RAY_TRAY_MSG_SECTION_CHANGE           "Section has changed to"

MainService::MainService(int &argc, char **argv) :
    QApplication(argc, argv),
    v2rayCore(new QProcess(this)),
    trayIcon(new QSystemTrayIcon(this)),
    mainMenu(new QMenu()),
    switchAction(new QAction(mainMenu)),
    sectionMenu(new QMenu(mainMenu)),
    sectionGroup(new QActionGroup(mainMenu)),
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
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainService::sayHello);
    connect(v2rayCore, &QProcess::started, this, &MainService::processStartSlot);
    connect(v2rayCore, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &MainService::processFinishSlot);
    connect(v2rayCore, &QProcess::errorOccurred, this, &MainService::processErrorSlot);
}

MainService::~MainService()
{
    delete mainMenu;
    mainMenu = nullptr;
}

void MainService::sayHello(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick)
        trayIcon->showMessage(":)", "Hello!");
}

void MainService::setSysStatus(const MainService::AppStatus status)
{
    this->status = status;
    switch (status)
    {
    case AppStatus::Enabled:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_ENABLED_ICON_PATH));
        trayIcon->setToolTip(tr("Enabled"));
        switchAction->setText(tr("Disonnect"));
        switchAction->setEnabled(true);
        sectionMenu->setEnabled(true);
        break;
    case AppStatus::Disabled:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_DISABLED_ICON_PATH));
        trayIcon->setToolTip(tr("Disabled"));
        switchAction->setText(tr("Connect"));
        switchAction->setEnabled(true);
        sectionMenu->setEnabled(true);
        break;
    default:
        trayIcon->setIcon(QIcon(V2RAY_SYS_TRAY_ERROR_ICON_PATH));
        trayIcon->setToolTip(tr("Error"));
        switchAction->setText(tr("Connect"));
        switchAction->setEnabled(false);
        sectionMenu->setEnabled(false);
        break;
    }
}

MainService::AppStatus MainService::getSysStatus() const
{
    return status;
}

void MainService::initMainMenu() const
{
    // Connect/Disconnect
    mainMenu->addAction(switchAction);
    connect(switchAction, &QAction::triggered, this, &MainService::switchSlot);

    mainMenu->addSeparator();

    // Sections

    mainMenu->addMenu(sectionMenu);
    sectionMenu->setTitle(tr("Sections"));
    sectionGroup->setExclusive(true);

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
    connect(exitAction, &QAction::triggered, this, &MainService::quit);
}

void MainService::loadConfiguration()
{
    // Check if settings.json file exists
    QFileInfo settingsInfo(V2RAY_SETTINGS_FILE_PATH);
    if(settingsInfo.isFile())
    {
        // Load config file
        QString loadJsonErr;
        if(!configuration->loadFromJson(V2RAY_SETTINGS_FILE_PATH, loadJsonErr))
        {
            if(showMsgBox(QMessageBox::Critical, QString("%1\n%2").arg(loadJsonErr, tr(V2RAY_ERR_MSG_START_WITH_DEFAULT)),
                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
            {
                // Release resource json file
                if(configuration->releaseDefaultJson(V2RAY_SETTINGS_FILE_PATH))
                {
                    if(QMessageBox::Yes == QMessageBox::information(nullptr, tr("Reload"),
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
            // Add sections to menu
            loadSectionMenu();
            setSysStatus(AppStatus::Disabled);
            if(checkAvailable())
            {
                switchAction->setEnabled(true);
                if(configuration->getAutoConnect())
                {
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
        if(showMsgBox(QMessageBox::Critical, QString("%1\n%2").arg(V2RAY_ERR_MSG_NO_CONFIG_FILE, tr(V2RAY_ERR_MSG_START_WITH_DEFAULT)),
                      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
        {
            // Release resource json file
            if(configuration->releaseDefaultJson(V2RAY_SETTINGS_FILE_PATH))
            {
                if(showMsgBox(QMessageBox::Question, QString("%1\n%2").arg(V2RAY_QUES_MSG_DEFAULT_CONFIG_RELEASED, tr(V2RAY_QUES_MSG_WHETHER_RELOAD)),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    loadConfiguration();
                    return;
                }
            }
        }

    }
}

void MainService::clearSectionMenu() const
{
    foreach (QAction *action, sectionMenu->actions())
    {
        delete action;
        action = nullptr;
    }
    sectionGroup->actions().clear();
    sectionMenu->actions().clear();
}

void MainService::loadSectionMenu()
{
    SectionList sections = configuration->getSections();
    currentSection.reset();
    QAction *configLevel = nullptr, *nameLevel = nullptr, *uniqueLevel = nullptr;
    clearSectionMenu();
    foreach (SectionPtr item, sections)
    {
        QString tag = item->take(V2RAY_CONFIG_SECTION_TAG_KEY);
        QString configPath = item->take(V2RAY_CONFIG_SECTION_CONFIG_PATH_KEY);
        QVariant sectionVariant = QVariant::fromValue(item);
        QAction *sectionAction = sectionMenu->addAction(tag);
        sectionGroup->addAction(sectionAction);
        sectionAction->setCheckable(true);
        sectionAction->setToolTip(configPath);
        sectionAction->setData(sectionVariant);
        connect(sectionAction, &QAction::toggled, this, &MainService::selectSectionSlot);
        // Get each level's default section
        if(tag == configuration->getCurrentSection())
        {
            configLevel = sectionAction;
        }
        if(tag == "default")
        {
            nameLevel = sectionAction;
        }
    }
    if(sections.count() == 1)
    {
        uniqueLevel = sectionMenu->actions().at(0);
    }
    // Select each level's default section
    (configLevel == nullptr ? (nameLevel == nullptr ? uniqueLevel : nameLevel) : configLevel)->setChecked(true);
    configLevel = nameLevel = uniqueLevel = nullptr;
}

bool MainService::checkAvailable() const
{
    return currentSection.get() != nullptr;
}

void MainService::restartConnect() const
{
    closeConnect();
    v2rayCore->setProgram(configuration->getCorePath());
    QStringList args;
    args << currentSection->take(V2RAY_CONFIG_CURRENT_SECTION_KEY);
    v2rayCore->setArguments(args);
    v2rayCore->setProcessChannelMode(QProcess :: MergedChannels);
    v2rayCore->start();
    v2rayCore->closeWriteChannel();
}

void MainService::closeConnect() const
{
    v2rayCore->kill();
    v2rayCore->waitForFinished();
    v2rayCore->close();
}

int MainService::showMsgBox(const QMessageBox::Icon type, const QString &text,
                            const QMessageBox::StandardButtons buttons,
                            const QMessageBox::StandardButton defaultButton) const
{
    QMessageBox msgBox;
    msgBox.setIcon(type);
    msgBox.setText(text);
    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(defaultButton);
    switch (type)
    {
    case QMessageBox::Question:
        msgBox.setWindowTitle("Question");
        break;
    case QMessageBox::Information:
        msgBox.setWindowTitle("Information");
        break;
    case QMessageBox::Warning:
        msgBox.setWindowTitle("Warning");
        break;
    case QMessageBox::Critical:
        msgBox.setWindowTitle("Critical");
        break;
    default:
        msgBox.setWindowTitle("Unknown");
        break;
    }
    msgBox.show();
    msgBox.move((desktop()->width() - msgBox.width()) / 2, (desktop()->height() - msgBox.height()) / 2);
    return msgBox.exec();
}


void MainService::notReady() const
{
//    QMessageBox::information(nullptr, tr("Sorry"), tr("This feature is under Developing."));
    showMsgBox(QMessageBox::Information, tr(V2RAY_INFO_MSG_FEATURE_UNDER_DEV));
}

/******************************************** SLOTS ********************************************/

void MainService::switchSlot() const
{
    if(status == AppStatus::Disabled)
    {
        restartConnect();
    }
    else
    {
        closeConnect();
    }
}

void MainService::configSlot() const
{
    notReady();
}

void MainService::monitorSlot() const
{
    notReady();
}

void MainService::aboutSlot() const
{
    notReady();
}

void MainService::quit() const
{
    closeConnect();
    QApplication::quit();
}

void MainService::restartSlot() const
{

}

void MainService::selectSectionSlot(bool checked)
{
    if(checked)
    {
        QAction *act=qobject_cast<QAction*>(sender());
        currentSection = act->data().value<SectionPtr>();
        // restart v2ray core if it is running
        if(status == AppStatus::Enabled)
        {
            restartConnect();
            trayIcon->showMessage("V2Ray", QString("%1 %2").arg(tr(V2RAY_TRAY_MSG_SECTION_CHANGE), currentSection->take(V2RAY_CONFIG_SECTION_TAG_KEY)));
        }
    }
}

void MainService::processStartSlot()
{
    setSysStatus(AppStatus::Enabled);
}

void MainService::processFinishSlot(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    setSysStatus(AppStatus::Disabled);
}

void MainService::processErrorSlot(QProcess::ProcessError error) const
{
    QString errDes;
    switch (error)
    {
    case QProcess::FailedToStart:
        errDes = tr("V2Ray core failed to start.");
        break;
    case QProcess::Crashed:
//        errDes = tr("V2Ray core crashed.");
        break;
    default:
//        errDes = tr("Unknown error.");
        break;
    }
    if(!errDes.isEmpty())
    {
//        QMessageBox::critical(nullptr, tr("V2Ray Aborted"), errDes);
        showMsgBox(QMessageBox::Critical, errDes);
    }
}
