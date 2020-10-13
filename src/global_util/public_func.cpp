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

#include <QFile>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDebug>
#include <QGSettings>
#include <QStandardPaths>

#include <stdio.h>
#include <time.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <signal.h>

using namespace std;

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
    FILE *fd;
    struct stat buf;
    char path[100];
    memset(path, 0, 100);
    //崩溃日志路径
    QString strPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation)[0] + "/dde-collapse.log";
    memcpy(path, strPath.toStdString().data(), strPath.length());

    stat(path, &buf);
    if (buf.st_size > 10 * 1024 * 1024) {
        // 超过10兆则清空内容
        fd = fopen(path, "w");
    } else {
        fd = fopen(path, "at");
    }

    if (nullptr == fd) {
        exit(0);
    }
    //捕获异常，打印崩溃日志到配置文件中
    try {
        char szLine[512] = {0};
        time_t t = time(nullptr);
        tm *now = localtime(&t);
        QString log = "#####" + qApp->applicationName() + "#####\n[%04d-%02d-%02d %02d:%02d:%02d][crash signal number:%d]\n";
        sprintf(szLine, log.toStdString().c_str(),
                            now->tm_year + 1900,
                            now->tm_mon + 1,
                            now->tm_mday,
                            now->tm_hour,
                            now->tm_min,
                            now->tm_sec,
                            sig);
        fwrite(szLine, 1, strlen(szLine), fd);

#ifdef __linux
        void *array[MAX_STACK_FRAMES];
        int  size = 0;
        char **strings = nullptr;
        signal(sig, SIG_DFL);
        size = backtrace(array, MAX_STACK_FRAMES);
        strings =static_cast<char **>(backtrace_symbols(array, size));
        for (int i = 0; i < size; ++i) {
            char szLine[512] = {0};
            sprintf(szLine, "%d %s\n", i, strings[i]);
            fwrite(szLine, 1, strlen(szLine), fd);

            std::string symbol(strings[i]);

            size_t pos1 = symbol.find_first_of("[");
            size_t pos2 = symbol.find_last_of("]");
            std::string address = symbol.substr(pos1 + 1, pos2 - pos1 - 1);
            char cmd[128] = {0};
            sprintf(cmd, "addr2line -C -f -e %s", address.c_str()); // 打印当前进程的id和地址
            FILE *fPipe = popen(cmd, "r");
            if (fPipe != nullptr) {
                char buff[1024];
                memset(buff, 0, sizeof(buff));
                char *ret = fgets(buff, sizeof(buff), fPipe);
                pclose(fPipe);
                fwrite(ret, 1, strlen(ret), fd);
            }
        }
        free(strings);
#endif // __linux
    } catch (...) {
        //
    }
    fflush(fd);
    fclose(fd);
    fd = nullptr;
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
