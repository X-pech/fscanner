//
// Created by xpech on 01.03.19.
//

#ifndef FSCANNER_SCANNER_H
#define FSCANNER_SCANNER_H

#include <vector>
#include <string>
#include <algorithm>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <scanfile.h>
#include <atomic>

class Scanner : public QObject {
Q_OBJECT;
public:
    Scanner();
    //void delete_checked(const );

public slots:
    void scan(const QString &dir);
    void cancel(const bool &show_msg);
    //void set_directory(const QString &dir);
    void delete_files(const std::vector<std::pair<qint32, qint32>> &ids);

signals:
    void send_amount(const qint32 &cnt);
    void new_message(const QString &msg);
    void new_exception(const QString &msg);
    void send_results(const std::vector<std::vector<scanfile>> &res);
    void update_progress_bar(qint32 value);
    void deleting_finished();

private:
    const int PROGRESS_SEND_INTERVAL = 5;

    void index_files();
    void find_dupes();
    void progress_check(const int &index, const int &size);
    void clear_data();

    QDir directory;
    std::atomic<bool> _canceled;
    std::map<std::pair<QString, qint64>, qint32> info2id;
    qint32 progress, cid;
    std::vector<scanfile> files;
    std::vector<std::vector<scanfile>> dupes;

//public signals:

};


#endif //FSCANNER_SCANNER_H
