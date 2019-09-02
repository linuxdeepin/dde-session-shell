#include "lockcontent.h"

#include "src/widgets/controlwidget.h"
#include "userinputwidget.h"
#include "sessionbasemodel.h"
#include "userframe.h"
#include "src/widgets/shutdownwidget.h"
#include "src/widgets/virtualkbinstance.h"
#include "src/widgets/logowidget.h"
#include "src/widgets/timewidget.h"
#include "userlogininfo.h"
#include "userloginwidget.h"
#include "userframelist.h"

#include <QMouseEvent>

LockContent::LockContent(SessionBaseModel * const model, QWidget *parent)
    : SessionBaseWindow(parent)
    , m_model(model)
    , m_imageBlurInter(new ImageBlur("com.deepin.daemon.Accounts", "/com/deepin/daemon/ImageBlur", QDBusConnection::systemBus(), this))
    , m_virtualKB(nullptr)
    , m_translator(new QTranslator)
    , m_userLoginInfo(new UserLoginInfo(model))
{
    m_controlWidget = new ControlWidget;
    m_userInputWidget = new UserInputWidget;
    m_userFrame = new UserFrame;
    m_shutdownFrame = new ShutdownWidget;
    m_logoWidget = new LogoWidget;
    m_timeWidget = new TimeWidget;
    m_mediaWidget = nullptr;

    m_userInputWidget->updateAuthType(model->currentType());

    m_shutdownFrame->setModel(model);
    m_userFrame->setModel(model);

    model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);

    setRightBottomWidget(m_controlWidget);
    setCenterTopWidget(m_timeWidget);
    setLeftBottomWidget(m_logoWidget);
    connect(model, &SessionBaseModel::currentUserChanged, this, [ = ](std::shared_ptr<User> user) {
        if (user.get()) {
            m_logoWidget->updateLocale(user->locale().split(".").first());
        }
    });

    switch (model->currentType()) {
    case SessionBaseModel::AuthType::LockType:
        setMPRISEnable(true);
        break;
    default:
        break;
    }

    // init connect
    connect(model, &SessionBaseModel::currentUserChanged, this, &LockContent::onCurrentUserChanged);
    connect(m_userInputWidget, &UserInputWidget::requestAuthUser, this, &LockContent::requestAuthUser);
    connect(m_userFrame, &UserFrame::requestSwitchUser, this, &LockContent::requestSwitchToUser);
    connect(m_userFrame, &UserFrame::requestSwitchUser, this, &LockContent::restoreMode);
    connect(m_controlWidget, &ControlWidget::requestSwitchUser, this, [=] {
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::UserMode) return;
        m_userFrame->setFixedSize(m_userInputWidget->size());
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
    });
    connect(m_controlWidget, &ControlWidget::requestShutdown, this, [=] {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PowerMode);
    });
    connect(m_controlWidget, &ControlWidget::requestSwitchVirtualKB, this, &LockContent::toggleVirtualKB);
    connect(m_userFrame, &UserFrame::hideFrame, this, &LockContent::restoreMode);

    //lixin
    connect(m_userLoginInfo, &UserLoginInfo::requestAuthUser, this, &LockContent::requestAuthUser);
    connect(m_userLoginInfo, &UserLoginInfo::hideUserFrameList, this, &LockContent::restoreMode);
    connect(m_userLoginInfo, &UserLoginInfo::requestSwitchUser, this, &LockContent::requestSwitchToUser);
    connect(m_userLoginInfo, &UserLoginInfo::requestSwitchUser, this, &LockContent::restoreMode);
    connect(m_userLoginInfo, &UserLoginInfo::requestSetLayout, this, &LockContent::requestSetLayout);

    connect(m_shutdownFrame, &ShutdownWidget::abortOperation, this, &LockContent::restoreMode);

    connect(model, &SessionBaseModel::authFinished, this, [=] (bool success) {
        if (success) {
            m_userInputWidget->resetAllState();
        }
    });
    connect(model, &SessionBaseModel::authFaildMessage, m_userInputWidget, &UserInputWidget::setFaildMessage);
    connect(model, &SessionBaseModel::authFaildTipsMessage, m_userInputWidget, &UserInputWidget::setFaildTipMessage);
    connect(model, &SessionBaseModel::onStatusChanged, this, &LockContent::onStatusChanged);
    connect(model, &SessionBaseModel::onPowerActionChanged, this, [=] (SessionBaseModel::PowerAction poweraction) {
        switch (poweraction) {
        case SessionBaseModel::RequireNormal:
            m_userInputWidget->normalMode();
            break;
        case SessionBaseModel::RequireRestart:
            if (model->currentType() == SessionBaseModel::AuthType::LockType) {
                m_userInputWidget->restartMode();
            }
            break;
        case SessionBaseModel::RequireShutdown:
            if (model->currentType() == SessionBaseModel::AuthType::LockType) {
                m_userInputWidget->shutdownMode();
            }
        default:
            break;
        }
    });

    connect(m_userInputWidget, &UserInputWidget::abortOperation, this, [=] {
        model->setPowerAction(SessionBaseModel::PowerAction::RequireNormal);
    });

    connect(m_userInputWidget, &UserInputWidget::requestUserKBLayoutChanged, this, [=] (const QString &value) {
        emit requestSetLayout(m_user, value);
    });

    auto initVirtualKB = [&] (bool hasvirtualkb) {
        if (hasvirtualkb && !m_virtualKB) {
            connect(&VirtualKBInstance::Instance(), &VirtualKBInstance::initFinished, this, [&] {
                m_virtualKB = VirtualKBInstance::Instance().virtualKBWidget();
                m_controlWidget->setVirtualKBVisible(true);
            }, Qt::QueuedConnection);
            VirtualKBInstance::Instance().init();
        }
    };

    connect(model, &SessionBaseModel::hasVirtualKBChanged, this, initVirtualKB);
    connect(model, &SessionBaseModel::onUserListChanged, this, &LockContent::onUserListChanged);
    connect(m_imageBlurInter, &ImageBlur::BlurDone, this, &LockContent::onBlurDone);

    onCurrentUserChanged(model->currentUser());
    initVirtualKB(model->hasVirtualKB());

    onUserListChanged(model->userList());
}

void LockContent::onCurrentUserChanged(std::shared_ptr<User> user)
{
    if (user.get() == nullptr) return; // if dbus is async

    // set user language
    qApp->removeTranslator(m_translator);
    const QString locale { user->locale().split(".").first() };
    m_translator->load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale(locale.isEmpty() ? "en_US" : locale).name());
    qApp->installTranslator(m_translator);

    for (auto connect : m_currentUserConnects) {
        m_user.get()->disconnect(connect);
    }

    m_currentUserConnects.clear();

    m_user = user;

    m_currentUserConnects << connect(user.get(), &User::greeterBackgroundPathChanged, this, &LockContent::updateBackground , Qt::UniqueConnection);

    m_userInputWidget->setUser(user);

    //lixin
    m_userLoginInfo->setUser(user);

    //TODO: refresh blur image
    QTimer::singleShot(0, this, [=] {
        updateBackground(user->greeterBackgroundPath());
    });
}

void LockContent::pushUserFrame()
{
    releaseAllKeyboard();
//    setCenterContent(m_userFrame);
//    m_userFrame->expansion(true);

//    QTimer::singleShot(300, m_userFrame, &UserFrame::grabKeyboard);

    setCenterContent(m_userLoginInfo->getUserFrameList());
    QTimer::singleShot(300, m_userLoginInfo->getUserFrameList(), &UserFrameList::grabKeyboard);
}

void LockContent::pushShutdownFrame()
{
    releaseAllKeyboard();
    setCenterContent(m_shutdownFrame);

    QTimer::singleShot(300, m_shutdownFrame, &ShutdownWidget::grabKeyboard);
}

void LockContent::setMPRISEnable(const bool state)
{
    if (m_mediaWidget) {
        m_mediaWidget->setVisible(state);
    } else {
        m_mediaWidget = new MediaWidget;
        m_mediaWidget->initMediaPlayer();
        setCenterBottomWidget(m_mediaWidget);
    }
}

void LockContent::onStatusChanged(SessionBaseModel::ModeStatus status)
{
    switch (status) {
    case SessionBaseModel::ModeStatus::PasswordMode:
        restoreCenterContent();
        break;
    case SessionBaseModel::ModeStatus::UserMode:
        pushUserFrame();
        break;
    case SessionBaseModel::ModeStatus::PowerMode:
        pushShutdownFrame();
        break;
    default:
        break;
    }
}

void LockContent::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_model->currentModeState() != SessionBaseModel::ModeStatus::PasswordMode) {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        m_model->setPowerAction(SessionBaseModel::PowerAction::RequireNormal);
    }

    // hide keyboardlayout widget
    m_userInputWidget->hideKeyboard();
    m_userLoginInfo->hideKBLayout();

    restoreCenterContent();

    return SessionBaseWindow::mouseReleaseEvent(event);
}

void LockContent::showEvent(QShowEvent *event)
{
    onStatusChanged(m_model->currentModeState());

    return QFrame::showEvent(event);
}

void LockContent::hideEvent(QHideEvent *event)
{
    releaseAllKeyboard();

    return QFrame::hideEvent(event);
}

void LockContent::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, [=] {
        if (m_virtualKB && m_virtualKB->isVisible()) {
            updateVirtualKBPosition();
        }
    });

    return QFrame::resizeEvent(event);
}

void LockContent::releaseAllKeyboard()
{
    m_userInputWidget->releaseKeyboard();
    m_userFrame->releaseKeyboard();
    m_userLoginInfo->getUserLoginWidget()->releaseKeyboard();
    m_userLoginInfo->getUserFrameList()->releaseKeyboard();
    m_shutdownFrame->releaseKeyboard();
}

void LockContent::restoreCenterContent()
{
    releaseAllKeyboard();

    setCenterContent(m_userLoginInfo->getUserLoginWidget());
    QTimer::singleShot(300, m_userLoginInfo->getUserLoginWidget(), &UserLoginWidget::grabKeyboard);
}

void LockContent::restoreMode()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
}

void LockContent::updateBackground(const QString &path)
{
//    const QString &wallpaper = m_imageBlurInter->Get(path);

//    emit requestBackground(wallpaper.isEmpty() ? path : wallpaper);
    emit requestBackground(path);
}

void LockContent::onBlurDone(const QString &source, const QString &blur, bool status)
{
    const QString &sourcePath = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;

    if (status && m_model->currentUser()->greeterBackgroundPath() == sourcePath)
        emit requestBackground(blur);
}

void LockContent::toggleVirtualKB()
{
    if (!m_virtualKB) {
        VirtualKBInstance::Instance();
        QTimer::singleShot(500, this, [=] {
            m_virtualKB = VirtualKBInstance::Instance().virtualKBWidget();
            qDebug() << "init virtualKB over." << m_virtualKB;
            toggleVirtualKB();
        });
        return;
    }

    m_virtualKB->setParent(this);
    m_virtualKB->setVisible(!m_virtualKB->isVisible());
    m_virtualKB->raise();

    updateVirtualKBPosition();
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

    m_controlWidget->setUserSwitchEnable(alwaysShowUserSwitchButton || (allowShowUserSwitchButton && list.size() > 1));
}
