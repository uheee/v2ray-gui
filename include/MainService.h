#ifndef MainService_H
#define MainService_H

#include <QWidget>
#include <QMenu>
#include <QSystemTrayIcon>
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
    QSystemTrayIcon *trayIcon;
    QMenu *mainMenu;
    QAction *switchAction;
    QMenu *instanceAction;
    QAction *configAction;
    QAction *monitorAction;
    QAction *aboutAction;
    QAction *exitAction;

    // Functions
    void setSysStatus(AppStatus status);
    AppStatus getSysStatus();
    void initMainMenu();
    void loadConfiguration();

public slots:
    void switchSlot();
    void configSlot();
    void monitorSlot();
    void aboutSlot();
    void exitSlot();
    void restartSlot();

};

#endif // MainService_H
