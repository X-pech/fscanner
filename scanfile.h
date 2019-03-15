//
// Created by xpech on 01.03.19.
//

#ifndef FSCANNER_SCANFILE_H
#define FSCANNER_SCANFILE_H

#include <QString>

class scanfile {
public:
    explicit scanfile(const QString &path = "", const QString &abspath = "", qint64 size = 0, const qint32 &fid = 0);

    scanfile &operator=(const scanfile &f) = default;

    QString get_path() const {
        return _path;
    }

    QString get_abspath() const {
        return _abspath;
    }

    qint64 size() const {
        return _size;
    }

    QString hash();

    bool is_hashed() {
        return _hashed;
    }

    bool is_deleted() const {
        return _deleted;
    }

    void set_deleted(const bool &deleted) {
        _deleted = deleted;
    }

    qint32 get_fid() const {
        return fid;
    }

    void set_fid(const qint32 &c) {
        fid = c;
    }

private:
    QString _path;
    QString _abspath;
    qint64 _size;
    qint32 fid;
    QString _hash;
    bool _hashed;
    bool _deleted;

    QByteArray cnt_hash();
};


#endif //FSCANNER_SCANFILE_H
