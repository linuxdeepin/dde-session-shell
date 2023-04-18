// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include <QApplication>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QListView>
#include <QComboBox>
#include <QDebug>
#include <QProcess>

#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>

#include <DBlurEffectWidget>
#include <DConfig>

#include "lightergreeter.h"
#include "timewidget.h"
#include "dlineeditex.h"
#include "transparentbutton.h"
#include "useravatar.h"
#include "public_func.h"
#include "dconfig_helper.h"

#define AVATAR_ICON_SIZE UserAvatar::AvatarSmallSize
#define AVATAR_SIZE UserAvatar::AvatarLargeSize
#define SPACING 10

LighterGreeter::LighterGreeter(QWidget *parent)
    : QWidget(parent)
    , m_gridLayout(new QGridLayout(this))
    , m_loginFrame(new DBlurEffectWidget(this))
    , m_avatar(new UserAvatar(this))
    , m_userCbx(new QComboBox(this))
    , m_passwordEdit(new DLineEditEx(this))
    , m_sessionCbx(new QComboBox(this))
    , m_loginBtn(new TransparentButton(this))
    , m_switchGreeter(new QPushButton(tr("Switch To Normal"), this))
    , m_greeter(new QLightDM::Greeter(this))
    , m_sessionModel(new QLightDM::SessionsModel(this))
    , m_allowSwitchToWayland(DConfigHelper::instance()->getConfig("allowSwitchingToWayland", false).toBool())
    , m_defaultSessionName(DConfigHelper::instance()->getConfig("defaultSession", "deepin").toString())
    , m_respond(false)
{
    if (!m_greeter->connectSync()) {
        qWarning() << "connect sync failed";
        close();
    }

    initUI();
    initConnections();
}

void LighterGreeter::initUI()
{
    // 顶部区域
    m_gridLayout->addWidget(new TimeWidget(this), 0, 1, Qt::AlignCenter);

    // 中间区域
    QVBoxLayout *layout = new QVBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout(m_loginFrame);
    vLayout->setContentsMargins(SPACING, SPACING, SPACING, SPACING);
    vLayout->addSpacing(SPACING + (AVATAR_SIZE / 2));
    vLayout->addWidget(m_userCbx, 0, Qt::AlignCenter);
    vLayout->addWidget(m_passwordEdit, 0, Qt::AlignCenter);

    m_loginFrame->setMaskColor(DBlurEffectWidget::LightColor);
    m_loginFrame->setMaskAlpha(70);
    m_loginFrame->setBlurRectXRadius(15);
    m_loginFrame->setBlurRectYRadius(15);

    m_loginFrame->setMaximumSize(1152, 300);
    m_userCbx->setMinimumWidth(300);
    m_passwordEdit->setMinimumWidth(300);
    m_passwordEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_passwordEdit->setClearButtonEnabled(false);
    m_passwordEdit->setPlaceholderText(tr("Password"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);
    m_passwordEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp("^[ -~]+$")));

    m_loginBtn->setIcon(DStyle::SP_LockElement);

    auto lockPalette = m_loginBtn->palette();
    QColor color = lockPalette.color(QPalette::Active, QPalette::Highlight);
    lockPalette.setColor(QPalette::Disabled, QPalette::Highlight, color);
    m_loginBtn->setPalette(lockPalette);

    // 头像始终在LoginFrame区域的上部中间位置
    m_avatar->setFocusPolicy(Qt::NoFocus);
    m_avatar->setIcon("/var/lib/AccountsService/icons/guest.png");
    m_avatar->setAvatarSize(AVATAR_ICON_SIZE);

    layout->addWidget(m_loginFrame, 0, Qt::AlignCenter);
    layout->addWidget(m_loginBtn, 0, Qt::AlignCenter);

    m_gridLayout->addLayout(layout, 1, 1, Qt::AlignCenter);

    /* 整体布局，横向1:5:1,纵向1:1:1
    |---------------------------|
    |    |                  |   |
    |    |       Time       |   |
    |---------------------------|
    |    |                  |   |
    |    |      Login       |   |
    |---------------------------|
    |    |                  |   |
    |    |                  | s |
    |---------------------------|
    */

    m_gridLayout->setColumnStretch(0, 1);
    m_gridLayout->setColumnStretch(1, 5);
    m_gridLayout->setColumnStretch(2, 1);

    m_gridLayout->setRowStretch(0, 1);
    m_gridLayout->setRowStretch(1, 1);
    m_gridLayout->setRowStretch(2, 1);

    // 右下角区域
    DBlurEffectWidget *controlFrame = new DBlurEffectWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(controlFrame);
    hLayout->setContentsMargins(SPACING, SPACING, SPACING, SPACING);
    hLayout->addWidget(m_sessionCbx, 0, Qt::AlignCenter);
    hLayout->addWidget(m_switchGreeter, 0, Qt::AlignCenter);

    controlFrame->setMaskColor(DBlurEffectWidget::LightColor);
    controlFrame->setMaskAlpha(70);
    controlFrame->setBlurRectXRadius(15);
    controlFrame->setBlurRectYRadius(15);

    controlFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_gridLayout->addWidget(controlFrame, 2, 2, Qt::AlignBottom);

    m_userCbx->setModel(new QLightDM::UsersModel(this));

    for (int i = 0; i < m_sessionModel->rowCount(QModelIndex()); ++i) {
        const QString &sessionName = m_sessionModel->data(m_sessionModel->index(i), QLightDM::SessionsModel::IdRole).toString();
        QString displayName = (sessionName == "deepin") ? "X11" : sessionName;
        m_sessionCbx->addItem(displayName, sessionName);
    }

    if (!m_allowSwitchToWayland) {
        m_sessionCbx->removeItem(m_sessionCbx->findData("Wayland"));
    }

    if (!m_defaultSessionName.isEmpty()) {
        m_sessionCbx->setCurrentIndex(m_sessionCbx->findData(m_defaultSessionName));
    }

    m_sessionCbx->setVisible(m_sessionCbx->count() > 1);

    m_loginFrame->installEventFilter(this);
    installEventFilter(this);

    // Select last user
    const QString &user = DConfigHelper::instance()->getConfig("lastUser", QString()).toString();
    int index = m_userCbx->findText(user);
    if (index != -1)
        m_userCbx->setCurrentIndex(index);

    setFocusProxy(m_passwordEdit);
}

void LighterGreeter::initConnections()
{
    connect(m_passwordEdit, &DLineEditEx::returnPressed, this, &LighterGreeter::onRespond);
    connect(m_passwordEdit, &DLineEditEx::textChanged, this, [ = ] {
        m_passwordEdit->setAlert(false);
        m_passwordEdit->hideAlertMessage();
    });
    connect(m_userCbx, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LighterGreeter::onCurrentUserChanged);
    connect(m_loginBtn, &TransparentButton::clicked, this, &LighterGreeter::onRespond);

    connect(m_switchGreeter, &QPushButton::clicked, this, &LighterGreeter::resetToNormalGreeter);

    connect(m_greeter, &QLightDM::Greeter::authenticationComplete, this, &LighterGreeter::onAuthenticationComplete);
    connect(m_greeter, &QLightDM::Greeter::showPrompt, this, &LighterGreeter::onShowPrompt);
    connect(m_greeter, &QLightDM::Greeter::showMessage, this, &LighterGreeter::onShowMessage);

    onCurrentUserChanged();
}

void LighterGreeter::onRespond()
{
    // Respond to prompt with password text and disable window
    m_greeter->respond(m_passwordEdit->text());
    m_respond = true;

    m_userCbx->setEnabled(false);
    m_passwordEdit->setEnabled(false);
    m_loginBtn->setEnabled(false);
    m_sessionCbx->setEnabled(false);
    m_switchGreeter->setEnabled(false);

    // Start password field animation
    m_passwordEdit->startAnimation();
}

void LighterGreeter::onCurrentUserChanged()
{
    m_respond = false;
    m_passwordEdit->setVisible(true);
    disconnect(m_loginBtn);
    connect(m_loginBtn, &TransparentButton::clicked, this, &LighterGreeter::onRespond);

    // Clear password field
    m_passwordEdit->clear();

    // Cancel ongoing authentication
    if (m_greeter->inAuthentication()) {
        m_greeter->cancelAuthentication();
    }

    // Begin new authentication process with selected user
    int index = m_userCbx->currentIndex();
    const QString &userName = m_userCbx->itemData(index, QLightDM::UsersModel::NameRole).toString();
    qInfo() << "Authenticating user: " << userName;
    m_greeter->authenticate(userName);

    // Set focus to password field
    m_passwordEdit->setFocus();
}

void LighterGreeter::onAuthenticationComplete()
{
    qInfo() << "Authentication complete";
    if (m_greeter->isAuthenticated()) {
        auto startLogin = [this](const QString & session) {
            // Update cache with user information
            DConfigHelper::instance()->setConfig("lastUser", m_userCbx->currentText());

            // Hide current window and emit signal to start new session
            this->setVisible(false);
            Q_EMIT authFinished();

            // Set cursor to system default
            setRootWindowCursor();

            // Start session synchronization
            QMetaObject::invokeMethod(m_greeter, "startSessionSync", Qt::QueuedConnection
                                      , Q_ARG(QString, session));
        };

        qInfo() << "Passwordless login：" << m_respond;
        // Determine whether it is a passwordless login based on whether a password is required
        if (m_respond) {
            startLogin(m_sessionCbx->currentData().toString());
        } else {
            // Passwordless login
            m_passwordEdit->setVisible(false);
            disconnect(m_loginBtn);
            connect(m_loginBtn, &TransparentButton::clicked, this, [ = ] {
                startLogin(m_sessionCbx->currentData().toString());
            });
            m_loginBtn->setFocus();
        }
    } else {
        // Display error message for incorrect password and restart authentication process
        m_passwordEdit->showAlertMessage(tr("Incorrect password, please try again"));
        qInfo() << "Restarting authentication";
        onCurrentUserChanged();
    }
}

void LighterGreeter::onShowPrompt(QString promptText, int promptType)
{
    qInfo() << "Showing prompt: " <<  promptText << promptType;

    // Stop password field animation and enable window
    m_passwordEdit->stopAnimation();

    m_userCbx->setEnabled(true);
    m_passwordEdit->setEnabled(true);
    m_loginBtn->setEnabled(true);
    m_sessionCbx->setEnabled(true);
    m_switchGreeter->setEnabled(true);

    // Determine type of prompt being displayed
    auto type = static_cast<QLightDM::Greeter::PromptType>(promptType);
    switch (type) {
    case QLightDM::Greeter::PromptType::PromptTypeSecret:
        // Focus on password field for secret prompts
        m_passwordEdit->setFocus();
        break;
    case QLightDM::Greeter::PromptType::PromptTypeQuestion:
        // Clear password field and set placeholder text for question prompts
        m_passwordEdit->clear();
        m_passwordEdit->setPlaceholderText(promptText);
        break;
    }
}

void LighterGreeter::onShowMessage(QString messageText, int messageType)
{
    qInfo() << "Showing message: " <<  messageText << messageType;

    // Stop password field animation and enable window
    m_passwordEdit->stopAnimation();

    m_userCbx->setEnabled(true);
    m_passwordEdit->setEnabled(true);
    m_loginBtn->setEnabled(true);
    m_sessionCbx->setEnabled(true);
    m_switchGreeter->setEnabled(true);

    // Determine type of message being displayed
    auto type = static_cast<QLightDM::Greeter::MessageType>(messageType);
    switch (type) {
    case QLightDM::Greeter::MessageType::MessageTypeInfo:
        // Do nothing for info messages
        break;
    case QLightDM::Greeter::MessageType::MessageTypeError:
        // Clear password field, set alert flag, and show error message for error messages
        m_passwordEdit->clear();
        m_passwordEdit->setAlert(true);
        m_passwordEdit->showAlertMessage(messageText);
        break;
    }
}

void LighterGreeter::resetToNormalGreeter()
{
    // If it exits abnormally, try to pull up the normal Greeter, this part is handled in the script
    qApp->exit(-1);
}

void LighterGreeter::showEvent(QShowEvent *event)
{
    m_passwordEdit->setFocus();
    m_passwordEdit->lineEdit()->setFocus();

    return QWidget::showEvent(event);
}

bool LighterGreeter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::Show) {
        // Set focus to password field when window is displayed
        if (!qgetenv("XDG_SESSION_TYPE").startsWith("wayland"))
            activateWindow();

        m_passwordEdit->setFocus();
        m_passwordEdit->lineEdit()->setFocus();
    } else if ((watched == this || watched == m_loginFrame) && event->type() == QEvent::Resize) {
        int x = m_loginFrame->mapToGlobal(QPoint(0, 0)).x() + ((m_loginFrame->width() - AVATAR_SIZE) / 2);
        int y = m_loginFrame->mapToGlobal(QPoint(0, 0)).y() - (AVATAR_SIZE / 2);

        m_avatar->move(mapFromGlobal(QPoint(x, y)));
    }

    return QWidget::eventFilter(watched, event);
}
