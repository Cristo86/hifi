//
//  main.cpp
//  interface/src
//
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <thread>

#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QLocalSocket>
#include <QLocalServer>
#include <QSettings>
#include <QSharedMemory>
#include <QTranslator>

#include <gl/OpenGLVersionChecker.h>
#include <SharedUtil.h>

#ifndef ANDROID
#include <steamworks-wrapper/SteamClient.h>
#endif

#include "AddressManager.h"
#include "Application.h"
#include "InterfaceLogging.h"
#include "UserActivityLogger.h"
#include "MainWindow.h"
#include <QtCore/QProcess>

#ifdef HAS_BUGSPLAT
#include <BuildInfo.h>
#include <BugSplat.h>
#include <CrashReporter.h>
#endif

int main(int argc, const char* argv[]) {
    qDebug() << "First main line";
#if HAS_BUGSPLAT
    static QString BUG_SPLAT_DATABASE = "interface_alpha";
    static QString BUG_SPLAT_APPLICATION_NAME = "Interface";
    CrashReporter crashReporter { BUG_SPLAT_DATABASE, BUG_SPLAT_APPLICATION_NAME, BuildInfo::VERSION };
#endif
        qDebug() << "A1";

    disableQtBearerPoll(); // Fixes wifi ping spikes
        qDebug() << "A2";
    
    QString applicationName = "High Fidelity Interface - myusername";  //qgetenv("USERNAME");

    bool instanceMightBeRunning = true;
        qDebug() << "A3";

    QStringList arguments;
    for (int i = 0; i < argc; ++i) {
        arguments << argv[i];
    }


#ifdef Q_OS_WIN
    // Try to create a shared memory block - if it can't be created, there is an instance of
    // interface already running. We only do this on Windows for now because of the potential
    // for crashed instances to leave behind shared memory instances on unix.
    QSharedMemory sharedMemory { applicationName };
    instanceMightBeRunning = !sharedMemory.create(1, QSharedMemory::ReadOnly);
#endif
        qDebug() << "A4";

    if (instanceMightBeRunning) {
                qDebug() << "A5";

        // Try to connect and send message to existing interface instance
        QLocalSocket socket;
                            qDebug() << "A51" << applicationName;

        socket.connectToServer(applicationName);
                            qDebug() << "A53";

        static const int LOCAL_SERVER_TIMEOUT_MS = 500;
                            qDebug() << "A55";

        // Try to connect - if we can't connect, interface has probably just gone down
        if (socket.waitForConnected(LOCAL_SERVER_TIMEOUT_MS)) {
                            qDebug() << "A6";

            QCommandLineParser parser;
            QCommandLineOption urlOption("url", "", "value");
            parser.addOption(urlOption);
            parser.process(arguments);

            if (parser.isSet(urlOption)) {
                QUrl url = QUrl(parser.value(urlOption));
                if (url.isValid() && url.scheme() == HIFI_URL_SCHEME) {
                    qDebug() << "Writing URL to local socket";
                    socket.write(url.toString().toUtf8());
                    if (!socket.waitForBytesWritten(5000)) {
                        qDebug() << "Error writing URL to local socket";
                    }
                }
            }

            socket.close();

            qDebug() << "Interface instance appears to be running, exiting";
            qDebug() << "A99"; 
            return EXIT_SUCCESS;
        }
        qDebug() << "A7";

#ifdef Q_OS_WIN
        qDebug() << "A8";
        return EXIT_SUCCESS;
#endif
    }
        qDebug() << "A9";

    // Check OpenGL version.
    // This is done separately from the main Application so that start-up and shut-down logic within the main Application is
    // not made more complicated than it already is.
    bool override = false;
    QJsonObject glData;
    {
        QApplication app2(argc, const_cast<char**>(argv));
        qDebug("A10 - %d" , argc);
        qDebug("A10 - %s" , argv);
        OpenGLVersionChecker openGLVersionChecker(argc, const_cast<char**>(argv));
        qDebug() << "A11";
        bool valid = true;
        glData = openGLVersionChecker.checkVersion(valid, override);
        qDebug() << "A12";
        if (!valid) {
            qDebug() << "A14";
            if (override) {
                qDebug() << "A15";
                auto glVersion = glData["version"].toString();
                qCDebug(interfaceapp, "Running on insufficient OpenGL version: %s.", glVersion.toStdString().c_str());
                qDebug() << "A16";
            } else {
                qDebug() << "A17";
                qCDebug(interfaceapp, "Early exit due to OpenGL version.");
                qDebug() << "A18";
                return 0;
            }
        }
    }

    QCommandLineParser parser;
    QCommandLineOption runServerOption("runServer", "Whether to run the server");
    QCommandLineOption serverContentPathOption("serverContentPath", "Where to find server content", "serverContentPath");
    parser.addOption(runServerOption);
    parser.addOption(serverContentPathOption);
    parser.parse(arguments);
    if (parser.isSet(runServerOption)) {
        QString applicationDirPath = QFileInfo(arguments[0]).path();
        QString serverPath = applicationDirPath + "/server-console/server-console.exe";
        qDebug() << "Application dir path is: " << applicationDirPath;
        qDebug() << "Server path is: " << serverPath;
        QStringList args;
        if (parser.isSet(serverContentPathOption)) {
            QString serverContentPath = QFileInfo(arguments[0]).path() + "/" + parser.value(serverContentPathOption);
            args << "--" << "--contentPath" << serverContentPath;
        }
        qDebug() << QFileInfo(arguments[0]).path();
        qDebug() << QProcess::startDetached(serverPath, args);

        // Sleep a short amount of time to give the server a chance to start
        usleep(2000000);
    }

    QElapsedTimer startupTime;
    startupTime.start();

    // Debug option to demonstrate that the client's local time does not
    // need to be in sync with any other network node. This forces clock
    // skew for the individual client
    const char* CLOCK_SKEW = "--clockSkew";
    const char* clockSkewOption = getCmdOption(argc, argv, CLOCK_SKEW);
    if (clockSkewOption) {
        qint64 clockSkew = atoll(clockSkewOption);
        usecTimestampNowForceClockSkew(clockSkew);
        qCDebug(interfaceapp) << "clockSkewOption=" << clockSkewOption << "clockSkew=" << clockSkew;
    }
    qDebug() << "about to call initplugins ";

    // Oculus initialization MUST PRECEDE OpenGL context creation.
    // The nature of the Application constructor means this has to be either here,
    // or in the main window ctor, before GL startup.
    Application::initPlugins(arguments);

#ifndef ANDROID
        qDebug() << "2";

    SteamClient::init();
        qDebug() << "3";
#endif

    int exitCode;
    {
        qDebug() << "4";
        QSettings::setDefaultFormat(QSettings::IniFormat);
        qDebug() << "5: building app";
        Application app(argc, const_cast<char**>(argv), startupTime);
        qDebug() << "6: building app";

        // If we failed the OpenGLVersion check, log it.
        if (override) {
            qDebug() << "7";

            auto accountManager = DependencyManager::get<AccountManager>();
            if (accountManager->isLoggedIn()) {
                UserActivityLogger::getInstance().insufficientGLVersion(glData);
            } else {
                QObject::connect(accountManager.data(), &AccountManager::loginComplete, [glData](){
                    static bool loggedInsufficientGL = false;
                    if (!loggedInsufficientGL) {
                        UserActivityLogger::getInstance().insufficientGLVersion(glData);
                        loggedInsufficientGL = true;
                    }
                });
            }
        }
            qDebug() << "8";

        // Setup local server
        QLocalServer server { &app };

        // We failed to connect to a local server, so we remove any existing servers.
        server.removeServer(applicationName);
        server.listen(applicationName);

        QObject::connect(&server, &QLocalServer::newConnection, &app, &Application::handleLocalServerConnection, Qt::DirectConnection);
            qDebug() << "9";

#ifdef HAS_BUGSPLAT
        auto accountManager = DependencyManager::get<AccountManager>();
        crashReporter.mpSender.setDefaultUserName(qPrintable(accountManager->getAccountInfo().getUsername()));
        QObject::connect(accountManager.data(), &AccountManager::usernameChanged, &app, [&crashReporter](const QString& newUsername) {
            crashReporter.mpSender.setDefaultUserName(qPrintable(newUsername));
        });

        // BugSplat WILL NOT work with file paths that do not use OS native separators.
        auto logger = app.getLogger();
        auto logPath = QDir::toNativeSeparators(logger->getFilename());
        crashReporter.mpSender.sendAdditionalFile(qPrintable(logPath));

        QMetaObject::Connection connection;
        connection = QObject::connect(logger, &FileLogger::rollingLogFile, &app, [&crashReporter, &connection](QString newFilename) {
            // We only want to add the first rolled log file (the "beginning" of the log) to BugSplat to ensure we don't exceed the 2MB
            // zipped limit, so we disconnect here.
            QObject::disconnect(connection);
            auto rolledLogPath = QDir::toNativeSeparators(newFilename);
            crashReporter.mpSender.sendAdditionalFile(qPrintable(rolledLogPath));
        });
#endif
            qDebug() << "10";

        printSystemInformation();
            qDebug() << "11";

        QTranslator translator;
        translator.load("i18n/interface_en");
        app.installTranslator(&translator);

        qCDebug(interfaceapp, "Created QT Application.");
        exitCode = app.exec();
        server.close();
    }

    Application::shutdownPlugins();

#ifndef ANDROID
    SteamClient::shutdown();
#endif

    qCDebug(interfaceapp, "Normal exit.");
#if !defined(DEBUG) && !defined(Q_OS_LINUX)
    // HACK: exit immediately (don't handle shutdown callbacks) for Release build
    _exit(exitCode);
#endif
    return exitCode;
}
