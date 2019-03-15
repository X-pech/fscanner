//
// Created by xpech on 01.03.19.
//

#include "scanfile.h"
#include <QFile>
#include <QCryptographicHash>

scanfile::scanfile(const QString &path, const QString &abspath, qint64 size, const qint32 &fid) :
        _path(path), _abspath(abspath), _size(size), fid(fid) {
    _hash = "";
    _hashed = false;
    _deleted = false;
}

QString scanfile::hash() {
    if (_hashed) return _hash;
    _hashed = true;
    return _hash = cnt_hash();
}

QByteArray scanfile::cnt_hash() {
    QFile f(_abspath);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&f)) {
            return hash.result();
        } else {
            throw std::runtime_error("An error occured while reading the file");
        }
    } else {
        throw std::runtime_error("An error occured while opening the file");
    }
    return QByteArray();
}