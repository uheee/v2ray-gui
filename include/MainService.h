#ifndef MainService_H
#define MainService_H

#include <QApplication>
#include <QWidget>
#include <QPoint>
#include <QMenu>
#include <QActionGroup>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QMessageBox>
#include "ConfigService.h"

class MainService : public QApplication
{
    Q_OBJECT

public:
    explicit MainService(int &argc, char **argv);
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
    void restartConnect();
    void closeConnect();
    int ShowMsgBox(const QMessageBox::Icon type, const QString &text,
                   const QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                   const QMessageBox::StandardButton defaultButton = QMessageBox::Ok);

    void notReady();

public slots:
    void switchSlot();
    void configSlot();
    void monitorSlot();
    void aboutSlot();
    void quit();
    void restartSlot();
    void selectWorkInstanceSlot(bool checked);
    void processStartSlot();
    void processFinishSlot(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrorSlot(QProcess::ProcessError error);
};

#endif // MainService_H
