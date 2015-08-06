/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef APPMODEL_H
#define APPMODEL_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>
#include <QtQml/qqml.h>
#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlPropertyMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <qtconcurrentrun.h>
#include "bloom_filter.h"

#if defined(Q_OS_ANDROID)
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QException>

class InterfaceConnFailedException : public QException
{
public:
    void raise() const { throw *this; }
    InterfaceConnFailedException *clone() const { return new InterfaceConnFailedException(*this); }
};
#elif defined(Q_OS_WINDOWS)
#elif defined(Q_OS_DARWIN)
#else
#endif


class AppModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int applicationWidth READ applicationWidth WRITE setApplicationWidth NOTIFY applicationWidthChanged)
    Q_PROPERTY(bool isMobile READ isMobile CONSTANT)
    Q_PROPERTY(QObject *constants READ constants CONSTANT)

    Q_PROPERTY(bool isPortraitMode READ isPortraitMode WRITE setIsPortraitMode NOTIFY portraitModeChanged)
    Q_PROPERTY(qreal ratio READ ratio CONSTANT)
    Q_PROPERTY(qreal hMargin READ hMargin NOTIFY hMarginChanged)
    Q_PROPERTY(qreal sliderHandleWidth READ sliderHandleWidth CONSTANT)
    Q_PROPERTY(qreal sliderHandleHeight READ sliderHandleHeight CONSTANT)
    Q_PROPERTY(qreal sliderGapWidth READ sliderGapWidth CONSTANT)

    Q_PROPERTY(int defectCount READ defectCount CONSTANT)
    Q_PROPERTY(QStringList defectFiles READ defectFiles() CONSTANT)
    Q_PROPERTY(QString currentScanFile READ currentScanFile WRITE setCurrentScanFile NOTIFY currentScanFileChanged)
    Q_PROPERTY(int currentScanPercent READ currentScanPercent NOTIFY currentScanPercentChanged)
    Q_PROPERTY(NOTIFY fileCountComplete)
    Q_PROPERTY(NOTIFY fileCountStart)
    // email
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)


public:
    AppModel();
    void reset();
    bool isMobile() const { return m_isMobile; }

    QQmlPropertyMap *constants() const { return m_constants; }
    int applicationWidth() const { return m_applicationWidth; }
    void setApplicationWidth(const int newWidth);

    int defectCount() const { return m_defectFiles.length(); }
    const QStringList & defectFiles() const { return m_defectFiles; }

    QString currentScanFile() const { return m_currentScanFile; }
    void setCurrentScanFile(const QString file);

    Q_INVOKABLE void scanDefectFile();
    Q_INVOKABLE void cancelScan();

    static void scanDefectFile2(AppModel *self, QStringList strDirs, QStringList strFilters);

    bool isPortraitMode() const { return m_isPortraitMode; }
    void setIsPortraitMode(const bool newMode);
    qreal hMargin() const { return m_hMargin; }
    qreal ratio() const { return m_ratio; }
    qreal sliderHandleHeight()  { return m_sliderHandleHeight; }
    qreal sliderGapWidth()  { return m_sliderGapWidth; }
    qreal sliderHandleWidth()  { return m_sliderHandleWidth; }

    int currentScanPercent() const;
    int scanCount(){ return m_scanCount; }
    void setScanCount(int count);

    void queryDeviceInfo();

    Q_INVOKABLE void queryScanData(const QString os,
                                   const QString version,
                                   const QString vendor,
                                   const QString model);
    const QStringList & appFiles() { return m_appFiles; }

    void setEmail(const QString &email);
    const QString& email() const{ return m_email; }

signals:
    void emailChanged();

private slots:
    void updateEmail();

protected slots:
    void notifyPortraitMode(Qt::ScreenOrientation);

private slots:
    void replyFinished(QNetworkReply *reply);

protected:
    qreal getSizeWithRatio(const qreal height) { return ratio() * height; }

signals:
    void applicationWidthChanged();
    void portraitModeChanged();
    void hMarginChanged();
    void currentScanFileChanged(const QString fileName);
    void currentScanPercentChanged(int percent);
    void foundCitiesChanged();
    void waitForScanDataQueryReply(const QString message);
    void errorOnQueryScanData(const QString errorMessage);
    void fileCountComplete();
    void fileCountStart();

private:
    int m_applicationWidth;
    QQmlPropertyMap *m_constants;
    bool m_isPortraitMode;
    bool m_isMobile;
    qreal m_ratio;
    qreal m_hMargin;
    qreal m_sliderHandleHeight, m_sliderHandleWidth, m_sliderGapWidth;
    QNetworkAccessManager *manager;

    int m_scanCount;
    QString m_currentScanFile;
    QStringList m_appFiles;
    QStringList m_defectFiles;
    QStringList m_defectFilesSha1;
    QJsonObject m_scanData;

    QString m_productOS;
    QString m_productModel;
    QString m_productName;
    QString m_productVersion;
    QString m_productManufacturer;

    QFuture<void> m_futureScan;

    bloom_filter m_bloomFilter;

    QString m_email;
};

//! [4]

#endif // APPMODEL_H
