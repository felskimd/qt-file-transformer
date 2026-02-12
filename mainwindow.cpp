#include "mainwindow.h"
#include "qtconcurrentrun.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::SingleShot);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_choose_folder_btn_clicked() {
    auto folder = QFileDialog::getExistingDirectory(this);
    ui->saving_folder_ledit->setText(folder);
}


void MainWindow::on_value_ledit_textChanged(const QString &arg1) {
    if (ui->value_text_rbtn->isChecked()) {
        QByteArray val{arg1.toStdString()};
        if (val.size() > 8) {
            ui->value_ledit->setText(val.first(8));
        }
    }
    else if (ui->value_nums_rbtn->isChecked()) {
        static QRegularExpression reg("^[0-9.]*$");
        QRegularExpressionMatch matching_result = reg.match(arg1);
        if (!matching_result.hasMatch()) {
            ui->value_ledit->undo();
            return;
        }
        auto splitted = arg1.split(".");
        bool change = false;
        for (auto& substr : splitted) {
            if (substr != "") {
                if (substr.toUInt() > 255) {
                    substr = "255";
                    change = true;
                }
                else if (substr.startsWith('0') && substr.size() > 1) {
                    substr = "0";
                    change = true;
                }
                else if (substr.size() > 3) {
                    substr = substr.first(3);
                    change = true;
                }
            }
        }
        if (splitted.size() != 8) {
            change = true;
        }
        if (change) {
            ui->value_ledit->undo();
        }
    }
}


void MainWindow::on_value_text_rbtn_clicked() {
    ui->value_ledit->setInputMask("");
    ui->value_ledit->setText("");
}


void MainWindow::on_value_directly_rbtn_clicked() {
    ui->value_ledit->setInputMask("Hh-Hh-Hh-Hh-Hh-Hh-Hh-Hh");
    ui->value_ledit->setText("00-00-00-00-00-00-00-00");
}


void MainWindow::on_value_nums_rbtn_clicked() {
    ui->value_ledit->setInputMask("");
    ui->value_ledit->setText("0.0.0.0.0.0.0.0");
}

QStringList GetFilesByMask(const Ui::MainWindow* ui) {
    QStringList filters = ui->mask_ledit->text().split(' ');
    QDir dir;
    dir.setPath(QDir::currentPath());
    return dir.entryList(filters, QDir::Files);
}

void PrintFilesToStatus(const Ui::MainWindow* ui, const QStringList& files);

bool ValidateBytesValue(const Ui::MainWindow* ui, QByteArray& out);

void MainWindow::on_start_btn_clicked() {
    bool use_timer = ui->use_timer_chk->isChecked();
    ui->start_btn->setEnabled(false);
    if (use_timer) {
        auto time = ui->timer_period_tedit->time();
        timer_->setInterval(time.second() * 1000);
        timer_->start();
        ui->timer_stop_btn->setEnabled(true);
        SingleShot();
    }
    else {
        SingleShot();
    }
}

std::optional<model::ModifyInput> MainWindow::GetInputData() {
    auto files = GetFilesByMask(ui);
    PrintFilesToStatus(ui, files);
    if (files.size() == 0) {
        return std::nullopt;
    }

    QByteArray value;
    if (!ValidateBytesValue(ui, value)) {
        ui->status_value_lbl->setText("wrong bytes (may be it's less, than 8)");
        return std::nullopt;
    }

    QString output_folder = ui->saving_folder_ledit->text();

    bool override = ui->rewriting_files_chk->isChecked();
    bool delete_input = ui->input_deleting_chk->isChecked();
    return model::ModifyInput{std::move(value), std::move(files), std::move(output_folder), override, delete_input};
}

void MainWindow::SingleShot() {
    auto input = GetInputData();
    if (!input) {
        if (!timer_->isActive()) {
            ui->start_btn->setEnabled(true);
        }
        return;
    }

    auto* worker = new model::Worker();

    connect(worker, &model::Worker::statusUpdated, ui->progress_bar, &QProgressBar::setValue);
    connect(&watcher_, &QFutureWatcher<QString>::finished, this, &MainWindow::HandleSingleShot);
    connect(&watcher_, &QFutureWatcher<QString>::finished, worker, &QObject::deleteLater);

    auto work = [worker, input = std::move(input)]() mutable
    {
        return worker->Modify(std::move(input.value()));
    };

    QFuture<QString> future = QtConcurrent::run(work);
    watcher_.setFuture(future);
}

void MainWindow::HandleSingleShot() {
    if (!timer_->isActive()) {
        ui->start_btn->setEnabled(true);
    }
    ui->progress_bar->setValue(0);
    QString error = watcher_.result();
    if (error.isEmpty()) {
        ui->status_value_lbl->setText("SUCCESS!!!");
    }
    else {
        ui->status_value_lbl->setText(error);
    }
}

void MainWindow::on_mask_btn_clicked() {
    const auto files = GetFilesByMask(ui);
    PrintFilesToStatus(ui, files);
}

bool ValidateBytesValue(const Ui::MainWindow* ui, QByteArray& out) {
    QByteArray result;
    if (ui->value_text_rbtn->isChecked()) {
        result.assign(ui->value_ledit->text().toStdString());
        if (result.size() != 8) {
            return false;
        }
    }
    else if (ui->value_nums_rbtn->isChecked()) {
        const QStringList nums = ui->value_ledit->text().split('.');
        if (nums.size() != 8) {
            return false;
        }
        for (const auto& num : nums) {
            result.append(num.toUInt());
        }
    }
    else if (ui->value_directly_rbtn->isChecked()) {
        const QStringList nums = ui->value_ledit->text().split('-');
        if (nums.size() != 8) {
            return false;
        }
        for (const auto& num : nums) {
            if (num.size() != 2) {
                return false;
            }
            result.append(QByteArray::fromHex(num.toUtf8()));
        }

    }
    out = std::move(result);
    return true;
}

void PrintFilesToStatus(const Ui::MainWindow* ui, const QStringList& files) {
    QString status = "found ";
    status.append(std::to_string(files.size()));
    status += " files";
    for (const auto& file : files) {
        status.append('\n');
        status.append(file);
    }
    ui->status_value_lbl->setText(status);
}


void MainWindow::on_timer_stop_btn_clicked() {
    timer_->stop();
    ui->start_btn->setEnabled(true);
    ui->timer_stop_btn->setEnabled(false);
}

