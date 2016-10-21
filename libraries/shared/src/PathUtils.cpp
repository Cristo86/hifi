//
//  PathUtils.cpp
//  libraries/shared/src
//
//  Created by Brad Hefta-Gaub on 12/15/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QCoreApplication>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include "PathUtils.h"
#include <QDebug>
#include <QDirIterator>
#include <QStandardPaths>
const QString& PathUtils::resourcesPath() {
#ifdef Q_OS_MAC
    static QString staticResourcePath = QCoreApplication::applicationDirPath() + "/../Resources/";
#elif defined (ANDROID)
    static QString staticResourcePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/resources/";
#else
    static QString staticResourcePath = QCoreApplication::applicationDirPath() + "/resources/";
#endif

    return staticResourcePath;
}


QString fileNameWithoutExtension(const QString& fileName, const QVector<QString> possibleExtensions) {
    QString fileNameLowered = fileName.toLower();
    foreach (const QString possibleExtension, possibleExtensions) {
        if (fileNameLowered.endsWith(possibleExtension.toLower())) {
            return fileName.left(fileName.count() - possibleExtension.count() - 1);
        }
    }
    return fileName;
}

QString findMostRecentFileExtension(const QString& originalFileName, QVector<QString> possibleExtensions) {
    QString sansExt = fileNameWithoutExtension(originalFileName, possibleExtensions);
    QString newestFileName = originalFileName;
    QDateTime newestTime = QDateTime::fromMSecsSinceEpoch(0);
    foreach (QString possibleExtension, possibleExtensions) {
        QString fileName = sansExt + "." + possibleExtension;
        QFileInfo fileInfo(fileName);
        if (fileInfo.exists() && fileInfo.lastModified() > newestTime) {
            newestFileName = fileName;
            newestTime = fileInfo.lastModified();
        }
    }
    return newestFileName;
}

QUrl defaultScriptsLocation() {
    // return "http://s3.amazonaws.com/hifi-public";
#ifdef Q_OS_WIN
    QString path = QCoreApplication::applicationDirPath() + "/scripts";
#elif defined(Q_OS_OSX)
    QString path = QCoreApplication::applicationDirPath() + "/../Resources/scripts";
#elif defined (ANDROID) 
    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/scripts";
#else
    QString path = QCoreApplication::applicationDirPath() + "/scripts";
#endif

#if defined(ANDROID) 
    return QUrl::fromLocalFile(path);
#else
    QFileInfo fileInfo(path);
    return QUrl::fromLocalFile(fileInfo.canonicalFilePath());    
#endif
}

void copyDirDeep(QString src, QString dst) {
    QDir dir = QDir::root();
    dir.mkpath(dst);
    QDirIterator it(src, QStringList() << "*", QDir::Files|QDir::AllDirs, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString f = it.next();
        QFileInfo fInfo(f);
        QString newDst = dst + (dst.endsWith("/")?"":"/") + fInfo.fileName();
        if (fInfo.isFile()) {
            QFile dfile(f);
            if (dfile.exists(f))
            {
                if (dfile.copy(newDst)) {
                    QFile::setPermissions(newDst, QFile::ReadOwner);
                } else {
                    QFile::setPermissions(newDst, QFile::ReadOwner);
                    // sometimes copy returns false but it worked anyway
                    //qWarning() << "Could not copy to " << newDst;
                }
            }
        } else if (fInfo.isDir() ) {
            copyDirDeep(f, newDst);
        }
    }
}
