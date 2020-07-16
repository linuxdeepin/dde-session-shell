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

#include <DApplication>
#include <QtCore/QTranslator>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDBusAbstractInterface>
#include <QStandardPaths>
#include <DDBusSender>
#include <DGuiApplicationHelper>

#include <DLog>
#include <string>

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <signal.h>

#include "src/dde-shutdown/app/shutdownframe.h"
#include "src/session-widgets/sessionbasemodel.h"
#include "src/dde-shutdown/shutdownworker.h"
#include "src/widgets/propertygroup.h"
#include "src/global_util/multiscreenmanager.h"

#include "src/dde-shutdown/dbusshutdownagent.h"

const QString DBUS_PATH = "/com/deepin/dde/shutdownFront";
const QString DBUS_NAME = "com.deepin.dde.shutdownFront";

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
    qDebug() << path;

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
        int nLen1 = sprintf(szLine, log.toStdString().c_str(),
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
        size_t size = 0;
        char **strings = nullptr;
        size_t i, j;
        signal(sig, SIG_DFL);
        size = backtrace(array, MAX_STACK_FRAMES);
        strings = (char **)backtrace_symbols(array, size);
        for (i = 0; i < size; ++i) {
            char szLine[512] = {0};
            sprintf(szLine, "%d %s\n", i, strings[i]);
            fwrite(szLine, 1, strlen(szLine), fd);

            std::string symbol(strings[i]);

            size_t pos1 = symbol.find_first_of("[");
            size_t pos2 = symbol.find_last_of("]");
            std::string address = symbol.substr(pos1 + 1, pos2 - pos1 - 1);
            char cmd[128] = {0};
            sprintf(cmd, "addr2line -C -f -e dde-shutdown %s", address.c_str()); // 打印当前进程的id和地址
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
    DApplication app(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-shutdown");
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
    pa.setColor(QPalette::Normal, DPalette::AlternateBase, QColor(0, 0, 0, 76));
    pa.setColor(QPalette::Normal, DPalette::Button, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::Light, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::Dark, QColor(255, 255, 255, 76));
    pa.setColor(QPalette::Normal, DPalette::ButtonText, QColor("#FFFFFF"));
    DGuiApplicationHelper::generatePaletteColor(pa, DPalette::WindowText, DGuiApplicationHelper::LightType);
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

    // NOTE: it's better to be -h/--show, but some apps may have used the binary to show
    // the shutdown frame directly, we need to keep the default behavier to that.
    QCommandLineOption daemon(QStringList() << "d" << "daemon",
                              "start the program, but do not show the window.");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(daemon);
    parser.process(app);

    QDBusConnection session = QDBusConnection::sessionBus();
    if (!session.registerService(DBUS_NAME) ||
            !app.setSingleInstance(QString("dde-shutdown%1").arg(getuid()), DApplication::UserScope)) {
        qWarning() << "dde-shutdown is running...";

        if (!parser.isSet(daemon)) {
            DDBusSender()
            .service(DBUS_NAME)
            .interface(DBUS_NAME)
            .path(DBUS_PATH)
            .method("Show")
            .call();
        }

        return 0;
    } else {
        qDebug() << "dbus registration success.";

        SessionBaseModel *model = new SessionBaseModel(SessionBaseModel::AuthType::UnknowAuthType);
        ShutdownWorker *worker = new ShutdownWorker(model); //

        DBusShutdownAgent *dbusAgent = new DBusShutdownAgent;
        PropertyGroup *property_group = new PropertyGroup(worker);

        property_group->addProperty("contentVisible");

        auto createFrame = [&](QScreen * screen) -> QWidget* {
            ShutdownFrame *frame = new ShutdownFrame(model);
            dbusAgent->addFrame(frame);
            frame->setScreen(screen);
            property_group->addObject(frame);
            QObject::connect(model, &SessionBaseModel::visibleChanged, frame, &ShutdownFrame::setVisible);
            QObject::connect(frame, &ShutdownFrame::requestEnableHotzone, worker, &ShutdownWorker::enableZoneDetected);
            QObject::connect(frame, &ShutdownFrame::destroyed, property_group, &PropertyGroup::removeObject);
            QObject::connect(frame, &ShutdownFrame::destroyed, frame, [ = ] {
                dbusAgent->removeFrame(frame);
            });
            QObject::connect(frame, &ShutdownFrame::buttonClicked, frame, [ = ](const Actions action)
            {
                dbusAgent->sync(action);
            });

            frame->setVisible(model->isShow());
            return frame;
        };

        MultiScreenManager multi_screen_manager;
        multi_screen_manager.register_for_mutil_screen(createFrame);

        QObject::connect(model, &SessionBaseModel::visibleChanged, &multi_screen_manager, &MultiScreenManager::startRaiseContentFrame);

        if (!parser.isSet(daemon)) {
            model->setIsShow(true);
        }

        ShutdownFrontDBus adaptor(dbusAgent,model); Q_UNUSED(adaptor);
        QDBusConnection::sessionBus().registerObject(DBUS_PATH, dbusAgent);

        return app.exec();
    }



    qWarning() << "have unknow error!";
    return -1;
}
