/*
 * Copyright (C) 2013-2015 Atmel Corporation.
 *
 * This file is licensed under the terms of the Atmel LIMITED License Agreement
 * as written in the COPYING file that is delivered in the project root directory.
 *
 * DISCLAIMER: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 * See the COPYING license file for more details.
 */

#include "qmlapplicationviewer.h"

#include <QDir>
#include <QFileInfo>
#include <QApplication>

#include <QCoreApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDebug>
#include <qplatformdefs.h> // MEEGO_EDITION_HARMATTAN

#ifdef HARMATTAN_BOOSTER
#include <MDeclarativeCache>
#endif

#if defined(QMLJSDEBUGGER) && QT_VERSION < 0x040800
#include <qt_private/qdeclarativedebughelper_p.h>

#if !defined(NO_JSDEBUGGER)
#include <jsdebuggeragent.h>
#endif

#if !defined(NO_QMLOBSERVER)
#include <QQuickViewobserver.h>
#endif

// Enable debugging before any QDeclarativeEngine is created
struct QmlJsDebuggingEnabler
{
    QmlJsDebuggingEnabler()
    {
        QDeclarativeDebugHelper::enableDebugging();
    }
};

// Execute code in constructor before first QDeclarativeEngine is instantiated
static QmlJsDebuggingEnabler enableDebuggingHelper;
#endif // QMLJSDEBUGGER


#if (QT_VERSION < 0x050000)
class QmlApplicationViewerPrivate
{
    QString mainQmlFile;
    friend class QmlApplicationViewer;
    static QString adjustPath(const QString &path);
};

QString QmlApplicationViewerPrivate::adjustPath(const QString &path)
{
#ifdef Q_OS_MAC
    if (!QDir::isAbsolutePath(path))
        return QString::fromLatin1("%1/../Resources/%2")
                .arg(QCoreApplication::applicationDirPath(), path);
#elif defined(Q_OS_BLACKBERRY)
    if (!QDir::isAbsolutePath(path))
        return QString::fromLatin1("app/native/%1").arg(path);
#elif !defined(Q_OS_ANDROID)
    QString pathInInstallDir =
            QString::fromLatin1("%1/../%2").arg(QCoreApplication::applicationDirPath(), path);
    if (QFileInfo(pathInInstallDir).exists())
        return pathInInstallDir;
    pathInInstallDir =
            QString::fromLatin1("%1/%2").arg(QCoreApplication::applicationDirPath(), path);
    if (QFileInfo(pathInInstallDir).exists())
        return pathInInstallDir;
#endif
    return path;
}
#endif // (QT_VERSION < 0x050000)


QmlApplicationViewer::QmlApplicationViewer(QWindow *parent)
    : QQuickView(parent)
    #if (QT_VERSION < 0x050000)
    , d(new QmlApplicationViewerPrivate())
    #endif
{
    connect(engine(), SIGNAL(quit()), SLOT(close()));
    setResizeMode(QQuickView::SizeRootObjectToView);

    // Qt versions prior to 4.8.0 don't have QML/JS debugging services built in
#if defined(QMLJSDEBUGGER) && QT_VERSION < 0x040800
#if !defined(NO_JSDEBUGGER)
    new QmlJSDebugger::JSDebuggerAgent(engine());
#endif
#if !defined(NO_QMLOBSERVER)
    new QmlJSDebugger::QQuickViewObserver(this, this);
#endif
#endif
}

QmlApplicationViewer::~QmlApplicationViewer()
{
#if (QT_VERSION < 0x050000)
    delete d;
#endif // (QT_VERSION < 0x050000)
}

QmlApplicationViewer *QmlApplicationViewer::create()
{
    return new QmlApplicationViewer();
}

void QmlApplicationViewer::setMainQmlFile(const QString &file)
{
#if (QT_VERSION >= 0x050000)
    setSource(file);
#else
    d->mainQmlFile = QmlApplicationViewerPrivate::adjustPath(file);
#ifdef Q_OS_ANDROID
    setSource(QUrl(QLatin1String("assets:/")+d->mainQmlFile));
#else
    setSource(QUrl::fromLocalFile(d->mainQmlFile));
#endif
#endif // (QT_VERSION >= 0x050000)
}

void QmlApplicationViewer::addImportPath(const QString &path)
{
#if (QT_VERSION >= 0x050000)
    engine()->addImportPath(path);
#else
    engine()->addImportPath(QmlApplicationViewerPrivate::adjustPath(path));
#endif // (QT_VERSION >= 0x050000)
}

void QmlApplicationViewer::setOrientation(ScreenOrientation orientation)
{
    Q_UNUSED(orientation)
}

void QmlApplicationViewer::showExpanded()
{
#if defined(MEEGO_EDITION_HARMATTAN) || defined(Q_WS_SIMULATOR) || defined(Q_OS_QNX)
    showFullScreen();
#elif defined(Q_WS_MAEMO_5)
    showMaximized();
#else
    show();
#endif
}

QApplication *createApplication(int &argc, char **argv)
{
#ifdef HARMATTAN_BOOSTER
    return MDeclarativeCache::qApplication(argc, argv);
#else
    return new QApplication(argc, argv);
#endif
}
