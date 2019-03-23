#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scanner.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scanner_thread(new QThread(this)),
    scanner(new Scanner),
    timer()
{
    ui->setupUi(this);

    setWindowTitle("Duplicate Files Scanner");
    hide_progress_bar();

    // Type
    qRegisterMetaType<std::vector<std::vector<scanfile>>>("std::vector<std::vector<scanfile>>");
    qRegisterMetaType<std::vector<std::pair<qint32, qint32>>>("std::vector<std::pair<qint32, qint32>>");

    //COnnections

    connect(ui->actionScan, SIGNAL(triggered()), this, SLOT(open_directory()));
    connect(ui->actionCancel, SIGNAL(triggered()), this, SLOT(cancel_pushed()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(delete_files()));
    // TODO: rewrite to SIGNAL and SLOT
    connect(scanner, &Scanner::send_amount, this, &MainWindow::recieve_amount);
    connect(scanner, &Scanner::new_message, this, &MainWindow::recieve_msg);
    connect(scanner, &Scanner::new_exception, this, &MainWindow::recieve_err);
    connect(scanner, &Scanner::send_results, this, &MainWindow::recieve_results);
    connect(scanner, &Scanner::update_progress_bar, this, &MainWindow::update_progress_bar);
    connect(this, &MainWindow::start_scan, scanner, &Scanner::scan);
    connect(this, &MainWindow::delete_files_sc, scanner, &Scanner::delete_files);
    connect(scanner, &Scanner::deleting_finished, this, &MainWindow::deleting_finished);
    connect(this, &MainWindow::cancel_process, scanner, &Scanner::cancel, Qt::DirectConnection);
    connect(scanner, &Scanner::process_started, this, &MainWindow::process_started);
    connect(scanner, &Scanner::process_finished, this, &MainWindow::process_finised);



    scanner->moveToThread(scanner_thread);
    scanner_thread->start();
}

MainWindow::~MainWindow()
{
    emit cancel_process();
    scanner_thread->quit();
    scanner_thread->wait();
    delete scanner_thread;
    delete ui;
}

void MainWindow::show_progress_bar() {
    ui->progressBar->setMaximum(0);
    ui->progressBar->reset();
    ui->progressBar->show();
}

void MainWindow::hide_progress_bar() {
    ui->progressBar->hide();
    ui->progressBar->reset();
    ui->progressBar->setMaximum(0);
}

void MainWindow::cancel_pushed() {
    hide_progress_bar();
    emit cancel_process(true);
}

void MainWindow::process_started() {
    show_progress_bar();
}

void MainWindow::process_finised() {
    hide_progress_bar();
}

void MainWindow::open_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", QDir::currentPath());
    if (!dir.isEmpty()) {
        emit cancel_process();

        setWindowTitle("Scanning...");
        ui->treeWidget->clear();
        ui->treeWidget->setSortingEnabled(false);
        show_progress_bar();
        timer.restart();

        emit start_scan(dir);
    }
}

void MainWindow::recieve_results(const std::vector<std::vector<scanfile>> &res) {
    ui->treeWidget->clear();
    for (qint32 i = 0; i < (qint32)res.size(); i++) {
        auto group_item = new QTreeWidgetItem(ui->treeWidget);
        group_item->setText(0, "Group");
        qint64 group_size = 0;
        int cnt = 0;
        for (const scanfile &file : res[i]) {
            if (file.is_deleted())
                continue;
            cnt++;
            auto file_item = new QTreeWidgetItem(group_item);
            file_item->setText(0, file.get_abspath());
            group_size += file.size();
            file_item->setText(1, QString::number(file.get_fid()));
            file_item->setText(2, QString::number(file.size()));
            file_item->setCheckState(3, Qt::Unchecked);
        }
        if (cnt > 1) {
            ui->treeWidget->addTopLevelItem(group_item);
            group_item->setText(1, QString::number(i));
            group_item->setText(2, QString::number(group_size));
        }
    }
    ui->statusBar->showMessage("Done, time elapsed: " + QString::number(timer.elapsed() / 1000.0) + QString("sec."));
    setWindowTitle("Duplicate Files Scanner");
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setEnabled(false);
}

void MainWindow::recieve_amount(const qint32 &cnt) {
    ui->progressBar->setRange(0, cnt);
    if (cnt > 0) {
       ui->statusBar->showMessage("Processing... Files processed: " + QString::number(cnt));
    } else {
        ui->statusBar->showMessage("Processing...");
    }
}

void MainWindow::recieve_msg(const QString &msg) {
    ui->plainTextEdit->appendPlainText("\n[NOTIFY] " + msg);
}

void MainWindow::recieve_err(const QString &msg) {
    ui->plainTextEdit->appendPlainText("\n[ERROR] " + msg);
}

void MainWindow::update_progress_bar(const qint32 &val) {
    ui->progressBar->setValue(val);
}

void MainWindow::deleting_finished() {
    ui->statusBar->showMessage("Deleting finished, time: " + QString::number(timer.elapsed() / 1000.0) + " sec.");
}

void MainWindow::delete_files() {
    std::vector<std::pair<qint32, qint32>> ids;
    QTreeWidgetItemIterator it(ui->treeWidget);
    setWindowTitle("Deleting");

    qint32 cid = 0;
    QTreeWidgetItemIterator cur_group(it);
    bool active = false;
    size_t group_actual_size;
    std::vector < QTreeWidgetItemIterator > to_delete;
    while (*it) {
        if ((*it)->text(0) == "Group") {
            cid =((*it)->text(1)).toInt();
            if (active && !group_actual_size) {
                to_delete.push_back(cur_group);
            }
            group_actual_size = 0;
            cur_group = QTreeWidgetItemIterator(it);
            active = true;
        } else {
            group_actual_size++;
            if ((*it)->checkState(3) == Qt::Checked) {
                ids.push_back(std::make_pair(cid, (qint32)((*it)->text(1)).toInt()));
                to_delete.push_back(it);
                group_actual_size--;
            }
        }
        it++;
    }
    if (active && !group_actual_size) {
        to_delete.push_back(cur_group);
    }

    for (auto &it : to_delete) {
        delete *it;
    }

    show_progress_bar();
    ui->progressBar->setMaximum(ids.size());
    ui->progressBar->setValue(0);
    ui->progressBar->setEnabled(true);
    emit cancel_process();
    ui->statusBar->showMessage("Processing...");
    emit delete_files_sc(ids);
}
