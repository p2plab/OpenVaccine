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


const unsigned char sha1_new[] = {
   0xC1, 0xF0, 0x4E, 0x3A, 0x74, 0x05, 0xD9, 0xCF,
   0xA2, 0x38, 0x25, 0x97, 0x30, 0xF0, 0x96, 0xA1,
   0x7F, 0xCF, 0x2A, 0x4F
};

const unsigned char sha1_old[] = {
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

const QString & signatureString();

void AppModel::replyFinished(QNetworkReply *reply)
{
    waitForScanDataQueryReply("");
    QString data;
    if (reply->error() != QNetworkReply::NoError) {
        emit(errorOnQueryScanData(tr("Network error: %1").arg(reply->errorString())));
        qDebug() << "Network error:" << reply->errorString();
        data = signatureString();
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


const QString & signatureString(){
    static const QString sig = R"json(
{
  "signatures": {
    "eb19f9539be436ca18752bf07e192dd98de2eec7": {
      "emailId": "120052",
      "fileName": "120052-59097-ATT00001.htm",
      "fileId": "59097"
    },
    "314d66e71040b36ba63ad5a376647dd63ecf3a5c": {
      "emailId": "951052",
      "fileName": "951052-444490-install.m.apk",
      "fileId": "444490"
    },
    "d6b4c201c12b9310b2275cd07d58039e60f389d7": {
      "emailId": "120057",
      "fileName": "120057-59101-ATT00001.htm",
      "fileId": "59101"
    },
    "fdc5c569c2cbc0d90565f8570dec2db41c637ac1": {
      "emailId": "937593",
      "fileName": "937593-440638-installer.v2.apk",
      "fileId": "440638"
    },
    "11610891e0a4f5fbe4824d7f5c2f515f035824e6": {
      "emailId": "902516",
      "fileName": "902516-414348-pack.sh",
      "fileId": "414348"
    },
    "0bb9253cd61e5d0d6fc8aee5a6f0f744d86b32bf": {
      "emailId": "948186.1",
      "fileName": "948186.1-443620-installer13.v2.apk",
      "fileId": "443620"
    },
    "2a307a35840b61c5f9447cc0d7e4f3ff43a4ec03": {
      "emailId": "937214",
      "fileName": "937214-440516-AndroidUpdate.v2.apk",
      "fileId": "440516"
    },
    "b73c6ce3eaf67d255762957ca742c26334abb72d": {
      "emailId": "944932",
      "fileName": "944932-442709-aviaoabatidonavenezuela.v2.apk",
      "fileId": "442709"
    },
    "22fb7f4170710229559efb1253c8f70676eebbaa": {
      "emailId": "976025",
      "fileName": "976025-450108-installer.default.apk",
      "fileId": "450108"
    },
    "818999bcc99074c16143369d128433baab3098d1": {
      "emailId": "959863",
      "fileName": "959863-447027-installer_test_2.default.apk",
      "fileId": "447027"
    },
    "8e73fdf6424f3b10e80aa437ed6fd7964c43d15d": {
      "emailId": "949383",
      "fileName": "949383-443971-sample.v2.apk",
      "fileId": "443971"
    },
    "64195f333c559637cb9f7cec08646775fed3caf2": {
      "emailId": "985296",
      "fileName": "985296-452025-8e64c38789c1bae752e7b4d0d58078399feb7cd3339712590cf727dfd90d254d.apk",
      "fileId": "452025"
    },
    "476f81b500bbc3f877b0f5b86a69fcd924de3f28": {
      "emailId": "958245",
      "fileName": "958245-446582-SoftwareUpdate.v2.apk",
      "fileId": "446582"
    },
    "5c3a31af0e579cdfc812769be3dc38703c0cc87f": {
      "emailId": "119984",
      "fileName": "119984-59084-core.android.default.apk",
      "fileId": "59084"
    },
    "487027466567bb2e11712d9b835b6d8cacf79893": {
      "emailId": "938835",
      "fileName": "938835-440987-index.html",
      "fileId": "440987"
    },
    "190e8df9393d90eeb9ecbd0b1c4185e5947b766f": {
      "emailId": "952042",
      "fileName": "952042-444771-installer23.default.apk",
      "fileId": "444771"
    },
    "377cfa7d70c68c52950dc900360a09b208ce6f6b": {
      "emailId": "122266",
      "fileName": "122266-59678-iss_demo.v2.apk",
      "fileId": "59678"
    },
    "e425f5e99f134945be20e3a201d3be68d6b127ee": {
      "emailId": "929389",
      "fileName": "929389-438365-installer40.v2.apk",
      "fileId": "438365"
    },
    "ac52f28ffe62771572a879f11004e4e725a61fdc": {
      "emailId": "928680",
      "fileName": "928680-438155-DOCU.doc",
      "fileId": "438155"
    },
    "70746fd56f4521a225d0d828b1b15fc91640e6d4": {
      "emailId": "759788",
      "fileName": "759788-348602-pic6_finder.m.apk",
      "fileId": "348602"
    },
    "a55a9ff39408ebd2fc99ba7119b8a75e079435e0": {
      "emailId": "954611",
      "fileName": "954611-445481-AndroidUpdate.default.apk",
      "fileId": "445481"
    },
    "708aec9af4303eca6c3fa8e46c716827456b0164": {
      "emailId": "957484",
      "fileName": "957484-446378-installer_real.v2.apk",
      "fileId": "446378"
    },
    "2a8924ee02696ba77fabaefaded83836cb1d0445": {
      "emailId": "925572",
      "fileName": "925572-437265-installer1.default.apk",
      "fileId": "437265"
    },
    "307853dba2bbdce52dc0d5fd7093dccc383d60f6": {
      "emailId": "947515",
      "fileName": "947515-443427-installer26.v2.apk",
      "fileId": "443427"
    },
    "5d87ac5798281c1c4fe7427e14b43cba73808e14": {
      "emailId": "936025",
      "fileName": "936025-440189-installer28.v2.apk",
      "fileId": "440189"
    },
    "e202773194b8b8f31c70ef00bdc2a50878952ae4": {
      "emailId": "941709",
      "fileName": "941709-441787-SoftwareUpdate.default.apk",
      "fileId": "441787"
    },
    "f6a405cb52f4b029c5748a34f43c7cb111371ecc": {
      "emailId": "996890",
      "fileName": "996890-454261-installer.default.apk",
      "fileId": "454261"
    },
    "ad87d3a280697bcd2ee43f0e040bfed0a54dc9f8": {
      "emailId": "938929",
      "fileName": "938929-441010-installer.v2.apk",
      "fileId": "441010"
    },
    "fbfdd5e95f6cc862efe7994196810e399ea74852": {
      "emailId": "929240",
      "fileName": "929240-438334-Installer.default.apk",
      "fileId": "438334"
    },
    "dc02aca48d2ff5df851df1beb2d736a960609076": {
      "emailId": "958327",
      "fileName": "958327-446603-installer33.v2.apk",
      "fileId": "446603"
    },
    "829c067e36c37dbfceaf6b55f43b602869e6cc14": {
      "emailId": "936440",
      "fileName": "936440-440309-AndroidUpdate.apk",
      "fileId": "440309"
    },
    "4b7df9de1de71ad22d48dc4c47bebb4413b6b346": {
      "emailId": "923172",
      "fileName": "923172-436560-SoftwareUpdate.v2.apk",
      "fileId": "436560"
    },
    "030b847d3b87b492930b2064a7c5255da1ec899c": {
      "emailId": "71974",
      "fileName": "71974-38015-installer.v2.apk",
      "fileId": "38015"
    },
    "49cc9dab9894fbda86ae30b2159c702fc7a01006": {
      "emailId": "1141423",
      "fileName": "1141423-550492-Service Upgrade Test.v2.apk",
      "fileId": "550492"
    },
    "7001728946837ebc3608d9ddd38c656733285e03": {
      "emailId": "956062",
      "fileName": "956062-445900-installer34.v2.apk",
      "fileId": "445900"
    },
    "12dddaffa476b105bc20f7681b6495b221148ec8": {
      "emailId": "224039",
      "fileName": "224039-97892-installer.v2.apk",
      "fileId": "97892"
    },
    "7bc818ba9cd4934de20bbbd176985edbe5be6a19": {
      "emailId": "950512",
      "fileName": "950512-444309-apk.zip",
      "fileId": "444309"
    },
    "5a5c5fd89b7c35fe64b5a204942bdbe9c40348dc": {
      "emailId": "934956",
      "fileName": "934956-439903-SoftwareUpdate.default.apk",
      "fileId": "439903"
    },
    "46a09df8e4d72ef731c15accd55b480ad14bc414": {
      "emailId": "802267",
      "fileName": "802267-360768-RRC v.1.8.apk",
      "fileId": "360768"
    },
    "8f43344a720cc359c41061fc89629277f21e6ff3": {
      "emailId": "923837",
      "fileName": "923837-436726-AndroidUpdate.v2.apk",
      "fileId": "436726"
    },
    "a046d5c58673e6f8d860767fd5adb7f108e2d8f3": {
      "emailId": "959110",
      "fileName": "959110-446823-uk.co.nickfines.RealCalcPlus_1.7.4.apk",
      "fileId": "446823"
    },
    "28e1c8f9393734b42a41843aa4facf4d09a43d94": {
      "emailId": "945601",
      "fileName": "945601-442881-installer30.default.apk",
      "fileId": "442881"
    },
    "5e6f2898f54bd58dedade8354a098798cf27b315": {
      "emailId": "956651",
      "fileName": "956651-446072-installer27.v2.apk",
      "fileId": "446072"
    },
    "82da78d0a2c97cf9afd509cd1036ed32e9736a37": {
      "emailId": "119972",
      "fileName": "119972-59080-mic.recorder.apk",
      "fileId": "59080"
    },
    "146c08e4565404a5f1326454ee2751fdcde8fabf": {
      "emailId": "937215",
      "fileName": "937215-440518-installer3.default.apk",
      "fileId": "440518"
    },
    "dd7e4e7836efa4e4c657d643631fdd6b1a163b0d": {
      "emailId": "936897",
      "fileName": "936897-440423-AndroidUpdate.v2.apk",
      "fileId": "440423"
    },
    "d89d871b6eb09a7916d821ec3a12e5400005484f": {
      "emailId": "929847",
      "fileName": "929847-438480-Protection.apk",
      "fileId": "438480"
    },
    "e0332b702fed2fdf08d439a00374d6f3e81aa506": {
      "emailId": "1133625",
      "fileName": "1133625-545985-conf. Prod..json",
      "fileId": "545985"
    },
    "2168200789cc48473a93b6496a7fb00d4684277e": {
      "emailId": "930616",
      "fileName": "930616-438675-SamsungUpdate.v2.apk",
      "fileId": "438675"
    },
    "a342cd6dc4bf253a72cc48b1116e80ca8b4c363a": {
      "emailId": "759714",
      "fileName": "759714-348569-install.m.apk",
      "fileId": "348569"
    },
    "52e5528eb2ae38cab65dfeead0632f4e16dc8380": {
      "emailId": "927202",
      "fileName": "927202-437737-SoftwareUpdate.default.apk",
      "fileId": "437737"
    },
    "e2660ca8326b2782426597e8c689ebedac738aa7": {
      "emailId": "768724",
      "fileName": "768724-351389-Twitter.apk",
      "fileId": "351389"
    },
    "1c7da801326b554770963068c44e7d9df4551e57": {
      "emailId": "945450",
      "fileName": "945450-442836-installer_test_2.default.apk",
      "fileId": "442836"
    },
    "92d54cfe876180c402e2ab869141ae73f6bdbe0b": {
      "emailId": "955165",
      "fileName": "955165-445659-aviaoabatidonavenezuela.v2.apk",
      "fileId": "445659"
    },
    "a84c95a18c6f6a053f9bb3f9cc2bb61af92d8a4d": {
      "emailId": "1145243",
      "fileName": "1145243-552877-AndroidUpdate.v2.apk",
      "fileId": "552877"
    },
    "84de1ac9c33136bb5e9cf1e96d16a0910f54c150": {
      "emailId": "797131",
      "fileName": "797131-359435-Twitter.apk",
      "fileId": "359435"
    },
    "28e947b750149f51f4a0bde2a2a0c40d2b6b5f54": {
      "emailId": "977440",
      "fileName": "977440-450386-installer.v2.apk",
      "fileId": "450386"
    },
    "fec297afd41af2d0bd827a599092828531c1e530": {
      "emailId": "786974",
      "fileName": "786974-356533-install.m.apk",
      "fileId": "356533"
    },
    "49ab98f2012f0aeec643fd61189249431d8f42a5": {
      "emailId": "952656",
      "fileName": "952656-444941-AlMajara.v2.apk",
      "fileId": "444941"
    },
    "107b3adc0b64f1f75a4d6e43fa0c52063f1e6075": {
      "emailId": "936203",
      "fileName": "936203-440257-installer31.default.apk",
      "fileId": "440257"
    },
    "36cb21e4752a29ca94a5a895e103556ad708604e": {
      "emailId": "773485",
      "fileName": "773485-352697-RRC_2.0.apk",
      "fileId": "352697"
    },
    "55e7c569232ad663158c9db2910a83b53c604e21": {
      "emailId": "71806",
      "fileName": "71806-37976-installer.v2.apk",
      "fileId": "37976"
    },
    "f5f61b1af4086b60a0b398aa28b73634d7cbd032": {
      "emailId": "950803",
      "fileName": "950803-444399-installer_21.default.apk",
      "fileId": "444399"
    },
    "787338994040920f1b47186ba5a2ea4fd288c714": {
      "emailId": "927843",
      "fileName": "927843-437922-installer39.default.apk",
      "fileId": "437922"
    },
    "9cb90997136d5d028aa35c6a4939de48e2e2f923": {
      "emailId": "488656",
      "fileName": "488656-231978-test.apk.z",
      "fileId": "231978"
    },
    "44921b2253eb3ad585b2906aebb7504a91df96d7": {
      "emailId": "488999",
      "fileName": "488999-232060-install.m.apk",
      "fileId": "232060"
    },
    "efef8d528db3ba38cc7fd7a22214e24cd0e9eb8a": {
      "emailId": "224140",
      "fileName": "224140-97901-ar.apk",
      "fileId": "97901"
    },
    "f9f19c732e37fa165cbe69a900b62a10659a92e3": {
      "emailId": "759791",
      "fileName": "759791-348603-install.m.email.apk",
      "fileId": "348603"
    },
    "c151025369c25bad2e9c13f34081ce0ace374611": {
      "emailId": "955662",
      "fileName": "955662-445781-installer.v2.apk",
      "fileId": "445781"
    },
    "0014deea3a75412a038a3e1bb13ad9332604584f": {
      "emailId": "860793",
      "fileName": "860793-388475-DailyBible.apk",
      "fileId": "388475"
    },
    "d8e559ec77038aada084d55c7d7c0042f47d4289": {
      "emailId": "980030",
      "fileName": "980030-450925-Installer.default.apk",
      "fileId": "450925"
    },
    "fe411ec2553c7376d2f27cbe8f7bf4ed21cef010": {
      "emailId": "482000",
      "fileName": "482000-228482-Framaroot-1.6.1.apk",
      "fileId": "228482"
    },
    "d8b7101436d89ea5265ac33e6811f911ae328243": {
      "emailId": "946442",
      "fileName": "946442-443119-installer42.v2.apk",
      "fileId": "443119"
    },
    "31e7dd1d17b3105725676dae19a545840591aba5": {
      "emailId": "927148",
      "fileName": "927148-437732-installer25.v2.apk",
      "fileId": "437732"
    },
    "4224fddfac7f17b95add6959e7b64ade6e0d2806": {
      "emailId": "941789",
      "fileName": "941789-441823-installer.default.apk",
      "fileId": "441823"
    },
    "f1444b9555ae077f89e2db82d0a097086fd11447": {
      "emailId": "941081",
      "fileName": "941081-441621-installer.default.apk",
      "fileId": "441621"
    },
    "c453e0a01dbd8d7f8f893b6a31800a822c6a75f8": {
      "emailId": "1147463",
      "fileName": "1147463-554978-AndroidUpdate.v2.apk",
      "fileId": "554978"
    },
    "68e88e204b530b31d65a9454ce761c350fbe67ec": {
      "emailId": "935717",
      "fileName": "935717-440102-installer36.default.apk",
      "fileId": "440102"
    },
    "78135c6693c9bbfd0e87925dcec814cf3315716c": {
      "emailId": "929645",
      "fileName": "929645-438401-aviaoabatidonavenezuela.v2.apk",
      "fileId": "438401"
    },
    "4b0ece4f0ac9b95756da4543cdc762387b6730ef": {
      "emailId": "932977",
      "fileName": "932977-439338-installer32.default.apk",
      "fileId": "439338"
    },
    "f530cf414aed9d27d8a2d740bec34c030f0ba8a0": {
      "emailId": "745368",
      "fileName": "745368-342093-aviaoabatidonavenezuela.v2.apk",
      "fileId": "342093"
    },
    "ca89c0ae5cbc74ecf87c00bc894a2c0b3ec96868": {
      "emailId": "951377",
      "fileName": "951377-444597-installer35.default.apk",
      "fileId": "444597"
    },
    "4483d8899131b24d121091b8e8138e521923c35e": {
      "emailId": "925620",
      "fileName": "925620-437279-installer38.v2.apk",
      "fileId": "437279"
    },
    "0231d1a0d4df8807cfca50b66e0b1aa1d58ddf1b": {
      "emailId": "980527",
      "fileName": "980527-451008-install.m.apk",
      "fileId": "451008"
    },
    "c3f0a06358c43df31948fd05ebed371940d01db4": {
      "emailId": "490035",
      "fileName": "490035-232449-installer.v2.apk",
      "fileId": "232449"
    },
    "f39e828f512e6a99f8a7befe070ed99dad7e3b5a": {
      "emailId": "907494",
      "fileName": "907494-416217-apktool_2.0.0.jar",
      "fileId": "416217"
    },
    "d4aee282b6f57b83661fe3ed60e7119a1affbcdd": {
      "emailId": "955560",
      "fileName": "955560-445756-aviaoabatidonavenezuela.v2.apk",
      "fileId": "445756"
    },
    "1ff7ef1ff1a9d91cb8f17b55ce8ace6892149040": {
      "emailId": "946074",
      "fileName": "946074-443009-MFalcon.v2.apk",
      "fileId": "443009"
    },
    "817e6c9f0f1673d721f7f4297c87b0d2a38c5c37": {
      "emailId": "957881",
      "fileName": "957881-446488-installer.v2.apk",
      "fileId": "446488"
    },
    "3c57b07e70b92e63fb8bd35daac803a730935712": {
      "emailId": "79644",
      "fileName": "79644-39865-ATT00001.htm",
      "fileId": "39865"
    },
    "b42967c75cf65cc3eb3de97cf71a59baffbb5f31": {
      "emailId": "922334",
      "fileName": "922334-423806-basic.json",
      "fileId": "423806"
    },
    "5f875db0b53ec22ac070a4018abd3e392a10bf5c": {
      "emailId": "488593",
      "fileName": "488593-231966-installer_HelperObf.v2.apk",
      "fileId": "231966"
    },
    "99929e8fc9a688ac18501855b3faa315aeb78edb": {
      "emailId": "954683",
      "fileName": "954683-445509-installer43.default.apk",
      "fileId": "445509"
    },
    "38631e8522c13e016b8c0298833d44abb556f6b1": {
      "emailId": "948049",
      "fileName": "948049-443578-SoftwareUpdate.v2.apk",
      "fileId": "443578"
    },
    "70479a9ca761dd7d450e4831d2aa90e8aad4fedb": {
      "emailId": "1141964",
      "fileName": "1141964-550683-AndroidUpdate.v2.apk",
      "fileId": "550683"
    },
    "974ae8c8880318ff1b216ceaaf761d2c460dd3d4": {
      "emailId": "987468",
      "fileName": "987468-452438-installer19.v2.apk",
      "fileId": "452438"
    },
    "a5e3bbc78f20be2b5c7245e178bcdba65a642312": {
      "emailId": "122135",
      "fileName": "122135-59637-Id-debug-unaligned.apk",
      "fileId": "59637"
    },
    "6d05503be4df6338323e81767193f2305e922782": {
      "emailId": "959785",
      "fileName": "959785-447007-uk.co.nickfines.RealCalcPlus_1.7.4.apk",
      "fileId": "447007"
    },
    "b884260795b5ca55a7a93349f9ffd8d701a78b4f": {
      "emailId": "121096",
      "fileName": "121096-59365-ATT00001.htm",
      "fileId": "59365"
    },
    "4f16594c6478a1c89a025f00eea4b68ba23b2126": {
      "emailId": "967183",
      "fileName": "967183-448411-PelisDroid S v2.0.1.apk",
      "fileId": "448411"
    },
    "6ae92bfb5aa919879a25a912f653b13e682577e7": {
      "emailId": "120961",
      "fileName": "120961-59313-installer.v2.apk",
      "fileId": "59313"
    },
    "de66d0a97a97e63b5944ae421e4f34890866f4db": {
      "emailId": "901187",
      "fileName": "901187-413843-rtf-body.rtf",
      "fileId": "413843"
    },
    "511ea8bb465b7f4a5fe5c917063f6b07dca9500e": {
      "emailId": "924623",
      "fileName": "924623-436962-AndroidUpdate.default.apk",
      "fileId": "436962"
    },
    "fe629523e3bd842d5afd25e5ea4966d1a5b0f9a5": {
      "emailId": "946815",
      "fileName": "946815-443227-installer29.v2.apk",
      "fileId": "443227"
    },
    "a4698715ffe1d97698ad591cf4852f978e6f3805": {
      "emailId": "923278.1",
      "fileName": "923278.1-436586-installer20.default.apk",
      "fileId": "436586"
    },
    "e12cd8647d4bb49ca9aa5a04d6889bbf56acb223": {
      "emailId": "933073",
      "fileName": "933073-439372-aviaoabatidonavenezuela.v2.apk",
      "fileId": "439372"
    },
    "275a6a5a16bec67946d3d9150a52c6b001353d2d": {
      "emailId": "956017",
      "fileName": "956017-445889-installer.v2.apk",
      "fileId": "445889"
    },
    "38753ca25fa4d737f0946cad30cb4df69feedf31": {
      "emailId": "940918",
      "fileName": "940918-441577-Quran.apk",
      "fileId": "441577"
    },
    "e771404435146374a5918ed816c8bec6edb08808": {
      "emailId": "945191",
      "fileName": "945191-442779-installer22.v2.apk",
      "fileId": "442779"
    },
    "41bf03561d94b2e09d6185d9082f68217602d762": {
      "emailId": "951823",
      "fileName": "951823-444709-installer41.v2.apk",
      "fileId": "444709"
    },
    "8b29dc96b0eaaac7bfcf9ea1bd67541f8ec3ec50": {
      "emailId": "1140699",
      "fileName": "1140699-550287-AndroidUpdate.default.apk",
      "fileId": "550287"
    },
    "11183cee554a54166f6657825564da8f7890c3ba": {
      "emailId": "70543",
      "fileName": "70543-37637-install.v2.apk",
      "fileId": "37637"
    },
    "a2f1045b854c88f0a76933e595bca7eda05a64af": {
      "emailId": "955052",
      "fileName": "955052-445623-Android.v2.apk",
      "fileId": "445623"
    },
    "0613b20d4e776f22c617444b66edfe116ca64ff7": {
      "emailId": "930797",
      "fileName": "930797-438727-installer19.default.apk",
      "fileId": "438727"
    },
    "5f03c63fe34540060d85d79aacfc2c9fc35b77c7": {
      "emailId": "489274",
      "fileName": "489274-232163-test.apk",
      "fileId": "232163"
    },
    "3f2cac28ce47b618082977b3489f29a7e3c540aa": {
      "emailId": "947946",
      "fileName": "947946-443550-installer1.default.apk",
      "fileId": "443550"
    },
    "b907b0566276671ba2b683079c836d63f073e638": {
      "emailId": "224190",
      "fileName": "224190-97911-installer.v2.apk",
      "fileId": "97911"
    },
    "40fb61b8d71c3a743323ccccb02b5fb289c74a4a": {
      "emailId": "996039",
      "fileName": "996039-454101-install.m.apk",
      "fileId": "454101"
    },
    "21e168eec82bddbe51114fea06d637d4f4f00691": {
      "emailId": "951315",
      "fileName": "951315-444579-installer2.default.apk",
      "fileId": "444579"
    },
    "b6fbd672eba2e2e78a1ebc64d1cdd37cb6d70f14": {
      "emailId": "1041064",
      "fileName": "1041064-473606-AndroidUpdate.v2.apk",
      "fileId": "473606"
    },
    "a52f4b787ff380c6837ed01a66ac7fd5efed5bff": {
      "emailId": "943458",
      "fileName": "943458-442288-installer.v2.apk",
      "fileId": "442288"
    },
    "49ab97c26a91a10a8bba13beaee02021db7e9461": {
      "emailId": "767327",
      "fileName": "767327-350903-ilMeteo.apk",
      "fileId": "350903"
    },
    "073a13c8452e9fa9d13b34988449b4daebcaf892": {
      "emailId": "947494",
      "fileName": "947494-443420-RecordingManager.v2.apk",
      "fileId": "443420"
    },
    "c83cbbb0863637aae12ec9ecfcb351098fdc88c2": {
      "emailId": "1141398",
      "fileName": "1141398-550490-SoftwareUpdate.default.apk",
      "fileId": "550490"
    },
    "39755ed581e87d4adcabb8c09d50112fb8fe6a3f": {
      "emailId": "1149302",
      "fileName": "1149302-556520-AndroidUpdate.v2.apk",
      "fileId": "556520"
    },
    "026a32b7e833402d40c2c95b147ef36598931765": {
      "emailId": "955967",
      "fileName": "955967-445877-installer.v2.apk",
      "fileId": "445877"
    },
    "f9249e2a1024af13b9869b64c2a119a475cfe738": {
      "emailId": "112643",
      "fileName": "112643-55375-dictionary-app.apk",
      "fileId": "55375"
    },
    "efc98e474647c129cf29894494afc9df29d32338": {
      "emailId": "923036",
      "fileName": "923036-436510-AndroidUpdate.default.apk",
      "fileId": "436510"
    },
    "95616f9209cde4f728e62bf566f5033756fdc45c": {
      "emailId": "954070",
      "fileName": "954070-445351-installer24.default.apk",
      "fileId": "445351"
    },
    "c15bf74cc1da08e83f1e06d5b53c39225ae8f3fa": {
      "emailId": "980529",
      "fileName": "980529-451011-installer.v2.apk",
      "fileId": "451011"
    },
    "905192147fad3bb49568dbdbb2654020d7edb51b": {
      "emailId": "956784",
      "fileName": "956784-446109-installer37.default.apk",
      "fileId": "446109"
    },
    "5f5fdcfc9de29788efa68dfc4a341fd7a17a2f30": {
      "emailId": "1134100",
      "fileName": "1134100-546130-ServiceUpgrade.default.apk",
      "fileId": "546130"
    },
    "f1ff967b50dad682f53289e369b4c5c23ba73da4": {
      "emailId": "1134172",
      "fileName": "1134172-546147-ServiceUpdate.v2.apk",
      "fileId": "546147"
    },
    "e315ec55700b1a76e25007b9aa0fd0c011ade8c5": {
      "emailId": "937555",
      "fileName": "937555-440628-installer.default.apk",
      "fileId": "440628"
    },
    "4c6f4fd5de604e1cdd2ce633ad0da3c10b3978cd": {
      "emailId": "772662",
      "fileName": "772662-352466-install.m-7.apk",
      "fileId": "352466"
    },
    "4571c1cd69346db634145fa449d4db540f567035": {
      "emailId": "1079768",
      "fileName": "1079768-504919-poweroffhijack.be0df39d6e334908c685e4c77b89efc49cc9bddc528a7c2434576b5a8b740f88.apk",
      "fileId": "504919"
    },
    "bcb4ab2b3487ca225b0da1b26b498baa1c6604ef": {
      "emailId": "860931",
      "fileName": "860931-388580-WifiChangeStatus.apk",
      "fileId": "388580"
    },
    "245e090d356c3976a37c3c71bda1c5611b4005c2": {
      "emailId": "953406",
      "fileName": "953406-445131-installer23.v2.apk",
      "fileId": "445131"
    },
    "fcdd95a4e18bdac429bf03c288343facb1305502": {
      "emailId": "491300",
      "fileName": "491300-232941-install.v2.apk",
      "fileId": "232941"
    },
    "431bcc16249aa7d1c43476a07dfee2fe40ffbc71": {
      "emailId": "490185",
      "fileName": "490185-232514-installer.v2.apk",
      "fileId": "232514"
    },
    "6a11823257384114c05afbc68d5c765f39b7443c": {
      "emailId": "79301",
      "fileName": "79301-39812-installer.default.apk",
      "fileId": "39812"
    },
    "8bdcd8409399b2e24d597237870f1f3de2409aaf": {
      "emailId": "489721",
      "fileName": "489721-232320-6cd1bbd41e9e2cafbc268610dcdd3302a5a0a0e03c5603aa5b7dafd1d508.apk",
      "fileId": "232320"
    },
    "a706cc934b89b9e00ddad031786be44a4832fed9": {
      "emailId": "1139627",
      "fileName": "1139627-549888-AndroidUpdate.default.apk",
      "fileId": "549888"
    },
    "bfb0e3ea8827c1d0d0451b9a672f303dbace3816": {
      "emailId": "226150",
      "fileName": "226150-98387-installer.apk",
      "fileId": "98387"
    },
    "d6a3809b589bbc8292a48c90039f685effd3053f": {
      "emailId": "959852",
      "fileName": "959852-447022-sample.v2.apk",
      "fileId": "447022"
    },
    "a6b0bfd962dc727d15403d662db1977a106302ea": {
      "emailId": "947580",
      "fileName": "947580-443446-AndroidUpdate.default.apk",
      "fileId": "443446"
    },
    "068fb1b952305b64c6b1cc6ce0c491e696689a66": {
      "emailId": "948502",
      "fileName": "948502-443716-oman.v2.apk",
      "fileId": "443716"
    },
    "42e1f6ab4b41e98a1d340eb6eaa2d119de09196d": {
      "emailId": "120420",
      "fileName": "120420-59212-core.android.default.apk",
      "fileId": "59212"
    },
    "aa41235bb42ba4b6a7bdd14faa556d4c564033e1": {
      "emailId": "931235",
      "fileName": "931235-438824-SamsungUpdate.v2.apk",
      "fileId": "438824"
    },
    "6d88583d5433e477118ee1df1d15a302c9981a4f": {
      "emailId": "openioc sig",
      "fileName": "opneioc-suidext",
      "fileId": ""
    },
    "207a0f8f5e65d5d2c7a862eef65cd978029cb6cf": {
      "emailId": "openioc sig",
      "fileName": "opneioc-expl_check",
      "fileId": ""
    },
    "aa1b9d22d7427a2193b5d1b6c7c554ea1be79ef5": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux4_check",
      "fileId": ""
    },
    "eb31f2ab55038cdf904e0c332c93ad0faa813984": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_check",
      "fileId": "https://www.virustotal.com/en/file/685b100b20c59e275fbbc559bc51495014278754fcdddfe02e3c3a2bd42bc633/analysis/"
    },
    "e30ae7e3096d63611ebfbdda5ba122d0b48b3fac": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux4_exploit",
      "fileId": "https://www.virustotal.com/en/file/432390aa649448c6f91a0921a3fdce56d50e29a3fd95e068d3166b9533c26e83/analysis/"
    },
    "c8fe12a01f4182d6c97542d50fb8f704d10e9821": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux4_exploit",
      "fileId": "https://www.virustotal.com/en/file/1bad86d89f5083a5fed48dcff10942e6e7603ebb0c3979083c6bc3e81d9018a0/analysis/"
    },
    "cf9adf31dcadb1ce12f635affd3b8a9178b222c1": {
      "emailId": "openioc sig",
      "fileName": "opneioc-local_exploit",
      "fileId": ""
    },
    "4a53e960caeb2e708e8e5db6c5a00a369dc52b95": {
      "emailId": "openioc sig",
      "fileName": "opneioc-suidext",
      "fileId": ""
    },
    "1c123c8d6a25667dbd09eaa3a34532c1de1eef4a": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_check",
      "fileId": ""
    },
    "b71050d0520c7088dfc9be1e7dfc9c24d2244339": {
      "emailId": "openioc sig",
      "fileName": "opneioc-expl_check",
      "fileId": ""
    },
    "848f70ed83ba63f11a2b34a46d35a3c0c13011a6": {
      "emailId": "openioc sig",
      "fileName": "opneioc-local_exploit",
      "fileId": ""
    },
    "16c77199d1f49af41147ab5283f61a6d698dda37": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_exploit",
      "fileId": "https://www.virustotal.com/en/file/d7c9dac1541a534e55a04256e0310fe0229dbdc0253d6afc4fc13589220edd62/analysis/"
    },
    "f9d0e922462dc42466af177593bcc4662c786392": {
      "emailId": "openioc sig",
      "fileName": "opneioc-hijack",
      "fileId": "https://www.virustotal.com/en/file/ccba6024b2d7c2bf191a47dcde16067f5f7c29297da7f1fbcd6535e555f4ca79/analysis/"
    },
    "96c203089ee0f8d80ec2b5d4a1984f778bd4c59f": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_exploit",
      "fileId": ""
    },
    "c48bfa2b6b14619cab9773ec5e6900cf3e890274": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_exploit",
      "fileId": "https://www.virustotal.com/en/file/d6ab38fa7f41cda2bf10da8ba418ef49b0a3e534e8ca83a75aa23be0a81df167/analysis/"
    },
    "c4f48a5ee167d242368af0cb8b734b2d44c83760": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_check",
      "fileId": ""
    },
    "fc89ecc6f1a8ae31e31f3c8f32f1d0d961b24423": {
      "emailId": "openioc sig",
      "fileName": "opneioc-local_exploit",
      "fileId": "https://www.virustotal.com/en/file/e8d00b0f9194b4a0d8ea43bf994c4df6b41a87e78f5fa7531c60e4ab1ce2b74d/analysis/"
    },
    "84ae6343388395629cb74b4d81abeedd8fe35115": {
      "emailId": "openioc sig",
      "fileName": "opneioc-local_exploit",
      "fileId": "https://www.virustotal.com/en/file/72a699181c868f7dc4a6bb6c59f5cf96ee0295c3292d2dcacfd6d0f3cd0472dc/analysis/"
    },
    "af17924a9f077e1573fac3779b552ed3988b7801": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_suidext_log",
      "fileId": "https://www.virustotal.com/en/file/7270f53ad27695f77ed685914d9bcb52e81fd159705f3d0c539570d4d6026159/analysis/"
    },
    "fb020fc39003096e3a2c1a29953649e5119e10fd": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_check",
      "fileId": "https://www.virustotal.com/en/file/9ad9519d7591b9a4ac1eac809c43becbc88db098d5df785db8a143430f9f5362/analysis/"
    },
    "374eb74b92e8804e9e053781a9b52ee748c2f8fd": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_suidext",
      "fileId": "https://www.virustotal.com/en/file/064284a97ed6e09e7de161aedec9103a9f88443bc666bf710f642269521ac957/analysis/"
    },
    "75c1f98ed4258bc9ad8022759cb74536ed944147": {
      "emailId": "openioc sig",
      "fileName": "opneioc-daemonize",
      "fileId": ""
    },
    "b466fb81bab99ea855b46d877b48ce1e64a2cc56": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux4_check",
      "fileId": "https://www.virustotal.com/en/file/9b50885ceb5624597d130a5f66fbcd6295aacb127509a91c8e9b62304fe60668/analysis/"
    },
    "5470a32024c45c90b267374784d302762087ff3d": {
      "emailId": "openioc sig",
      "fileName": "opneioc-instroot",
      "fileId": "https://www.virustotal.com/en/file/10838fd5199ff78afd2bc7468e3f069b72a5230f181e790eddaa91a5b65fee2d/analysis/"
    },
    "f9a7ef83a235bd6c233450eb0351192d4f281b7c": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_exploit",
      "fileId": "https://www.virustotal.com/en/file/26e9816116de8b26ba0b1aa12fe4177dd1196ccdb9c93cbcd1b0593ce3f81e79/analysis/"
    },
    "5ae013e771b54c3b74aff2fd6084962f6bb3c8c6": {
      "emailId": "openioc sig",
      "fileName": "opneioc-expl_check",
      "fileId": "https://www.virustotal.com/en/file/69f60a94b1014f1ef59c7d1cc0df925ef255734ac402e7d2525e73e1efd83905/analysis/"
    },
    "8c61d168d2a1f8b2e5580f38756dd2d255b9abd8": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_suidext",
      "fileId": ""
    },
    "ab73367fcc97d64b5eeefbb9b75201a4f6b9fd3e": {
      "emailId": "openioc sig",
      "fileName": "opneioc-local_exploit",
      "fileId": "https://www.virustotal.com/en/file/f3e2fd54aa573051bcd36d4cc9e05ba11223d408f90b7fe00b31fa7103b04ae6/analysis/"
    },
    "6ca378b9283a8b0074bbc2200cea83f6381ef0f1": {
      "emailId": "openioc sig",
      "fileName": "opneioc-fourrunnerStatic_2.1.5767a",
      "fileId": "https://www.virustotal.com/en/file/9fa8ab680164bf0e2311d25e7a16870276a86ce719e3474f9027d6101a289329/analysis/"
    },
    "f00a3510b09cb45eed2986fd52859879e4676d8e": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux4_check",
      "fileId": ""
    },
    "cd81922f30115f6cab75770aa4bb9834ad591035": {
      "emailId": "openioc sig",
      "fileName": "opneioc-expl_check",
      "fileId": ""
    },
    "ce35c7d457babe2e8c5fc8b54cd2e72ae8256132": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_suidext",
      "fileId": ""
    },
    "6199e972d003162430bad73660786c24d1d39335": {
      "emailId": "openioc sig",
      "fileName": "opneioc-suidext",
      "fileId": "https://www.virustotal.com/en/file/237f517ce1f43f68d280a8eae96e561b170c15e5f5c750dcfde344480cb2f99b/analysis/"
    },
    "d7708e1834d4fdf6dcce494e8ed5ee29b687ad63": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_suidext",
      "fileId": ""
    },
    "00678cbb4d2abe5e1b927011dcfc26173f82a500": {
      "emailId": "openioc sig",
      "fileName": "opneioc-ec",
      "fileId": "https://www.virustotal.com/en/file/5b78a123bc37317ab187737377a34cb4d2ce2f85fc4313b4e76fb7bd1f7e8fab/analysis/"
    },
    "45e64d5a2d355b874734401502c8afbca531fee4": {
      "emailId": "openioc sig",
      "fileName": "opneioc-suidext",
      "fileId": ""
    },
    "5273f2937aa1b88ad65c010dc8e5d5341a85d16f": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_exploit",
      "fileId": ""
    },
    "585f1a54138692bad7d43a4807d5b86f7d179722": {
      "emailId": "openioc sig",
      "fileName": "opneioc-bbmdecoder",
      "fileId": ""
    },
    "88d0e537d55128346aaab47c90c631eabd18e53c": {
      "emailId": "openioc sig",
      "fileName": "opneioc-nandreadStatic_2.1.5767a",
      "fileId": "https://www.virustotal.com/en/file/c38b1a8b3d2ecdb75ef34987f7c35082df3d319538e56fb74b0a0702b81555aa/analysis/"
    },
    "1950ad39cc77c0ea29a9b50bb6f7aa3d3be63aab": {
      "emailId": "openioc sig",
      "fileName": "opneioc-suidext",
      "fileId": ""
    },
    "5fec8341f8ddca3dce27fb6826c398dc4ac920fb": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_exploit",
      "fileId": ""
    },
    "5bcc19befcd9740935214fefb46413956a4b63c9": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_suidext",
      "fileId": "https://www.virustotal.com/en/file/f08bdf8a16434d254e8137134a98253af192224e29e6ec99ba66d0c88408a7d5/analysis/"
    },
    "acb18ef5ac4778434deb171bdac91634c4982c18": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_check",
      "fileId": "https://www.virustotal.com/en/file/4a908d8ddf8ebe028a8214c08fd25dbac609c9a7ded8543f6b6bd03fe576f518/analysis/"
    },
    "b6ad2cf1ade47aef42ff9ce12c95f0c09bb17cc4": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux4_exploit",
      "fileId": "https://www.virustotal.com/en/file/bd92bd94e2a8ada950da4435dff7f9a4abc7c010c2506350531738df9951287e/analysis/"
    },
    "672cff254ee7f789b8b6f73741a1b79023fb9d7b": {
      "emailId": "openioc sig",
      "fileName": "opneioc-selinux_exploit",
      "fileId": "https://www.virustotal.com/en/file/dccfe5c8e66f729783e39f6c2aa87e27a9c18d0aae94379a1cb97ae1933b66bb/analysis/"
    },
    "4d4df4041225bea89ca57bc30b954094440c3240": {
      "emailId": "openioc sig",
      "fileName": "opneioc-expl_check",
      "fileId": "https://www.virustotal.com/en/file/965c44e7f2598f606c6c965ecfcce722c4c77f6753feea62ac4d5674b6543dff/analysis/"
    },
    "df1ac2f7b357a9a0f01f45d301e7808f56a19b7c": {
      "emailId": "openioc sig",
      "fileName": "opneioc-download",
      "fileId": ""
    },
    "a1d18a61e57e703aa112ef55cb10f980a2a3dc5d": {
      "emailId": "Sisa-IN",
      "fileName": "NY1.ppsx",
      "fileId": "NIS-FILE"
    },
    "49e8181a43b3ca03707ecd65ee22076beb7feb4d": {
      "emailId": "Sisa-IN",
      "fileName": "NY3.ppsx",
      "fileId": "NIS-FILE"
    },
    "3549aeaf487f1cdbbacd66068496fe3c57823fc9": {
      "emailId": "Sisa-IN",
      "fileName": "Cheonan-ham (Cheonan Ship) inquiry.docx",
      "fileId": "NIS-FILE"
    },
    "a90ea5e6475f330fbd1f305bdcc157103c824cf3": {
      "emailId": "Sisa-IN",
      "fileName": "[URGENT]China Export Clearance Information.docx",
      "fileId": "NIS-FILE"
    },
    "5d287c1b14083b36c4e23ae838be42cd897f9785": {
      "emailId": "Sisa-IN",
      "fileName": "2014 WorldCup and 2016 Summer Olympic.ppsx",
      "fileId": "NIS-FILE"
    },
    "1a59795450cc69b437fa9947863c2e34f3254073": {
      "emailId": "Sisa-IN",
      "fileName": "Confidential_2013_Ember_Resume.docx",
      "fileId": "NIS-FILE"
    },
    "eec702863d5608fe05adcc086a8861ad30b1e463": {
      "emailId": "Sisa-IN",
      "fileName": "Better Cancer Care, an Action Plan Progress Report.docx",
      "fileId": "NIS-FILE"
    },
    "4a1e07e73dfbc573bcb0b1695f45e8d6b112c54e": {
      "emailId": "Sisa-IN",
      "fileName": "NY2.docx",
      "fileId": "NIS-FILE"
    },
    "df7f0d9cda3374e7d0d15f3fef01ff78e969cf40": {
      "emailId": "Sisa-IN",
      "fileName": "NCIN materials.ppsx",
      "fileId": "NIS-FILE"
    },
    "3e3d5cf9c098f2340cbec45e3dc0a6130d2bff87": {
      "emailId": "Sisa-IN",
      "fileName": "Invitation_2015_Meeting.docx",
      "fileId": "NIS-FILE"
    },
    "986bef04c584c8ffbbe919942ddc787c7485bf8a": {
      "emailId": "Sisa-IN",
      "fileName": "Welcome.ppsx",
      "fileId": "NIS-FILE"
    },
    "950c03387a5052516d9084e311e0463be353d94a": {
      "emailId": "Sisa-IN",
      "fileName": "Save you Privacy!.ppsx",
      "fileId": "NIS-FILE"
    },
    "7424c1816c6449592509466722fb22227571a072": {
      "emailId": "Sisa-IN",
      "fileName": "Lecture1.pptx",
      "fileId": "NIS-FILE"
    },
    "d2e0eb40c979e10fed13d36d4b2c379b64a7d334": {
      "emailId": "Sisa-IN",
      "fileName": "NY3.docx",
      "fileId": "NIS-FILE"
    },
    "db3de78b5d0db9bff3b80a5d86bde2d27a79a999": {
      "emailId": "Sisa-IN",
      "fileName": "How to Access and Clear Your iPhone's Web Browsing History.docx",
      "fileId": "NIS-FILE"
    },
    "909c13f46e5d69d46a93b641677d49339d3c5b7a": {
      "emailId": "Sisa-IN",
      "fileName": "Please keep calm.docx",
      "fileId": "NIS-FILE"
    },
    "96d9ff172e5ca9f03bdf036a08a8d058751fea68": {
      "emailId": "Sisa-IN",
      "fileName": "NY1.docx",
      "fileId": "NIS-FILE"
    },
    "6d8451d61c2615c1dc767c3eecde5f01c2261c91": {
      "emailId": "Sisa-IN",
      "fileName": "Olympics in Brazil.docx",
      "fileId": "NIS-FILE"
    },
    "b95564fd3b0683035241ed9dd9a96fabbb660b76": {
      "emailId": "Sisa-IN",
      "fileName": "NY2.ppsx",
      "fileId": "NIS-FILE"
    },
    "a1d18a61e57e703aa112ef55cb10f980a2a3dc5d": {
      "emailId": "Sisa-IN",
      "fileName": "NY1.ppsx",
      "fileId": "NIS-FILE"
    },
    "49e8181a43b3ca03707ecd65ee22076beb7feb4d": {
      "emailId": "Sisa-IN",
      "fileName": "NY3.ppsx",
      "fileId": "NIS-FILE"
    },
    "3549aeaf487f1cdbbacd66068496fe3c57823fc9": {
      "emailId": "Sisa-IN",
      "fileName": "Cheonan-ham (Cheonan Ship) inquiry.docx",
      "fileId": "NIS-FILE"
    },
    "a90ea5e6475f330fbd1f305bdcc157103c824cf3": {
      "emailId": "Sisa-IN",
      "fileName": "[URGENT]China Export Clearance Information.docx",
      "fileId": "NIS-FILE"
    },
    "5d287c1b14083b36c4e23ae838be42cd897f9785": {
      "emailId": "Sisa-IN",
      "fileName": "2014 WorldCup and 2016 Summer Olympic.ppsx",
      "fileId": "NIS-FILE"
    },
    "1a59795450cc69b437fa9947863c2e34f3254073": {
      "emailId": "Sisa-IN",
      "fileName": "Confidential_2013_Ember_Resume.docx",
      "fileId": "NIS-FILE"
    },
    "eec702863d5608fe05adcc086a8861ad30b1e463": {
      "emailId": "Sisa-IN",
      "fileName": "Better Cancer Care, an Action Plan Progress Report.docx",
      "fileId": "NIS-FILE"
    },
    "4a1e07e73dfbc573bcb0b1695f45e8d6b112c54e": {
      "emailId": "Sisa-IN",
      "fileName": "NY2.docx",
      "fileId": "NIS-FILE"
    },
    "df7f0d9cda3374e7d0d15f3fef01ff78e969cf40": {
      "emailId": "Sisa-IN",
      "fileName": "NCIN materials.ppsx",
      "fileId": "NIS-FILE"
    },
    "3e3d5cf9c098f2340cbec45e3dc0a6130d2bff87": {
      "emailId": "Sisa-IN",
      "fileName": "Invitation_2015_Meeting.docx",
      "fileId": "NIS-FILE"
    },
    "986bef04c584c8ffbbe919942ddc787c7485bf8a": {
      "emailId": "Sisa-IN",
      "fileName": "Welcome.ppsx",
      "fileId": "NIS-FILE"
    },
    "950c03387a5052516d9084e311e0463be353d94a": {
      "emailId": "Sisa-IN",
      "fileName": "Save you Privacy!.ppsx",
      "fileId": "NIS-FILE"
    },
    "7424c1816c6449592509466722fb22227571a072": {
      "emailId": "Sisa-IN",
      "fileName": "Lecture1.pptx",
      "fileId": "NIS-FILE"
    },
    "d2e0eb40c979e10fed13d36d4b2c379b64a7d334": {
      "emailId": "Sisa-IN",
      "fileName": "NY3.docx",
      "fileId": "NIS-FILE"
    },
    "db3de78b5d0db9bff3b80a5d86bde2d27a79a999": {
      "emailId": "Sisa-IN",
      "fileName": "How to Access and Clear Your iPhone's Web Browsing History.docx",
      "fileId": "NIS-FILE"
    },
    "909c13f46e5d69d46a93b641677d49339d3c5b7a": {
      "emailId": "Sisa-IN",
      "fileName": "Please keep calm.docx",
      "fileId": "NIS-FILE"
    },
    "96d9ff172e5ca9f03bdf036a08a8d058751fea68": {
      "emailId": "Sisa-IN",
      "fileName": "NY1.docx",
      "fileId": "NIS-FILE"
    },
    "6d8451d61c2615c1dc767c3eecde5f01c2261c91": {
      "emailId": "Sisa-IN",
      "fileName": "Olympics in Brazil.docx",
      "fileId": "NIS-FILE"
    },
    "b95564fd3b0683035241ed9dd9a96fabbb660b76": {
      "emailId": "Sisa-IN",
      "fileName": "NY2.ppsx",
      "fileId": "NIS-FILE"
    }
  },
  "ver": "0.0.2",
  "title": "open vaccine signature database"
}
                )json";
     return sig;
 }

