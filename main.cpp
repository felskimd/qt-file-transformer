#include "mainwindow.h"

#include <QApplication>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QListView>
#include <QTreeView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    return a.exec();
}

