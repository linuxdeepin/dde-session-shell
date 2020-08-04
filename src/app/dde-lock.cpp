/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#include <DLog>

#include "src/dde-lock//lockframe.h"
#include "src/dde-lock/dbus/dbuslockfrontservice.h"
#include "src/dde-lock/dbus/dbuslockagent.h"
#include "src/global_util/multiscreenmanager.h"

#include "src/session-widgets/lockcontent.h"
#include "src/dde-lock/lockworker.h"
#include "src/session-widgets/sessionbasemodel.h"
#include "src/widgets/propertygroup.h"

#include <QLabel>
#include <QScreen>
#include <QWindow>
#include <dapplication.h>
#include <string>
#include <QDBusInterface>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <DGuiApplicationHelper>
#include <QMovie>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <dapplication.h>
#include <signal.h>


DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

const int MAX_STACK_FRAMES = 128;

using namespace std;

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
            sprintf(cmd, "addr2line -C -f -e dde-lock %s", address.c_str()); // 打印当前进程的id和地址
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

int main(int argc, char *argv[])
{
    if(DGuiApplicationHelper::isXWindowPlatform()) DApplication::loadDXcbPlugin();
    DApplication app(argc, argv);
    //解决Qt在Retina屏幕上图片模糊问题
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-lock");
    app.setApplicationVersion("2015.1.0");
    //崩溃信号
    signal(SIGTERM, sig_crash);
    signal(SIGSEGV, sig_crash);
    signal(SIGILL, sig_crash);
    signal(SIGINT, sig_crash);
    signal(SIGABRT, sig_crash);
    signal(SIGFPE, sig_crash);

    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
    DPalette pa = DGuiApplicationHelper::instance()->applicationPalette();
    pa.setColor(QPalette::Normal, DPalette::WindowText, QColor("#FFFFFF"));
    pa.setColor(QPalette::Normal, DPalette::Text, QColor("#FFFFFF"));
    pa.setColor(QPalette::Normal, DPalette::AlternateBase, QColor(0, 0, 0, 76));
    pa.setColor(QPalette::Normal, DPalette::Button, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::Light, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::Dark, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::ButtonText, QColor("#FFFFFF"));
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::WindowText, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::Text, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::AlternateBase, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::Button, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::Light, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::Dark, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::ButtonText, DGuiApplicationHelper::LightType);
    DGuiApplicationHelper::instance()->setApplicationPalette(pa);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QTranslator translator;
    translator.load("/usr/share/dde-session-shell/translations/dde-session-shell_" + QLocale::system().name());
    app.installTranslator(&translator);

    QCommandLineParser cmdParser;
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();

    QCommandLineOption backend(QStringList() << "d" << "daemon", "start to daemon mode");
    cmdParser.addOption(backend);
    QCommandLineOption switchUser(QStringList() << "s" << "switch", "show user switch");
    cmdParser.addOption(switchUser);
    cmdParser.process(app);

    bool runDaemon = cmdParser.isSet(backend);
    bool showUserList = cmdParser.isSet(switchUser);

    SessionBaseModel *model = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
    LockWorker *worker = new LockWorker(model); //
    PropertyGroup *property_group = new PropertyGroup(worker);
    qDebug() << "Supported animated file formats:" << QMovie::supportedFormats();

    property_group->addProperty("contentVisible");

    DBusLockAgent agent;
    agent.setModel(model);
    DBusLockFrontService service(&agent);

    auto createFrame = [&] (QScreen *screen) -> QWidget* {
        LockFrame *lockFrame = new LockFrame(model);
        lockFrame->setScreen(screen);
        property_group->addObject(lockFrame);
        QObject::connect(lockFrame, &LockFrame::requestSwitchToUser, worker, &LockWorker::switchToUser);
        QObject::connect(lockFrame, &LockFrame::requestAuthUser, worker, &LockWorker::authUser);
        QObject::connect(model, &SessionBaseModel::visibleChanged, lockFrame, &LockFrame::setVisible);
        QObject::connect(model, &SessionBaseModel::visibleChanged, lockFrame,[&](bool visible) {
            emit service.Visible(visible);
        });
        QObject::connect(model, &SessionBaseModel::showUserList, lockFrame, &LockFrame::showUserList);
        QObject::connect(lockFrame, &LockFrame::requestSetLayout, worker, &LockWorker::setLayout);
        QObject::connect(lockFrame, &LockFrame::requestEnableHotzone, worker, &LockWorker::enableZoneDetected, Qt::UniqueConnection);
        QObject::connect(lockFrame, &LockFrame::destroyed, property_group, &PropertyGroup::removeObject);
        QObject::connect(lockFrame, &LockFrame::sendKeyValue, [&](QString key) {
             emit service.ChangKey(key);
        });

        if (isDeepinAuth()) {
            lockFrame->setVisible(model->isShow());
            emit service.Visible(true);
        } else {
            model->setIsShow(false);
            lockFrame->setVisible(false);
        }
        
        return lockFrame;
    };

    MultiScreenManager multi_screen_manager;
    multi_screen_manager.register_for_mutil_screen(createFrame);

    QObject::connect(model, &SessionBaseModel::visibleChanged, &multi_screen_manager, &MultiScreenManager::startRaiseContentFrame);

    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService(DBUS_NAME) ||
            !conn.registerObject("/com/deepin/dde/lockFront", &agent) ||
            !app.setSingleInstance(QString("dde-lock%1").arg(getuid()), DApplication::UserScope)) {
        qDebug() << "register dbus failed"<< "maybe lockFront is running..." << conn.lastError();

        if (!runDaemon) {
            const char *interface = "com.deepin.dde.lockFront";
            QDBusInterface ifc(DBUS_NAME, DBUS_PATH, interface, QDBusConnection::sessionBus(), nullptr);
            if (showUserList) {
                ifc.asyncCall("ShowUserList");
            } else {
                ifc.asyncCall("Show");
            }
        }
    } else {
        if (isDeepinAuth() && !runDaemon) {
            if (showUserList) {
                emit model->showUserList();
            } else {
                model->setIsShow(true);
            }
        }
        app.exec();
    }

    return 0;
}
