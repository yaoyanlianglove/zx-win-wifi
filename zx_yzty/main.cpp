#include "yzty.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    YZTY w;
    w.show();

    return a.exec();
}
