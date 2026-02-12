#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "model.h"
#include <QTimer>
#include <QMainWindow>
#include <QProgressBar>
#include <QFutureWatcher>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_choose_folder_btn_clicked();

    void on_value_ledit_textChanged(const QString &arg1);

    void on_value_text_rbtn_clicked();

    void on_value_directly_rbtn_clicked();

    void on_value_nums_rbtn_clicked();

    void on_start_btn_clicked();

    void on_mask_btn_clicked();

    void SingleShot();

    void HandleSingleShot();

    void on_timer_stop_btn_clicked();

private:
    Ui::MainWindow *ui;
    QFutureWatcher<QString> watcher_;
    QTimer* timer_ = new QTimer(this);

    std::optional<model::ModifyInput> GetInputData();
};
#endif // MAINWINDOW_H
