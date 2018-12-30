#include <QFile>
#include "MainService.h"
#include "ConfigForm.h"

int main(int argc, char *argv[])
{
    MainService app(argc, argv);
    MainService::setQuitOnLastWindowClosed(false);
    return app.exec();
}
