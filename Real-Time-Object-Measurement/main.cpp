#include "widget.h"
#include"QTextCodec"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    a.setFont(QFont("Microsoft Yahei", 9));


    return a.exec();
}
