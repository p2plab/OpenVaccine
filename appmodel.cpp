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

#include <QtCore/qmath.h>
#include <QtCore/QRegExp>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include <openssl/pkcs7.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

#if defined(Q_OS_ANDROID)
#include <sys/cdefs.h>
#include <sys/system_properties.h>
#endif

#include "appmodel.h"
#include "sha1.h"


const char sha1_new[] = {
   0xC1, 0xF0, 0x4E, 0x3A, 0x74, 0x05, 0xD9, 0xCF,
   0xA2, 0x38, 0x25, 0x97, 0x30, 0xF0, 0x96, 0xA1,
   0x7F, 0xCF, 0x2A, 0x4F
};

const char sha1_old[] = {
   0x0A, 0xF4, 0xAB, 0xF5, 0x46, 0xFA, 0xD5, 0xA4,
   0xB2, 0xA4, 0x6A, 0x33, 0x64, 0x0B, 0xBE, 0x4F,
   0x61, 0xDD, 0x2C, 0x56
};

AppModel::AppModel()
{
    m_isMobile = false;
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_BLACKBERRY)
    m_isMobile = true;
#endif

    m_constants = new QQmlPropertyMap(this);
    m_constants->insert(QLatin1String("isMobile"), QVariant(m_isMobile));
    m_constants->insert(QLatin1String("errorLoadingImage"), QVariant(tr("Error loading image - Host not found or unreachable")));

    queryDeviceInfo();

    m_constants->insert(QLatin1String("productOS"), QVariant(m_productOS));
    m_constants->insert(QLatin1String("productVersion"), QVariant(m_productVersion));
    m_constants->insert(QLatin1String("productModel"), QVariant(m_productModel));
    m_constants->insert(QLatin1String("productName"), QVariant(m_productName));
    m_constants->insert(QLatin1String("productManufacturer"), QVariant(m_productManufacturer));

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    QRect rect = qApp->primaryScreen()->geometry();
    m_ratio = m_isMobile ? qMin(qMax(rect.width(), rect.height())/1136. , qMin(rect.width(), rect.height())/640.) : 1;
    m_sliderHandleWidth = getSizeWithRatio(70);
    m_sliderHandleHeight = getSizeWithRatio(87);
    m_sliderGapWidth = getSizeWithRatio(100);
    m_isPortraitMode = m_isMobile ? rect.height() > rect.width() : false;
    m_hMargin =  m_isPortraitMode ? 20 * ratio() : 50 * ratio();
    m_applicationWidth = m_isMobile ? rect.width() : 1120;

    m_constants->insert(QLatin1String("rowDelegateHeight"), QVariant(getSizeWithRatio(118)));

    m_currentScanFile = "";
    m_scanCount = 0;

    // initialize bloom filter
    bloom_parameters parameters;
    // How many elements roughly do we expect to insert?
    parameters.projected_element_count = 1000;
    // Maximum tolerable false positive probability? (0,1)
    parameters.false_positive_probability = 0.0001; // 1 in 10000
    // Simple randomizer (optional)
    parameters.random_seed = 0xA5A5A5A5;
    if (!parameters)
    {
       qCritical() << "Error - Invalid set of bloom filter parameters!";
       qApp->exit();
    }

    parameters.compute_optimal_parameters();

    //Instantiate Bloom Filter
    m_bloomFilter = bloom_filter(parameters);

    if (m_isMobile)
        connect(qApp->primaryScreen(), SIGNAL(primaryOrientationChanged(Qt::ScreenOrientation)), this, SLOT(notifyPortraitMode(Qt::ScreenOrientation)));

    connect(this, SIGNAL(emailChanged()),this,SLOT(updateEmail()));

    queryScanData(m_productOS,m_productVersion,m_productManufacturer, m_productModel);
}

void AppModel::queryDeviceInfo()
{
#if defined(Q_OS_ANDROID)
    {
    char version[PROP_NAME_MAX+1];
    char model[PROP_VALUE_MAX+1];
    char name[PROP_VALUE_MAX+1];
    char manufacturer[PROP_VALUE_MAX+1];

    __system_property_get("ro.build.version.release", version);
    __system_property_get("ro.product.model", model);
    __system_property_get("ro.product.name", name);
    __system_property_get("ro.product.manufacturer", manufacturer);

    m_productOS = "android";
    m_productVersion = version;
    m_productModel = model;
    m_productName = name;
    m_productManufacturer = manufacturer;
    }
#elif defined(Q_OS_WINDOWS)
    m_productOS = "WIN32";
    m_productVersion ="unknow";
    m_productModel = "unknown";
    m_productName = "unknown";
    m_productManufacturer = "unknown";
#elif defined(Q_OS_DARWIN)
    m_productOS = "DARWIN";
    m_productVersion ="unknow";
    m_productModel = "unknown";
    m_productName = "unknown";
    m_productManufacturer = "unknown";
#else
    Q_ASSERT(false, "", "not support os!");
#endif
}

void AppModel::setCurrentScanFile(QString file)
{
    if (file != m_currentScanFile)
    {
        m_currentScanFile = file;
        emit currentScanFileChanged(m_currentScanFile);
    }
}

void AppModel::setApplicationWidth(const int newWidth)
{
    if (newWidth != m_applicationWidth) {
        m_applicationWidth = newWidth;
        emit applicationWidthChanged();
    }
}

void AppModel::notifyPortraitMode(Qt::ScreenOrientation orientation)
{
    switch (orientation) {
    case Qt::LandscapeOrientation:
    case Qt::InvertedLandscapeOrientation:
        setIsPortraitMode(false);
        break;
    case Qt::PortraitOrientation:
    case Qt::InvertedPortraitOrientation:
        setIsPortraitMode(true);
        break;
    default:
        break;
    }
}

void AppModel::setIsPortraitMode(const bool newMode)
{
    if (m_isPortraitMode != newMode) {
        m_isPortraitMode = newMode;
        m_hMargin = m_isPortraitMode ? 20 * ratio() : 50 * ratio();
        emit portraitModeChanged();
        emit hMarginChanged();
    }
}

void AppModel::queryScanData(const QString os,
                             const QString version,
                             const QString vendor,
                             const QString model)
{
    if (os.isEmpty()||
        version.isEmpty() ||
        vendor.isEmpty() ||
        model.isEmpty())
        return;

    // In order to use yr.no weather data service, refer to their terms
    // and conditions of use. http://om.yr.no/verdata/free-weather-data/
    QString baseUrl("http://dev.p2p.or.kr:8080/open_vaccine_api/");

    // http://localhost:8080/open_vaccine_api/android/2.2.1/samsung/galuxy6/scan_data

    baseUrl.append(os);
    baseUrl.append("/");
    baseUrl.append(version);
    baseUrl.append("/");
    baseUrl.append(vendor);
    baseUrl.append("/");
    baseUrl.append(model);
    baseUrl.append("/scan_data");

    QUrl searchUrl(baseUrl);
    //manager->setConfiguration(QSslConfiguration::defaultConfiguration());
    manager->get(QNetworkRequest(searchUrl));
    waitForScanDataQueryReply(tr("Waiting for scan data, network may be slow..."));
}

int AppModel::currentScanPercent() const
{
    if(!m_scanCount)
        return 0;
    return (1.0 * m_scanCount  / m_appFiles.count()) * 100;
}

void AppModel::scanDefectFile()
{
    QStringList strDirs;
    QStringList strFilters;

    #if defined(Q_OS_ANDROID)
         strFilters += "*.apk";
         strDirs += "/";
         //strDirs += "/system/app";
    #elif defined(Q_OS_DARWIN64)
         strFilters += "*.apk";
         //strDirs += "/Applications";
         strDirs += "/Users/yezune/projects/OV/sigdb/apks";
         //strDirs += "/usr/local";
         //strDir = "/";
    #elif defined(Q_OS_LINUX)
         strFilters += "*";
         strDirs = "/";
    #elif defined(Q_OS_WIN32) | defined(Q_OS_WIN64)
         strFilters += "*.exe";
         strFilters += "*.dll";
         strDirs = "c:\\Program Files";
         //strDir = "c:\\Program Files(32)";
    #else
        qFatal("Not Support OS!!!");
        qApp->exit();
    #endif

    m_futureScan = QtConcurrent::run(AppModel::scanDefectFile2, this, strDirs, strFilters);
}

void AppModel::reset(){
    m_appFiles.clear();
    m_defectFiles.clear();
    m_defectFilesSha1.clear();
    setCurrentScanFile("");
    setScanCount(0);
}

void AppModel::cancelScan(){
    m_futureScan.cancel();
    reset();
}

void AppModel::setScanCount(int count)
{
    qDebug() << "Scan Count:" << count;
    Q_ASSERT(0 <= count);
    m_scanCount = count;
    emit(currentScanPercentChanged(currentScanPercent()));
}

void AppModel::scanDefectFile2(AppModel *self, QStringList strDirs, QStringList strFilters)
 {
     qDebug() << "scan Defect File" << strDirs << strFilters;

     self->reset();

     emit(self->fileCountStart());

     foreach(QString dir, strDirs){

         if( self->m_futureScan.isCanceled()) break;

         QDirIterator iterDir(dir, strFilters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

         while (iterDir.hasNext())
         {

            if( self->m_futureScan.isCanceled()) break;

            iterDir.next();
            self->m_appFiles += iterDir.filePath();
            qDebug() << "file:" << iterDir.filePath();
         }
     }

     emit(self->fileCountComplete());

     if( self->m_futureScan.isCanceled()) return;

     qDebug() << "app count:" << self->m_appFiles.length();

     foreach(QString filePath, self->m_appFiles){

        qDebug() << "Scanning file! ->" << filePath;

        if( self->m_futureScan.isCanceled()) break;

        self->setCurrentScanFile(filePath);
        std::string sha1 = SHA1::from_file(filePath.toUtf8().constData());

        // Open file as a .zip file
        #if defined(Q_OS_ANDROID)
              QuaZip zip(filePath.toUtf8().constData());
              zip.open(QuaZip::mdUnzip);
              QuaZipFile file(&zip);
              for(bool f=zip.goToFirstFile(); f; f=zip.goToNextFile()) {
                 QString zipFilePath = zip.getCurrentFileName();
                 if (zipFilePath.endsWith(".RSA")) {
                    qDebug() << "Sig file found! ->" << zipFilePath;
                    // Open the certificate file
                    file.open(QIODevice::ReadOnly);
                    QByteArray data = file.readAll();
                    file.close();

                    // Compute the SHA-1 digest of the cert file
                    const unsigned char *crtdata = (unsigned char*) data.constData();
                    PKCS7 *p7 = d2i_PKCS7(NULL, &crtdata, data.size());
                    PKCS7_SIGNER_INFO *si = sk_PKCS7_SIGNER_INFO_value(PKCS7_get_signer_info(p7),0);
                    X509 *crt = PKCS7_cert_from_signer_info(p7, si);

                    char shabuf[20];
                    const EVP_MD *digest = EVP_sha1();
                    unsigned int len;
                    int rc = X509_digest(crt, digest, (unsigned char*) shabuf, &len);
                    if (rc == 0 || len != 20) {
                        qDebug() << "Failed to calculate digest! ->" << filePath;
                    }

                    // Compare shabuf with sha1_old and sha1_new
                    // TODO: Make this more robust so we can add more signatures
                    bool eq1 = true;
                    bool eq2 = true;
                    int i;
                    for (i=0; i<20; i++){
                       if (shabuf[i] != sha1_new[i]) {
                          eq1 = false;
                       }
                       if (shabuf[i] != sha1_old[i]) {
                          eq2 = false;
                       }
                    }

                    if (eq1 || eq2) {
                       qDebug() << "digest match found!!";
                       self->m_defectFiles.append(filePath);
                    }

                    break;
                 }
            }
        #endif

        if(self->m_bloomFilter.contains(sha1))
        {
            qDebug() << "signature found! ->" << sha1.c_str();
            self->m_defectFiles.append(filePath + ',' + sha1.c_str());
            self->m_defectFilesSha1.append(sha1.c_str());
        }else{
            qDebug() << "signature not found! ->" << sha1.c_str();
        }

        self->setScanCount( self->scanCount() + 1);
     }
     qDebug() << "Scan Complete.";
     qDebug() << "defect files: " << self->m_defectFiles ;
     //emit((tr("Scan Complete: %1").arg(reply->errorString())));
}

void AppModel::replyFinished(QNetworkReply *reply)
{
    waitForScanDataQueryReply("");
    QString data;
    if (reply->error() != QNetworkReply::NoError) {
        emit(errorOnQueryScanData(tr("Network error: %1").arg(reply->errorString())));
        qDebug() << "Network error:" << reply->errorString();
        data = "{\
               \"ver\": \"0.0.1\", \
               \"title\": \"open vaccine signature database\"\
               \"signatures\": \
               {\
                 \"5f5fdcfc9de29788efa68dfc4a341fd7a17a2f30\": {\"emailId\": \"1134100\", \"fileName\": \"ServiceUpgrade.default.apk\", \"fileId\": \"546130\"}, \
                 \"f1ff967b50dad682f53289e369b4c5c23ba73da4\": {\"emailId\": \"1134172\", \"fileName\": \"ServiceUpdate.v2.apk\", \"fileId\": \"546147\"}, \
                 \"e0332b702fed2fdf08d439a00374d6f3e81aa506\": {\"emailId\": \"1133625\", \"fileName\": \"ServiceUpgrade.v2.apk\", \"fileId\": \"545986\"}, \
                 \"39755ed581e87d4adcabb8c09d50112fb8fe6a3f\": {\"emailId\": \"1149302\", \"fileName\": \"AndroidUpdate.default.apk\", \"fileId\": \"556521\"}\
               },\
              }";
    } else {
       data = reply->readAll();
    }

    qDebug() << "scanData:" << data;

    QJsonDocument jdoc( QJsonDocument::fromJson(data.toUtf8()) );

    m_scanData = jdoc.object();

    qDebug() << "scanData:" << m_scanData.value("signatrues").toString();

    // initialize bloom filter using sha1 signature

    if(m_scanData.contains("signatures")){
        if(m_scanData["signatures"].isObject()){

            QJsonValue sigs = m_scanData.value(QString("signatures"));
            QStringList keys = sigs.toObject().keys();

            foreach(QString sha1_key, keys){
                qDebug() << "sha1_key:" << sha1_key;
                m_bloomFilter.insert(std::string(sha1_key.toUtf8().constData()));
            }

        }else{
            Q_ASSERT(false);
        }
    }else{
        Q_ASSERT(false);
    }

    if (reply) {
        reply->deleteLater();
        reply = 0;
    }
}

void AppModel::updateEmail(){
#if defined(Q_OS_ANDROID)
    qDebug() << "updateMessage(android)" ;

//    QAndroidJniObject javaMessage = QAndroidJniObject::fromString(m_message);
//    QAndroidJniObject::callStaticMethod<void>("org/p2plab/openvaccine/SendMail",
//                                       "send",
//                                       "(Ljava/lang/String;)V",
//                                       javaMessage.object<jstring>());

//http://stackoverflow.com/questions/30128718/general-share-button-on-android-in-qt-quick

    QAndroidJniEnvironment _env;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");   //activity is valid
    if (_env->ExceptionCheck()) {
        _env->ExceptionClear();
        throw InterfaceConnFailedException();
    }
    if ( activity.isValid() )
    {
        QAndroidJniObject::callStaticMethod<void>("org/p2plab/openvaccine/SendMail",
                                                  "sendText","(Landroid/app/Activity;Ljava/lang/String;)V",
                                                  activity.object<jobject>(),
                                                  QAndroidJniObject::fromString(m_email).object<jstring>());
        if (_env->ExceptionCheck()) {
            _env->ExceptionClear();
            throw InterfaceConnFailedException();
        }
    }else
        throw InterfaceConnFailedException();
    qDebug() << "updateMessage(android)" << "SendMail.send() called!" ;

#elif defined(Q_OS_WINDOWS)
    qDebug() << "updateMessage(windows)" << m_email;
#elif defined(Q_OS_DARWIN)
    qDebug() << "updateMessage(osx)" << m_email;
#else
    Q_ASSERT(true);
#endif
}

void AppModel::setEmail(const QString &email){

    m_email = email;
    emit emailChanged();
}


