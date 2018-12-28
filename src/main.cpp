#include <QApplication>
#include <QFile>
#include "MainService.h"
#include "ConfigForm.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QFile style(":/qss/theme/style.qss");
//    if(style.open(QIODevice::ReadOnly))
//    {
//        qApp->setStyleSheet(style.readAll());
//        style.close();
//    }
    QApplication::setQuitOnLastWindowClosed(false);
    MainService ms;
//    aboutForm.show();
    return a.exec();
}
