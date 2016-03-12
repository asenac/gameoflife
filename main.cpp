#include <iostream>
#include <QApplication>
#include "widgets.hpp"

using namespace conway;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
