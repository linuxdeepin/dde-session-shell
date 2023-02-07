// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockcontent.h"

#include "controlwidget.h"
#include "logowidget.h"
#include "mfa_widget.h"
#include "modules_loader.h"
#include "sessionbasemodel.h"
#include "sfa_widget.h"
#include "shutdownwidget.h"
#include "userframelist.h"
#include "virtualkbinstance.h"
#include "fullscreenbackground.h"
#include "plugin_manager.h"
#include "fullmanagedauthwidget.h"

#include <DDBusSender>

#include <QLocalSocket>
#include <QMouseEvent>

using namespace dss;
using namespace dss::module;
DCORE_USE_NAMESPACE

Q_GLOBAL_STATIC(LockContent, lockContent)

LockContent::LockContent(QWidget *parent)
    : SessionBaseWindow(parent)
    , m_model(nullptr)
    , m_controlWidget(nullptr)
    , m_centerTopWidget(nullptr)
    , m_virtualKB(nullptr)
    , m_wmInter(new com::deepin::wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , m_sfaWidget(nullptr)
    , m_mfaWidget(nullptr)
    , m_authWidget(nullptr)
    , m_fmaWidget(nullptr)
    , m_userListWidget(nullptr)
    , m_localServer(new QLocalServer(this))
    , m_currentModeStatus(SessionBaseModel::ModeStatus::NoStatus)
    , m_initialized(false)
    , m_isUserSwitchVisible(false)
{

}

LockContent* LockContent::instance()
{
    return lockContent;
}

void LockContent::init(SessionBaseModel *model)
{
    if (m_initialized) {
        return;
    }
    m_initialized = true;
    m_model = model;
    // 在已显示关机或用户列表界面时，再插入另外一个显示器，会新构建一个LockContent，此时会设置为PasswordMode造成界面状态异常
    if (!m_model->visible()) {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    }

    initUI();
    initConnections();

    if (model->appType() == AuthCommon::Lock) {
        setMPRISEnable(model->currentModeState() != SessionBaseModel::ModeStatus::ShutDownMode);
    }

    QTimer::singleShot(0, this, [=] {
        onCurrentUserChanged(model->currentUser());
        onUserListChanged(model->isServerModel() ? model->loginedUserList() : model->userList());
    });

    // 创建套接字连接，用于与重置密码弹窗通讯
    m_localServer->setMaxPendingConnections(1);
    m_localServer->setSocketOptions(QLocalServer::WorldAccessOption);
    // 将greeter和lock的服务名称分开
    // 如果服务相同，但是创建套接字文件的用户不一样，greeter和lock不能删除对方的套接字文件，造成锁屏无法监听服务。
    QString serverName = QString("GrabKeyboard_") + (m_model->appType() == AuthCommon::Login ? "greeter" : ("lock_" + m_model->currentUser()->name()));
    // 将之前的server删除，如果是旧文件，即使监听成功，客户端也无法连接。
    QLocalServer::removeServer(serverName);
    if (!m_localServer->listen(serverName)) { // 监听特定的连接
        qWarning() << "listen failed!" << m_localServer->errorString();
    } else {
        qDebug() << "listen success!";
    }
}

void LockContent::initUI()
{
    m_centerTopWidget = new CenterTopWidget(this);
    setCenterTopWidget(m_centerTopWidget);

    m_shutdownFrame = new ShutdownWidget;
    m_shutdownFrame->setAccessibleName("ShutdownFrame");
    m_shutdownFrame->setModel(m_model);

    m_logoWidget = new LogoWidget;
    m_logoWidget->setAccessibleName("LogoWidget");
    setLeftBottomWidget(m_logoWidget);

    m_controlWidget = new ControlWidget(m_model);
    m_controlWidget->setAccessibleName("ControlWidget");
    setRightBottomWidget(m_controlWidget);

    if (m_model->getAuthProperty().MFAFlag) {
        initMFAWidget();
    } else {
        initSFAWidget();
    }
    m_authWidget->hide();

    initUserListWidget();

    initFMAWidget();
}

void LockContent::initConnections()
{
    connect(m_model, &SessionBaseModel::currentUserChanged, this, &LockContent::onCurrentUserChanged);
    connect(m_controlWidget, &ControlWidget::requestSwitchUser, this, [ = ] {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
        emit requestEndAuthentication(m_model->currentUser()->name(), AuthCommon::AT_All);
    });
    connect(m_controlWidget, &ControlWidget::requestShutdown, this, [ = ] {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PowerMode);
    });
    connect(m_controlWidget, &ControlWidget::requestSwitchVirtualKB, this, &LockContent::toggleVirtualKB);
    connect(m_controlWidget, &ControlWidget::requestShowModule, this, &LockContent::showModule);

    // 刷新背景单独与onStatusChanged信号连接，避免在showEvent事件时调用onStatusChanged而重复刷新背景，减少刷新次数
    connect(m_model, &SessionBaseModel::onStatusChanged, this, &LockContent::onStatusChanged);

    //在锁屏显示时，启动onboard进程，锁屏结束时结束onboard进程
    auto initVirtualKB = [&](bool hasVirtualKB) {
        if (hasVirtualKB && !m_virtualKB) {
            connect(&VirtualKBInstance::Instance(), &VirtualKBInstance::initFinished, this, [&] {
                m_virtualKB = VirtualKBInstance::Instance().virtualKBWidget();
                m_controlWidget->setVirtualKBVisible(true);
            }, Qt::QueuedConnection);
            VirtualKBInstance::Instance().init();
        } else {
            VirtualKBInstance::Instance().stopVirtualKBProcess();
            m_virtualKB = nullptr;
            m_controlWidget->setVirtualKBVisible(false);
        }
    };

    connect(m_model, &SessionBaseModel::hasVirtualKBChanged, this, initVirtualKB, Qt::QueuedConnection);
    connect(m_model, &SessionBaseModel::userListChanged, this, &LockContent::onUserListChanged);
    connect(m_model, &SessionBaseModel::userListLoginedChanged, this, &LockContent::onUserListChanged);
    connect(m_model, &SessionBaseModel::authFinished, this, [ this ] (bool successful) {
        if (successful)
            setVisible(false);
        restoreMode();
    });
    connect(m_model, &SessionBaseModel::MFAFlagChanged, this, [this](const bool isMFA) {
        isMFA ? initMFAWidget() : initSFAWidget();
        // 当前中间窗口为空或者中间窗口就是验证窗口的时候显示验证窗口
        if (!m_centerWidget || m_centerWidget == m_authWidget)
            setCenterContent(m_authWidget, 0, Qt::AlignTop, calcTopSpacing(m_authWidget->getTopSpacing()));
    });

    connect(m_wmInter, &__wm::WorkspaceSwitched, this, &LockContent::currentWorkspaceChanged);
    connect(m_localServer, &QLocalServer::newConnection, this, &LockContent::onNewConnection);
    connect(m_controlWidget, &ControlWidget::notifyKeyboardLayoutHidden, this, [this]{
        if (!m_model->isUseWayland() && isVisible() && window()->windowHandle()) {
            qDebug() << "Grab keyboard after keyboard layout hidden";
            window()->windowHandle()->setKeyboardGrabEnabled(true);
        }
    });

    connect(m_model, &SessionBaseModel::showUserList, this, &LockContent::showUserList);
    connect(m_model, &SessionBaseModel::showLockScreen, this, &LockContent::showLockScreen);
    connect(m_model, &SessionBaseModel::showShutdown, this, &LockContent::showShutdown);
}

/**
 * @brief 初始化多因认证界面
 */
void LockContent::initMFAWidget()
{
    qDebug() << "LockContent::initMFAWidget:" << m_sfaWidget << m_mfaWidget;
    if (m_sfaWidget) {
        m_sfaWidget->hide();
        delete m_sfaWidget;
        m_sfaWidget = nullptr;
    }
    if (m_mfaWidget) {
        m_authWidget = m_mfaWidget;
        return;
    }
    m_mfaWidget = new MFAWidget(this);
    m_mfaWidget->setModel(m_model);
    m_authWidget = m_mfaWidget;

    connect(m_mfaWidget, &MFAWidget::requestStartAuthentication, this, &LockContent::requestStartAuthentication);
    connect(m_mfaWidget, &MFAWidget::sendTokenToAuth, this, &LockContent::sendTokenToAuth);
    connect(m_mfaWidget, &MFAWidget::requestEndAuthentication, this, &LockContent::requestEndAuthentication);
    connect(m_mfaWidget, &MFAWidget::requestCheckAccount, this, &LockContent::requestCheckAccount);
}

/**
 * @brief 初始化单因认证界面
 */
void LockContent::initSFAWidget()
{
    qDebug() << "LockContent::initSFAWidget:" << m_sfaWidget << m_mfaWidget;
    if (m_mfaWidget) {
        m_mfaWidget->hide();
        delete m_mfaWidget;
        m_mfaWidget = nullptr;
    }
    if (m_sfaWidget) {
        m_authWidget = m_sfaWidget;
        return;
    }
    m_sfaWidget = new SFAWidget(this);
    m_sfaWidget->setModel(m_model);
    m_authWidget = m_sfaWidget;

    connect(m_sfaWidget, &SFAWidget::requestStartAuthentication, this, &LockContent::requestStartAuthentication);
    connect(m_sfaWidget, &SFAWidget::sendTokenToAuth, this, &LockContent::sendTokenToAuth);
    connect(m_sfaWidget, &SFAWidget::requestEndAuthentication, this, &LockContent::requestEndAuthentication);
    connect(m_sfaWidget, &SFAWidget::requestCheckAccount, this, &LockContent::requestCheckAccount);
    connect(m_sfaWidget, &SFAWidget::authFinished, this, &LockContent::authFinished);
    connect(m_sfaWidget, &SFAWidget::updateParentLayout, this, [this] {
        m_centerSpacerItem->changeSize(0, calcTopSpacing(m_sfaWidget->getTopSpacing()));
        m_centerVLayout->invalidate();
    });
}

// init full managed plugin widget
void LockContent::initFMAWidget()
{
    qDebug() << "LockContent::initFMAWidget" << m_fmaWidget;
    if (m_fmaWidget) {
        m_fmaWidget->hide();
        delete m_fmaWidget;
        m_fmaWidget = nullptr;
    }

    m_fmaWidget = new FullManagedAuthWidget();
    m_fmaWidget->setModel(m_model);
    if (m_fmaWidget->isPluginLoaded()) {
        setFullManagedLoginWidget(m_fmaWidget);
    }
    connect(m_fmaWidget, &FullManagedAuthWidget::requestStartAuthentication, this, &LockContent::requestStartAuthentication);
    connect(m_fmaWidget, &FullManagedAuthWidget::sendTokenToAuth, this, &LockContent::sendTokenToAuth);
    connect(m_fmaWidget, &FullManagedAuthWidget::requestEndAuthentication, this, &LockContent::requestEndAuthentication);
    connect(m_fmaWidget, &FullManagedAuthWidget::requestCheckAccount, this, &LockContent::requestCheckAccount);
    connect(m_fmaWidget, &FullManagedAuthWidget::authFinished, this, &LockContent::authFinished);
}

/**
 * @brief 初始化用户列表界面
 */
void LockContent::initUserListWidget()
{
    if (m_userListWidget) {
        return;
    }
    m_userListWidget = new UserFrameList(this);
    m_userListWidget->setModel(m_model);
    m_userListWidget->setVisible(false);

    connect(m_userListWidget, &UserFrameList::clicked, this, &LockContent::restoreMode);
    connect(m_userListWidget, &UserFrameList::requestSwitchUser, this, &LockContent::requestSwitchToUser);
}

void LockContent::onCurrentUserChanged(std::shared_ptr<User> user)
{
    if (user.get() == nullptr) return; // if dbus is async

    //如果是锁屏就用系统语言，如果是登陆界面就用用户语言
    auto locale = qApp->applicationName() == "dde-lock" ? QLocale::system().name() : user->locale();
    m_logoWidget->updateLocale(locale);

    for (auto connect : m_currentUserConnects) {
        m_user.get()->disconnect(connect);
    }

    m_currentUserConnects.clear();

    m_user = user;

    m_currentUserConnects << connect(user.get(), &User::greeterBackgroundChanged, this, &LockContent::updateGreeterBackgroundPath, Qt::UniqueConnection)
                          << connect(user.get(), &User::desktopBackgroundChanged, this, &LockContent::updateDesktopBackgroundPath, Qt::UniqueConnection);

    m_centerTopWidget->setCurrentUser(user.get());
    m_logoWidget->updateLocale(locale);
}

void LockContent::pushPasswordFrame()
{
    setCenterContent(m_authWidget, 0, Qt::AlignTop, calcTopSpacing(m_authWidget->getTopSpacing()));

    m_authWidget->syncResetPasswordUI();

    if (!m_fmaWidget->isPluginLoaded()) {
        showDefaultFrame();
        return;
    }

    showPasswdFrame();
}

void LockContent::pushUserFrame()
{
    qInfo() << "Push user frame";
    if(m_model->isServerModel())
        m_controlWidget->setUserSwitchEnable(false);

    m_userListWidget->updateLayout(width());
    setCenterContent(m_userListWidget);

    auto config = PluginConfigMap::instance().getConfig();
    if (config) {
        m_controlWidget->setUserSwitchEnable(config->isUserSwitchButtonVisiable());
    } else {
        m_controlWidget->setUserSwitchEnable(m_isUserSwitchVisible);
    }

    showDefaultFrame();
}

void LockContent::pushConfirmFrame()
{
    setCenterContent(m_authWidget, 0, Qt::AlignTop, calcTopSpacing(m_authWidget->getTopSpacing()));
    showDefaultFrame();
}

void LockContent::pushShutdownFrame()
{
    qInfo() << "Push shutdown frame";
    // 设置关机选项界面大小为中间区域的大小,并移动到左上角，避免显示后出现移动现象
    auto config = PluginConfigMap::instance().getConfig();
    if (config) {
        m_shutdownFrame->setUserSwitchEnable(config->isUserSwitchButtonVisiable());
        m_controlWidget->setUserSwitchEnable(config->isUserSwitchButtonVisiable());
    } else {
        m_shutdownFrame->setUserSwitchEnable(m_isUserSwitchVisible);
        m_controlWidget->setUserSwitchEnable(m_isUserSwitchVisible);
    }

    //设置关机选项界面大小为中间区域的大小,并移动到左上角，避免显示后出现移动现象
    m_shutdownFrame->onStatusChanged(m_model->currentModeState());
    setCenterContent(m_shutdownFrame);
    showDefaultFrame();
}

void LockContent::setMPRISEnable(const bool state)
{
    if (!m_mediaWidget) {
        m_mediaWidget = new MediaWidget;
        m_mediaWidget->setAccessibleName("MediaWidget");
        m_mediaWidget->initMediaPlayer();
    }

    m_mediaWidget->setVisible(state);
    setCenterBottomWidget(m_mediaWidget);
}

void LockContent::onNewConnection()
{
    // 重置密码界面显示前需要隐藏插件右键菜单，避免抢占键盘
    Q_EMIT m_model->hidePluginMenu();

    // 重置密码程序启动连接成功锁屏界面才释放键盘，避免点击重置密码过程中使用快捷键切走锁屏
    if (window()->windowHandle() && window()->windowHandle()->setKeyboardGrabEnabled(false)) {
        qDebug() << "setKeyboardGrabEnabled(false) success!";
    }

    if (m_localServer->hasPendingConnections()) {
        QLocalSocket *socket = m_localServer->nextPendingConnection();
        connect(socket, &QLocalSocket::disconnected, this, &LockContent::onDisConnect);
        connect(socket, &QLocalSocket::readyRead, this, [socket, this] {
            auto content = socket->readAll();
            if (content == "close") {
                if (m_authWidget) {
                    m_authWidget->syncPasswordResetPasswordVisibleChanged(QVariant::fromValue(true));
                    m_authWidget->syncResetPasswordUI();
                }
            }
        });
    }
}

void LockContent::onDisConnect()
{
    // 这种情况下不必强制要求可以抓取到键盘，因为可能是网络弹窗抓取了键盘
    tryGrabKeyboard(false);
}

void LockContent::onStatusChanged(SessionBaseModel::ModeStatus status)
{
    qInfo() << Q_FUNC_INFO << status << ", current status: " << m_currentModeStatus;
    refreshLayout(status);

    // select plugin config
    if (m_fmaWidget && m_fmaWidget->isPluginLoaded()) {
        PluginConfigMap::instance().requestAddConfig(PluginConfigMap::ConfigIndex::FullManagePlugin, m_fmaWidget);
    } else {
        PluginConfigMap::instance().requestRemoveConfig(PluginConfigMap::ConfigIndex::FullManagePlugin);
    }

    if(m_model->isServerModel())
        onUserListChanged(m_model->loginedUserList());

    if (m_currentModeStatus == status)
        return;
    m_currentModeStatus = status;

    switch (status) {
    case SessionBaseModel::ModeStatus::PasswordMode:
        pushPasswordFrame();
        break;
    case SessionBaseModel::ModeStatus::ConfirmPasswordMode:
        pushConfirmFrame();
        break;
    case SessionBaseModel::ModeStatus::UserMode:
        pushUserFrame();
        break;
    case SessionBaseModel::ModeStatus::PowerMode:
    case SessionBaseModel::ModeStatus::ShutDownMode:
        pushShutdownFrame();
        break;
    default:
        break;
    }

    m_model->setAbortConfirm(status == SessionBaseModel::ModeStatus::ConfirmPasswordMode);
    if (status != SessionBaseModel::ModeStatus::ConfirmPasswordMode)
        m_model->setPowerAction(SessionBaseModel::PowerAction::None);
}

void LockContent::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果是设置密码界面，不做处理
    if (m_model->currentModeState() == SessionBaseModel::ResetPasswdMode)
        return SessionBaseWindow::mouseReleaseEvent(event);

    //在关机界面没有点击按钮直接点击界面时，直接隐藏关机界面
    if (m_model->currentModeState() == SessionBaseModel::ShutDownMode) {
        m_model->setVisible(false);
    }

    if (m_model->currentModeState() == SessionBaseModel::UserMode
        || m_model->currentModeState() == SessionBaseModel::PowerMode) {
        // 触屏点击空白处不退出用户列表界面
        if (event->source() == Qt::MouseEventSynthesizedByQt)
            return SessionBaseWindow::mouseReleaseEvent(event);

        // 点击空白处的时候切换到当前用户，以开启验证。
        emit requestSwitchToUser(m_model->currentUser());
    }

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);

    SessionBaseWindow::mouseReleaseEvent(event);
}

void LockContent::showEvent(QShowEvent *event)
{
    onStatusChanged(m_model->currentModeState());
    // 重新设置一下焦点，否则用户列表界面切换屏幕的时候焦点异常
    if (m_centerWidget)
        m_centerWidget->setFocus();

    tryGrabKeyboard();
    QFrame::showEvent(event);
}

void LockContent::hideEvent(QHideEvent *event)
{
    m_shutdownFrame->recoveryLayout();
    QFrame::hideEvent(event);
}

void LockContent::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, [ = ] {
        if (m_virtualKB && m_virtualKB->isVisible()) {
            updateVirtualKBPosition();
        }
    });

    if (SessionBaseModel::PasswordMode == m_model->currentModeState() || (SessionBaseModel::ConfirmPasswordMode == m_model->currentModeState())) {
        m_centerSpacerItem->changeSize(0, calcTopSpacing(m_authWidget->getTopSpacing()));
        m_centerVLayout->invalidate();
    }

    SessionBaseWindow::resizeEvent(event);
}

void LockContent::restoreMode()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
}

void LockContent::updateGreeterBackgroundPath(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    if (m_model->currentModeState() != SessionBaseModel::ModeStatus::ShutDownMode) {
        emit requestBackground(path);
    }
}

void LockContent::updateDesktopBackgroundPath(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
        emit requestBackground(path);
    }
}

void LockContent::toggleVirtualKB()
{
    if (!m_virtualKB) {
        VirtualKBInstance::Instance();
        QTimer::singleShot(500, this, [ = ] {
            m_virtualKB = VirtualKBInstance::Instance().virtualKBWidget();
            qDebug() << "init virtualKB over." << m_virtualKB;
            toggleVirtualKB();
        });
        return;
    }

    m_virtualKB->setParent(this);
    m_virtualKB->raise();
    // m_userLoginInfo->getUserLoginWidget()->setPassWordEditFocus();

    updateVirtualKBPosition();
    m_virtualKB->setVisible(!m_virtualKB->isVisible());
}

void LockContent::showModule(const QString &name)
{
    PluginBase * plugin = PluginManager::instance()->findPlugin(name);
    if (!plugin) {
        return;
    }

    switch (plugin->type()) {
    case PluginBase::ModuleType::LoginType:
        m_loginWidget = plugin->content();
        setCenterContent(m_loginWidget);
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        break;
    case PluginBase::ModuleType::TrayType:
        m_loginWidget = plugin->content();
        setCenterContent(m_loginWidget);
        break;
    default:
        // 扩展插件类型 FullManagedLoginType，不在此处处理
        break;
    }
}

void LockContent::updateVirtualKBPosition()
{
    const QPoint point = mapToParent(QPoint((width() - m_virtualKB->width()) / 2, height() - m_virtualKB->height() - 50));
    m_virtualKB->move(point);
}

void LockContent::onUserListChanged(QList<std::shared_ptr<User> > list)
{
    const bool allowShowUserSwitchButton = m_model->allowShowUserSwitchButton();
    const bool alwaysShowUserSwitchButton = m_model->alwaysShowUserSwitchButton();
    bool haveLoginedUser = true;

    if (m_model->isServerModel() && m_model->appType() == AuthCommon::Login) {
        haveLoginedUser = !m_model->loginedUserList().isEmpty();
    }

    bool enable = (alwaysShowUserSwitchButton ||
                   (allowShowUserSwitchButton &&
                    (list.size() > (m_model->isServerModel() ? 0 : 1)))) &&
                  haveLoginedUser;

    m_controlWidget->setUserSwitchEnable(enable);
    m_shutdownFrame->setUserSwitchEnable(enable);

    m_isUserSwitchVisible = enable;
}

/**
 * @brief 抓取键盘,失败后继续抓取,最多尝试15次.
 *
 * @param exitIfFailed true: 发送失败通知，并隐藏锁屏。 false：不做任何处理。
 */
void LockContent::tryGrabKeyboard(bool exitIfFailed)
{
#ifndef QT_DEBUG
    if (!isVisible()) {
        return;
    }

    if (m_model->isUseWayland()) {
        static QDBusInterface *kwinInter = new QDBusInterface("org.kde.KWin","/KWin","org.kde.KWin", QDBusConnection::sessionBus());
        if (!kwinInter || !kwinInter->isValid()) {
            qWarning() << "kwinInter is invalid";
            m_failures = 0;
            return;
        }
        // wayland下判断是否有应用发起grab，如果有就不锁屏
        QDBusReply<bool> reply = kwinInter->call("xwaylandGrabed");
        if (!reply.isValid() || !reply.value()) {
            m_failures = 0;
            return;
        }
    } else {
        if (window()->windowHandle() && window()->windowHandle()->setKeyboardGrabEnabled(true)) {
            m_failures = 0;
            return;
        }
    }

    m_failures++;

    if (m_failures == 15) {
        qWarning() << "Trying to grab keyboard has exceeded the upper limit. dde-lock will quit.";

        m_failures = 0;

        if (!exitIfFailed) {
            return;
        }

        DDBusSender()
            .service("org.freedesktop.Notifications")
            .path("/org/freedesktop/Notifications")
            .interface("org.freedesktop.Notifications")
            .method(QString("Notify"))
            .arg(tr("Lock Screen"))
            .arg(static_cast<uint>(0))
            .arg(QString(""))
            .arg(QString(""))
            .arg(tr("Failed to lock screen"))
            .arg(QStringList())
            .arg(QVariantMap())
            .arg(5000)
            .call();

        DDBusSender()
            .service("com.deepin.daemon.ScreenSaver")
            .path("/org/freedesktop/ScreenSaver")
            .interface("org.freedesktop.ScreenSaver")
            .method(QString("SimulateUserActivity"))
            .call();

        qInfo() << "Request hide lock frame";
        emit requestLockFrameHide();
        return;
    }

    QTimer::singleShot(100, this, [this, exitIfFailed] {
        tryGrabKeyboard(exitIfFailed);
    });
#endif
}

void LockContent::currentWorkspaceChanged()
{
    QDBusPendingCall call = m_wmInter->GetCurrentWorkspaceBackgroundForMonitor(QGuiApplication::primaryScreen()->name());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [ = ] {
        if (!call.isError())
        {
            QDBusReply<QString> reply = call.reply();
            updateWallpaper(reply.value());
        } else
        {
            qWarning() << "get current workspace background error: " << call.error().message();
            updateWallpaper("/usr/share/backgrounds/deepin/desktop.jpg");
        }

        watcher->deleteLater();
    });
}

void LockContent::updateWallpaper(const QString &path)
{
    const QUrl url(path);
    QString wallpaper = path;
    if (url.isLocalFile()) {
        wallpaper = url.path();
    }

    updateDesktopBackgroundPath(wallpaper);
}

void LockContent::refreshBackground(SessionBaseModel::ModeStatus status)
{
    Q_UNUSED(status)

    // 根据当前状态刷新不同的背景
    auto user = m_model->currentUser();
    if (user != nullptr) {
        emit requestBackground(user->greeterBackground());
    }
}

void LockContent::refreshLayout(SessionBaseModel::ModeStatus status)
{
    setTopFrameVisible(status != SessionBaseModel::ModeStatus::ShutDownMode);
    setBottomFrameVisible(status != SessionBaseModel::ModeStatus::ShutDownMode);
}

void LockContent::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (m_mfaWidget) {
            m_mfaWidget->autoUnlock();
        }
        break;
    case Qt::Key_Escape:
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ConfirmPasswordMode) {
            m_model->setAbortConfirm(false);
            m_model->setPowerAction(SessionBaseModel::PowerAction::None);
        } else if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
            m_model->setVisible(false);
        } else if (m_model->currentModeState() == SessionBaseModel::ModeStatus::PowerMode) {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        }
        break;
    }
}

void LockContent::showUserList()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
    QTimer::singleShot(10, this, [ = ] {
        m_model->setVisible(true);
    });
}

void LockContent::showLockScreen()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    m_model->setVisible(true);
}

void LockContent::showShutdown()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
}
