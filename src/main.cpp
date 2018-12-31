#include <QFile>
#include <QLockFile>
#include "MainService.h"
#include "ConfigForm.h"

#define V2RAY_LOCK_FILE ".v2ray-gui-lock"

int main(int argc, char *argv[])
{
    QLockFile lockFile(V2RAY_LOCK_FILE);
    if(!lockFile.tryLock()) return 0;
    MainService app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
