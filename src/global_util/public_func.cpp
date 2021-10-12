/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             Hualet <mr.asianwang@gmail.com>
 *             kirigaya <kirigaya@mkacg.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             Hualet <mr.asianwang@gmail.com>
 *             kirigaya <kirigaya@mkacg.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "public_func.h"
#include "constants.h"

#include <DConfig>

#include <QFile>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDebug>
#include <QGSettings>
#include <QStandardPaths>
#include <QProcess>
#include <QDateTime>
#include <QDir>

#include <stdio.h>
#include <time.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <signal.h>

using namespace std;

DCORE_USE_NAMESPACE

QPixmap loadPixmap(const QString &file, const QSize& size)
{

    if(!QFile::exists(file)){
        return QPixmap(DDESESSIONCC::LAYOUTBUTTON_HEIGHT,DDESESSIONCC::LAYOUTBUTTON_HEIGHT);
    }

    qreal ratio = 1.0;
    qreal devicePixel = qApp->devicePixelRatio();

    QPixmap pixmap;

    if (!qFuzzyCompare(ratio, devicePixel) || size.isValid()) {
        QImageReader reader;
        reader.setFileName(qt_findAtNxFile(file, devicePixel, &ratio));
        if (reader.canRead()) {
            reader.setScaledSize((size.isNull() ? reader.size() : reader.size().scaled(size, Qt::KeepAspectRatio)) * (devicePixel / ratio));
            pixmap = QPixmap::fromImage(reader.read());
            pixmap.setDevicePixelRatio(devicePixel);
        }
    } else {
        pixmap.load(file);
    }

    return pixmap;
}

/**
 * @brief 获取图像共享内存
 *
 * @param uid 当前用户ID
 * @param purpose 图像用途，1是锁屏、关机、登录，2是启动器，3-19是工作区
 * @return QString Qt的共享内存key
 */
QString readSharedImage(uid_t uid, int purpose)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("com.deepin.dde.preload", "/com/deepin/dde/preload", "com.deepin.dde.preload", "requestSource");
    QList<QVariant> args;
    args.append(int(uid));
    args.append(purpose);
    msg.setArguments(args);
    QString shareKey;
    QDBusMessage ret = QDBusConnection::sessionBus().call(msg);
    if (ret.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "readSharedImage fail. user: " << uid << ", purpose: " << purpose << ", detail: " << ret;
    } else {
        QDBusReply<QString> reply(ret);
        shareKey = reply.value();
    }
#ifdef QT_DEBUG
    qInfo() << __FILE__ << ", " << Q_FUNC_INFO << " user: " << uid << ", purpose: " << purpose << " share memory key: " << shareKey;
#endif
    return shareKey;
}


/**
 * @brief 是否使用域管认证。
 *
 * @return true 使用域管认证
 * @return false 使用系统认证
 */
bool isDeepinAuth()
{
    const char* controlId = "com.deepin.dde.auth.control";
    const char* controlPath = "/com/deepin/dde/auth/control/";
    if (QGSettings::isSchemaInstalled (controlId)) {
        QGSettings controlObj (controlId, controlPath);
        bool bUseDeepinAuth =  controlObj.get ("use-deepin-auth").toBool();
    #ifdef QT_DEBUG
        qDebug() << "----------use deepin auth: " << bUseDeepinAuth;
    #endif
        return bUseDeepinAuth;
    }
    return true;
}

const int MAX_STACK_FRAMES = 128;

void sig_crash(int sig)
{
    QDir dir(QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0]);
    dir.cdUp();
    QString filePath = dir.path() + "/dde-collapse.log";

    QFile *file = new QFile(filePath);

    if (!file->open(QIODevice::Text | QIODevice::Append)) {
        qDebug() << file->errorString();
        exit(0);
    }

    if (file->size() >= 10 * 1024 * 1024) {
        // 清空原有内容
        file->close();
        if (file->open(QIODevice::Text | QIODevice::Truncate)) {
            qDebug() << file->errorString();
            exit(0);
        }
    }

    // 捕获异常，打印崩溃日志到配置文件中
    try {
        QString head = "\n#####" + qApp->applicationName() + "#####\n"
                + QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss:zzz]")
                + "[crash signal number:" + QString::number(sig) + "]\n";
        file->write(head.toUtf8());

#ifdef Q_OS_LINUX
        void *array[MAX_STACK_FRAMES];
        size_t size = 0;
        char **strings = nullptr;
        size_t i;
        signal(sig, SIG_DFL);
        size = static_cast<size_t>(backtrace(array, MAX_STACK_FRAMES));
        strings = backtrace_symbols(array, int(size));
        for (i = 0; i < size; ++i) {
            QString line = QString::number(i) + " " + QString::fromStdString(strings[i]) + "\n";
            file->write(line.toUtf8());

            std::string symbol(strings[i]);
            QString strSymbol = QString::fromStdString(symbol);
            int pos1 = strSymbol.indexOf("[");
            int pos2 = strSymbol.lastIndexOf("]");
            QString address = strSymbol.mid(pos1 + 1,pos2 - pos1 - 1);

            // 按照内存地址找到对应代码的行号
            QString cmd = "addr2line -C -f -e " + qApp->applicationName() + " " + address;
            QProcess *p = new QProcess;
            p->setReadChannel(QProcess::StandardOutput);
            p->start(cmd);
            p->waitForFinished();
            p->waitForReadyRead();
            file->write(p->readAllStandardOutput());
            delete p;
            p = nullptr;
        }
        free(strings);
#endif // __linux
    } catch (...) {
        //
    }
    file->close();
    delete file;
    file = nullptr;
    exit(0);
}

void init_sig_crash()
{
    signal(SIGTERM, sig_crash);
    signal(SIGSEGV, sig_crash);
    signal(SIGILL, sig_crash);
    signal(SIGINT, sig_crash);
    signal(SIGABRT, sig_crash);
    signal(SIGFPE, sig_crash);
}

uint timeFromString(QString time)
{
    if (time.isEmpty()) {
        return QDateTime::currentDateTime().toTime_t();
    }
    return QDateTime::fromString(time, Qt::ISODateWithMs).toLocalTime().toTime_t();
}

/**
 * @brief getDConfigValue 根据传入的\a key获取配置项的值，获取失败返回默认值
 * @param key 配置项键值
 * @param defaultValue 默认返回值，为避免出现返回值错误导致程序异常的问题，此参数必填
 * @param configFileName 配置文件名称
 * @return 配置项的值
 */
QVariant getDConfigValue(const QString &key, const QVariant &defaultValue, const QString &configFileName)
{
    DConfig config(configFileName);

    if (!config.isValid()) {
        qWarning() << QString("DConfig is invalid, name:[%1], subpath[%2].").
                        arg(config.name(), config.subpath());
        return defaultValue;
    }

    if (config.keyList().contains(key))
        return config.value(key);

    return defaultValue;
}