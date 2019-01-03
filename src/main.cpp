#include <QFile>
#include <QDir>
#include "MainService.h"
#include "ConfigForm.h"
#include "RunGuard.h"

#define V2RAY_LOCK_FILE QDir::homePath() + "/v2ray-gui-lock"

int main(int argc, char *argv[])
{
    RunGuard guard("3f65aa1a-a628-4e6c-b181-6794f106b261");
    if(!guard.tryToRun())
    {
        printf("Another application has already been running.\n");
        return 0;
    }
    MainService app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
