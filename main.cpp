#include "MainWindow.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    MainWindow main_window;
    if (!main_window.initialize())
        return 1;

    main_window.show();
    return application.exec();
}
