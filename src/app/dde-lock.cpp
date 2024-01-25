// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accessibilitycheckerex.h"
#include "appeventfilter.h"
#include "dbuslockagent.h"
#include "dbuslockfrontservice.h"
#include "dbusshutdownagent.h"
#include "dbusshutdownfrontservice.h"
#include "lockcontent.h"
#include "lockframe.h"
#include "lockworker.h"
#include "modules_loader.h"
#include "multiscreenmanager.h"
#include "sessionbasemodel.h"
#include "warningcontent.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>
#include <DPlatformTheme>

#include <QDBusInterface>
#include <QSurfaceFormat>
#include <unistd.h>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 7))
    DLogManager::registerLoggingRulesWatcher("org.deepin.dde.lock");
#endif
    if (qgetenv("XDG_SESSION_TYPE").contains("wayland")) {
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        QSurfaceFormat::setDefaultFormat(format);
    }

    DApplication *app = nullptr;
#if (DTK_VERSION < DTK_VERSION_CHECK(5, 4, 0, 0))
    app = new DApplication(argc, argv);
#else
    app = DApplication::globalApplication(argc, argv);
#endif

    // qt默认当最后一个窗口析构后，会自动退出程序，这里设置成false，防止插拔时，没有屏幕，导致进程退出
    QApplication::setQuitOnLastWindowClosed(false);
    //解决Qt在Retina屏幕上图片模糊问题
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    app->setOrganizationName("deepin");
    app->setApplicationName("org.deepin.dde.lock");
    app->setApplicationVersion("2015.1.0");

    //注册全局事件过滤器
    AppEventFilter appEventFilter;
    app->installEventFilter(&appEventFilter);
    setAppType(APP_TYPE_LOCK);

    DGuiApplicationHelper::instance()->setSizeMode(DGuiApplicationHelper::SizeMode::NormalMode);
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

    // follow system active color
    QObject::connect(DGuiApplicationHelper::instance()->systemTheme(), &DPlatformTheme::activeColorChanged, [](const QColor &color) {
        auto palette = DGuiApplicationHelper::instance()->applicationPalette();
        palette.setColor(QPalette::Highlight, color);
        DGuiApplicationHelper::instance()->setApplicationPalette(palette);
    });

    DLogManager::setLogFormat("%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [ %{function:-35} %{line}] %{message}\n");
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    DLogManager::registerJournalAppender();

    /* load translation files */
    loadTranslation(QLocale::system().name());

    ModulesLoader::instance().setLoadLoginModule(true);
    ModulesLoader::instance().start(QThread::LowestPriority);

    QCommandLineParser cmdParser;
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();

    QCommandLineOption backend(QStringList() << "d" << "daemon", "start to daemon mode");
    cmdParser.addOption(backend);
    QCommandLineOption switchUser(QStringList() << "s" << "switch", "show user switch");
    cmdParser.addOption(switchUser);

    QCommandLineOption shutdown(QStringList() << "t" << "shutdown", "show shut down");
    cmdParser.addOption(shutdown);

    QCommandLineOption lockScreen(QStringList() << "l" << "lockscreen", "show lock screen");
    cmdParser.addOption(lockScreen);

    QStringList xddd = app->arguments();
    cmdParser.process(*app);

    bool runDaemon = cmdParser.isSet(backend);
    bool showUserList = cmdParser.isSet(switchUser);
    bool showShutdown = cmdParser.isSet(shutdown);
    bool showLockScreen = cmdParser.isSet(lockScreen);

#ifdef  QT_DEBUG
    showLockScreen = true;
#endif

    SessionBaseModel *model = new SessionBaseModel();
    model->setAppType(Lock);
    LockWorker *worker = new LockWorker(model);
    QObject::connect(&appEventFilter, &AppEventFilter::userIsActive, worker, &LockWorker::restartResetSessionTimer);

    if (worker->isLocked()) {
        //如果当前Session被锁定，则启动dde-lock时直接进入锁屏状态
        //兼容在desktop文件中添加-l启动参数来解决杀掉dde-lock后启动器无法打开的问题,BUG91126
        //同时避免在desktop文件中添加-l启动参数在正常启动时就直接进入锁屏而无法调用任务栏关机按钮中接口的问题,BUG92030
        showLockScreen = true;
    }

    DBusLockAgent lockAgent;
    lockAgent.setModel(model);
    DBusLockFrontService lockService(&lockAgent);

    DBusShutdownAgent shutdownAgent;
    shutdownAgent.setModel(model);
    DBusShutdownFrontService shutdownServices(&shutdownAgent);

    LockContent::instance()->init(model);
    WarningContent::instance()->setModel(model);

    QObject::connect(LockContent::instance(), &LockContent::requestSwitchToUser, worker, &LockWorker::switchToUser);
    QObject::connect(LockContent::instance(), &LockContent::requestSetLayout, worker, &LockWorker::setKeyboardLayout);
    QObject::connect(LockContent::instance(), &LockContent::requestStartAuthentication, worker, &LockWorker::startAuthentication);
    QObject::connect(LockContent::instance(), &LockContent::sendTokenToAuth, worker, &LockWorker::sendTokenToAuth);
    QObject::connect(LockContent::instance(), &LockContent::requestEndAuthentication, worker, &LockWorker::onEndAuthentication);
    QObject::connect(LockContent::instance(), &LockContent::authFinished, worker, &LockWorker::authFinishedAction);
    QObject::connect(LockContent::instance(), &LockContent::requestCheckAccount, worker, &LockWorker::checkAccount);
    QObject::connect(LockContent::instance(), &LockContent::requestLockFrameHide, [model] {
        model->setVisible(false);
    });
    QObject::connect(LockContent::instance(), &LockContent::noPasswordLoginChanged, worker, &LockWorker::onNoPasswordLoginChanged);
    QObject::connect(WarningContent::instance(), &WarningContent::requestLockFrameHide, [model] {
        model->setVisible(false);
    });

    auto createFrame = [&](QPointer<QScreen> screen, int count) -> QWidget* {
        LockFrame *lockFrame = new LockFrame(model);
        // 创建Frame可能会花费数百毫秒，这个和机器性能有关，在此过程完成后，screen可能已经析构了
        // 在wayland的环境插拔屏幕或者显卡驱动有问题时可能会出现此类问题
        if (screen.isNull()) {
            lockFrame->deleteLater();
            lockFrame = nullptr;
            qCWarning(DDE_SHELL) << "Screen was released when the frame was created";
            return nullptr;
        }
        lockFrame->setScreen(screen, count <= 0);
        QObject::connect(model, &SessionBaseModel::visibleChanged, lockFrame, [lockFrame](const bool visible) {
            lockFrame->setVisible(visible);
            QTimer::singleShot(300, lockFrame, [lockFrame] {
                qCDebug(DDE_SHELL) << "Update frame after lock frame visible changed";
                lockFrame->update();
            });
        });
        QObject::connect(model, &SessionBaseModel::visibleChanged, lockFrame,[&](bool visible) {
            emit lockService.Visible(visible);
        });
        QObject::connect(model, &SessionBaseModel::onStatusChanged, lockFrame,[&](SessionBaseModel::ModeStatus status) {
            emit shutdownServices.Visible(model->visible() && (status == SessionBaseModel::ModeStatus::PowerMode || status == SessionBaseModel::ModeStatus::ShutDownMode));
        });
        QObject::connect(lockFrame, &LockFrame::requestEnableHotzone, worker, &LockWorker::enableZoneDetected, Qt::UniqueConnection);
        QObject::connect(lockFrame, &LockFrame::sendKeyValue, [&](QString key) {
             emit lockService.ChangKey(key);
        });
        if (model->isUseWayland()) {
            QObject::connect(lockFrame, &LockFrame::requestDisableGlobalShortcutsForWayland, worker, &LockWorker::disableGlobalShortcutsForWayland);
        }

        lockFrame->setVisible(model->visible());
        emit lockService.Visible(model->visible());
        return lockFrame;
    };

    MultiScreenManager multi_screen_manager;
    multi_screen_manager.register_for_multi_screen(createFrame);

#if defined(DSS_CHECK_ACCESSIBILITY) && defined(QT_DEBUG)
    AccessibilityCheckerEx checker;
    //Dtk::Widget::DBlurEffectWidget 添加accessibleName、objectName都会检查出内存泄露，原因不知
    checker.addIgnoreClasses(QStringList()
                          << "Dtk::Widget::DBlurEffectWidget");
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();
#endif

    QObject::connect(model, &SessionBaseModel::visibleChanged, &multi_screen_manager, &MultiScreenManager::startRaiseContentFrame);

    // 加载Tray插件
    ModulesLoader::instance().setLoadLoginModule(false);
    ModulesLoader::instance().start(QThread::LowestPriority);

    auto isSingle = app->setSingleInstance(QString("dde-lock%1").arg(getuid()), DApplication::UserScope);
    QDBusConnection conn = QDBusConnection::sessionBus();
    int ret = 0;
    if (!conn.registerService(DBUS_LOCK_NAME) ||
        !conn.registerObject(DBUS_LOCK_PATH, &lockAgent) ||
        !conn.registerService(DBUS_SHUTDOWN_NAME) ||
        !conn.registerObject(DBUS_SHUTDOWN_PATH, &shutdownAgent) ||
        !isSingle) {
        qCWarning(DDE_SHELL) << "Register DBus failed, maybe lock front is running, error: " << conn.lastError();

        if (!runDaemon) {
            const char *lockFrontInter = "com.deepin.dde.lockFront";
            const char *shutdownFrontInter = "com.deepin.dde.shutdownFront";
            if (showUserList) {
                QDBusInterface ifc(DBUS_LOCK_NAME, DBUS_LOCK_PATH, lockFrontInter, QDBusConnection::sessionBus(), nullptr);
                ifc.asyncCall("ShowUserList");
            } else if (showShutdown) {
                QDBusInterface ifc(DBUS_SHUTDOWN_NAME, DBUS_SHUTDOWN_PATH, shutdownFrontInter, QDBusConnection::sessionBus(), nullptr);
                ifc.asyncCall("Show");
            } else if (showLockScreen) {
                do {
                    // 当前会话被锁定时，强制等待10ms后再获取一次锁定状态，如果变了直接退出，避免快速锁屏解锁时，lock状态未及时更新导致再启动dde-lock直接进入了锁屏状态,From bug: 200415
                    if (worker->isLocked()) {
                        QThread::msleep(10);

                        if (!worker->isLocked())
                            return 0;
                    }

                } while (false);

                QDBusInterface ifc(DBUS_LOCK_NAME, DBUS_LOCK_PATH, lockFrontInter, QDBusConnection::sessionBus(), nullptr);
                ifc.asyncCall("Show");
            }
        }
    } else {
        if (!runDaemon) {
            if (showUserList) {
                emit model->showUserList();
            } else if (showShutdown) {
                emit model->showShutdown();
            } else if (showLockScreen) {
                emit model->showLockScreen();
            }
        }
        ret = app->exec();
    }
    return ret;
}
