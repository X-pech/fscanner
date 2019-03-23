#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QThread>
#include <scanner.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // From self
    void open_directory();
    void delete_files();
    void cancel_pushed();
    // From scanner
    void recieve_results(const std::vector<std::vector<scanfile>> &res);
    void update_progress_bar(const qint32 &val);
    void recieve_amount(const qint32 &cnt);
    void recieve_msg(const QString &msg);
    void recieve_err(const QString &msg);
    void deleting_finished();
    void process_started();
    void process_finised();

signals:
    void start_scan(const QString &dir);
    void delete_files_sc(const std::vector<std::pair<qint32, qint32>> &ids);
    void cancel_process(const bool &show_msg = false);

private:
    Ui::MainWindow *ui;
    QThread *scanner_thread;
    Scanner *scanner;
    QTime timer;

    void show_progress_bar();
    void hide_progress_bar();

};

#endif // MAINWINDOW_H
