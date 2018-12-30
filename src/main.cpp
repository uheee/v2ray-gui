#include <QApplication>
#include <QFile>
#include "MainService.h"
#include "ConfigForm.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    MainService ms;
    return a.exec();
}
