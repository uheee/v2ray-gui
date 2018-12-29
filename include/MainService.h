#ifndef MainService_H
#define MainService_H

#include <QWidget>
#include <QMenu>
#include <QActionGroup>
#include <QSystemTrayIcon>
#include <QProcess>
#include "ConfigService.h"

class MainService : public QWidget
{
    Q_OBJECT

public:
    explicit MainService(QWidget *parent = nullptr);
    ~MainService();

    // specify application status
    enum AppStatus
    {
        Enabled,
        Disabled,
        Error
    };

private:
    // Properties
    AppStatus status;
    ConfigService *configuration;
    QProcess *v2rayCore;
    QSystemTrayIcon *trayIcon;
    QMenu *mainMenu;
    QAction *switchAction;
    QMenu *workInstanceMenu;
    QActionGroup *workInstanceGroup;
    QAction *configAction;
    QAction *monitorAction;
    QAction *aboutAction;
    QAction *exitAction;
    WorkInstance *currentWorkInstance;

    // Functions
    void setSysStatus(AppStatus status);
    AppStatus getSysStatus();
    void initMainMenu();
    void loadConfiguration();
    void clearWorkInstanceMenu();
    void loadWorkInstanceMenu();
    bool checkAvailable();
    void startConnect();
    void closeConnect();

    void notReady();

public slots:
    void switchSlot();
    void configSlot();
    void monitorSlot();
    void aboutSlot();
    void exitSlot();
    void restartSlot();
    void selectWorkInstanceSlot(bool checked);
    void processStartSlot();
    void processFinishSlot(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrorSlot(QProcess::ProcessError error);
};

#endif // MainService_H
