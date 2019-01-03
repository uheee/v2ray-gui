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
    ConfigService *configuration = ConfigService::getInstance();
    QProcess *v2rayCore;
    QSystemTrayIcon *trayIcon;
    QMenu *mainMenu;
    QAction *switchAction;
    QMenu *sectionMenu;
    QActionGroup *sectionGroup;
    QAction *configAction;
    QAction *monitorAction;
    QAction *aboutAction;
    QAction *exitAction;
    SectionPtr currentSection;

    // Functions
    void sayHello(QSystemTrayIcon::ActivationReason reason);
    void setSysStatus(const AppStatus status);
    AppStatus getSysStatus() const;
    void initMainMenu() const;
    void loadConfiguration();
    void clearSectionMenu() const;
    void loadSectionMenu();
    bool checkAvailable() const;
    void restartConnect() const;
    void closeConnect() const;
    int showMsgBox(const QMessageBox::Icon type, const QString &text,
                   const QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                   const QMessageBox::StandardButton defaultButton = QMessageBox::Ok) const;

    void notReady() const;

public slots:
    void switchSlot() const;
    void configSlot() const;
    void monitorSlot() const;
    void aboutSlot() const;
    void quit() const;
    void restartSlot() const;
    void selectSectionSlot(bool checked);
    void processStartSlot();
    void processFinishSlot(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrorSlot(QProcess::ProcessError error) const;
};

#endif // MainService_H
