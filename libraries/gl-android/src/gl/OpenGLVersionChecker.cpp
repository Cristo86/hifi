//
//  OpenGLVersionChecker.cpp
//  libraries/gl/src/gl
//
//  Created by David Rowe on 28 Jan 2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "OpenGLVersionChecker.h"
#include "Config.h"
#include <QDebug>
#include <mutex>

#include <QtCore/QRegularExpression>
#include <QtCore/QJsonObject>
#include <QtWidgets/QMessageBox>
#include <QtOpenGL/QGLWidget>

#include "GLHelpers.h"

#define MINIMUM_GL_VERSION 0x0301

OpenGLVersionChecker::OpenGLVersionChecker(int& argc, char** argv) :
    QApplication(argc, argv)
{
        qDebug() << "X1";

}

const QGLFormat& getDefaultGLFormat() {
    // Specify an OpenGL 3.3 format using the Core profile.
    // That is, no old-school fixed pipeline functionality
    static QGLFormat glFormat;
    static std::once_flag once;
    std::call_once(once, [] {
        setGLFormatVersion(glFormat);
        glFormat.setProfile(QGLFormat::CoreProfile); // Requires >=Qt-4.8.0
        glFormat.setSampleBuffers(false);
        glFormat.setDepth(false);
        glFormat.setStencil(false);
        QGLFormat::setDefaultFormat(glFormat);
    });
    return glFormat;
}


QJsonObject OpenGLVersionChecker::checkVersion(bool& valid, bool& override) {
    valid = true;
    override = false;
        qDebug() << "B1";

    QGLWidget* glWidget = new QGLWidget();
        qDebug() << "B2";
    valid = glWidget->isValid();
    qDebug() << "B3";
    // Inform user if no OpenGL support
    if (!valid) {
                qDebug() << "B4";

        QMessageBox messageBox;
        messageBox.setWindowTitle("Missing OpenGL Support");
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(QString().sprintf("Your system does not support OpenGL, Interface cannot run."));
        messageBox.setInformativeText("Press OK to exit.");
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.setDefaultButton(QMessageBox::Ok);
                qDebug() << "B5";

        messageBox.exec();
                qDebug() << "B6";

        return QJsonObject();
    }
            qDebug() << "B7";

    // Retrieve OpenGL version
    // glWidget->initializeGL();
    glWidget->makeCurrent();
            qDebug() << "B8";

    QJsonObject glData = getGLContextData();
            qDebug() << "B9";

    delete glWidget;

    // Compare against minimum
    // The GL_VERSION string begins with a version number in one of these forms:
    // - major_number.minor_number 
    // - major_number.minor_number.release_number
    // Reference: https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glGetString.xml
    const QString version { "version" };
qDebug() << "B10";
    QString glVersion = glData[version].toString();
    qDebug() << "B11" << glVersion;
    QStringList versionParts = glVersion.split(QRegularExpression("[\\.\\s]"));
    qDebug() << "B11bis" << versionParts;
    int majorNumber = versionParts[0].toInt();
    int minorNumber = versionParts[1].toInt();
    int minimumMajorNumber = (MINIMUM_GL_VERSION >> 8);
    int minimumMinorNumber = (MINIMUM_GL_VERSION & 0xFF);
    qDebug ("FOUND %d.%d MIN %d.%d", majorNumber, minorNumber, minimumMajorNumber, minimumMinorNumber);
    valid = (majorNumber > minimumMajorNumber
        || (majorNumber == minimumMajorNumber && minorNumber >= minimumMinorNumber));

    // Prompt user if below minimum
    if (!valid) {
        QMessageBox messageBox;
        messageBox.setWindowTitle("OpenGL Version Too Low");
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(QString().sprintf("Your OpenGL version of %i.%i is lower than the minimum of %i.%i.",
            majorNumber, minorNumber, minimumMajorNumber, minimumMinorNumber));
        messageBox.setInformativeText("Press OK to exit; Ignore to continue.");
        messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Ignore);
        messageBox.setDefaultButton(QMessageBox::Ok);
        override = messageBox.exec() == QMessageBox::Ignore;
    }

    return glData;
}
