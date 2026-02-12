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

    // модель файловой системы
    QFileSystemModel *model = new QFileSystemModel;
    // устанавливаем корневую папку
    model->setRootPath(QDir::currentPath());
    // устанавливаем модель для древовидного представления


    w.show();
    return a.exec();
}
