//
// Created by xpech on 01.03.19.
//

#include <QtCore/QDirIterator>
#include <QFile>
#include "scanner.h"

Scanner::Scanner() : directory(""), _canceled(false), progress(0), cid(0) {}


void Scanner::scan(const QString &dir) {
    clear_data();

    directory = dir;
    emit new_message("Started scanning");
    index_files();
    emit new_message("Finished indexation");
    emit new_message(QString::number(files.size()) + " files total");
    find_dupes();
    emit send_results(dupes);
    emit new_message("Done");
    emit process_finished();
}

void Scanner::clear_data() {
    files.clear();
    dupes.clear();
    info2id.clear();
    _canceled.store(false);
    progress = cid = 0;
    directory = "";
}

void Scanner::index_files() {
    qint32 result = 0;
    QDirIterator it(directory, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    QString path;
    emit process_started();
    while (it.hasNext()) {
        path = it.next();
        try {
            QFileInfo info(path);
            if (info.isFile() && (info.permissions() & QFileDevice::ReadUser)) {
                files.emplace_back(path, info.absoluteFilePath(), info.size());
                result++;
            }
        } catch (std::exception const &err) {
            qWarning() << err.what();
        }

        if (_canceled.load()) {
            emit new_message("Canceled");
            return;
        }
    }
    emit send_amount(result);
}

void Scanner::progress_check(const int &index, const int &size) {
    qint32 new_progress = (((double) index / size) * 100.0);
    if (new_progress > progress) {
        progress = new_progress;
        emit update_progress_bar(progress);
    }
}

void Scanner::find_dupes() {
    int step = 0, index = 0;
    std::vector<std::vector<scanfile>> dupes;
    for (scanfile &file : files) {
        try {
            auto res = info2id.insert(std::make_pair(std::make_pair(file.hash(), file.size()), cid));
            if (res.second) {
                cid++;
                dupes.emplace_back();
            }
            file.set_fid((qint32)dupes[res.first->second].size());
            dupes[res.first->second].push_back(file);
            step++;
            index++;
            if (step == PROGRESS_SEND_INTERVAL) {
                step = 0;
                progress_check(index, files.size());
            }
        } catch (std::runtime_error &err) {
            emit new_exception(err.what() + (" " + file.get_path()));
        }
    }

    for (const std::vector <scanfile> &group : dupes) {
        if (group.size() > 1) {
            this->dupes.push_back(group);
        }
    }
}

void Scanner::delete_files(const std::vector<std::pair<qint32, qint32>> &ids) {
    int index = 0;
    _canceled.store(false);

    emit process_started();
    emit new_message("Started deleting");
    for (const std::pair<qint32, qint32> &id : ids) {
        if (_canceled.load()) {
            emit new_message("Canceled");
            break;
        }

        if (!QFile::remove(dupes[id.first][id.second].get_abspath())) {
            emit new_exception("Error occured while deleting " + dupes[id.first][id.second].get_abspath());
        } else {
            dupes[id.first][id.second].set_deleted(true);
        }
        progress_check(index, ids.size());
    }
    progress_check(ids.size(), ids.size());
    emit new_message("Deleting is done");
    emit deleting_finished();
    emit process_finished();
}

void Scanner::cancel(const bool &show_msg) {
    if (show_msg)
        emit new_message("Cancelling...");
    _canceled.store(true);
}
