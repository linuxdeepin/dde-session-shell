/*
* Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     Zhang Qipeng <zhangqipeng@uniontech.com>
*
* Maintainer: Zhang Qipeng <zhangqipeng@uniontech.com>
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

#include "auth_widget.h"

#include "auth_face.h"
#include "auth_fingerprint.h"
#include "auth_iris.h"
#include "auth_password.h"
#include "auth_single.h"
#include "auth_ukey.h"
#include "dlineeditex.h"
#include "kblayoutwidget.h"
#include "framedatabind.h"
#include "keyboardmonitor.h"
#include "sessionbasemodel.h"
#include "useravatar.h"

#include <DFontSizeManager>

const int BlurRadius = 15;
const int BlurTransparency = 70;

const QColor ShutdownColor(QColor(247, 68, 68));

AuthWidget::AuthWidget(QWidget *parent)
    : QWidget(parent)
    , m_frameDataBind(FrameDataBind::Instance())
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_lockButton(nullptr)
    , m_userAvatar(nullptr)
    , m_expiredStatusLabel(new DLabel(this))
    , m_nameLabel(nullptr)
    , m_accountEdit(nullptr)
    , m_capslockMonitor(nullptr)
    , m_keyboardTypeWidget(nullptr)
    , m_keyboardTypeBorder(nullptr)
    , m_keyboardTypeClip(nullptr)
    , m_singleAuth(nullptr)
    , m_passwordAuth(nullptr)
    , m_fingerprintAuth(nullptr)
    , m_ukeyAuth(nullptr)
    , m_faceAuth(nullptr)
    , m_irisAuth(nullptr)
{
    setObjectName(QStringLiteral("AuthWidget"));
    setAccessibleName(QStringLiteral("AuthWidget"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::NoFocus);

    m_capslockMonitor = KeyboardMonitor::instance();
    m_capslockMonitor->start(QThread::LowestPriority);
    m_frameDataBind = FrameDataBind::Instance();
}

AuthWidget::~AuthWidget()
{
    for (auto it = m_registerFunctions.constBegin(); it != m_registerFunctions.constEnd(); ++it) {
        m_frameDataBind->unRegisterFunction(it.key(), it.value());
    }
}

void AuthWidget::initUI()
{
    /* 头像 */
    m_userAvatar = new UserAvatar(this);
    m_userAvatar->setFocusPolicy(Qt::NoFocus);
    m_userAvatar->setIcon(m_user->avatar());
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);
    /* 用户名 */
    m_nameLabel = new DLabel(this);
    m_nameLabel->setAccessibleName(QStringLiteral("NameLabel"));
    m_nameLabel->setTextFormat(Qt::TextFormat::PlainText);
    m_nameLabel->setText(m_model->currentUser()->displayName());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setTextFormat(Qt::TextFormat::PlainText);
    DFontSizeManager::instance()->bind(m_nameLabel, DFontSizeManager::T2);
    QPalette palette = m_nameLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLabel->setPalette(palette);
    /* 用户名输入框 */
    m_accountEdit = new DLineEditEx(this);
    m_accountEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_accountEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_accountEdit->setClearButtonEnabled(false);
    m_accountEdit->setPlaceholderText(tr("Account"));
    /* 用户名和用户名输入框的可见性 */
    if (m_model->appType() == AppTypeLogin && m_model->isServerModel()) {
        m_accountEdit->show();
        m_nameLabel->hide();
    } else {
        m_accountEdit->hide();
        m_nameLabel->show();
    }
    /* 密码过期提示 */
    m_expiredStatusLabel->setAccessibleName("ExpiredStatusLabel");
    m_expiredStatusLabel->setWordWrap(true);
    m_expiredStatusLabel->setAlignment(Qt::AlignHCenter);
    m_expiredStatusLabel->hide();
    /* 解锁按钮 */
    m_lockButton = new DFloatingButton(this);
    if (m_model->appType() == AppTypeLock) {
        m_lockButton->setIcon(DStyle::SP_LockElement);
    } else {
        m_lockButton->setIcon(DStyle::SP_ArrowNext);
    }
    /* 模糊背景 */
    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurEffectWidget->setMaskAlpha(BlurTransparency);
    m_blurEffectWidget->setBlurRectXRadius(BlurRadius);
    m_blurEffectWidget->setBlurRectYRadius(BlurRadius);
    /* 键盘布局选择菜单 */
    m_keyboardTypeWidget = new KbLayoutWidget(QStringList(), this);
    m_keyboardTypeWidget->setAccessibleName(QStringLiteral("KbLayoutWidget"));
    m_keyboardTypeWidget->updateButtonList(m_user->keyboardLayoutList());
    m_keyboardTypeBorder = new DArrowRectangle(DArrowRectangle::ArrowTop, this);
    m_keyboardTypeBorder->setAccessibleName(QStringLiteral("KeyboardTypeBorder"));
    m_keyboardTypeBorder->setContentsMargins(0, 0, 0, 0);
    m_keyboardTypeBorder->setBorderWidth(0);
    m_keyboardTypeBorder->setContent(m_keyboardTypeWidget);
    m_keyboardTypeBorder->setRadius(8);
}

void AuthWidget::initConnections()
{
    /* model */
    connect(m_model, &SessionBaseModel::authFaildTipsMessage, this, &AuthWidget::setAccountErrorMsg);
    connect(m_model, &SessionBaseModel::currentUserChanged, this, &AuthWidget::setUser);
    connect(m_model, &SessionBaseModel::onPowerActionChanged, this, [this](SessionBaseModel::PowerAction action) {
        setLockButtonType(action);
    });
    /* 用户名 */
    connect(qGuiApp, &QGuiApplication::fontChanged, this, &AuthWidget::setNameFont);
    /* 用户名输入框 */
    connect(m_accountEdit, &DLineEditEx::returnPressed, this, [this] {
        if (m_accountEdit->isVisible() && !m_accountEdit->text().isEmpty()) {
            emit requestCheckAccount(m_accountEdit->text());
        }
    });
    connect(m_accountEdit, &DLineEditEx::editingFinished, this, [this] {
        emit m_accountEdit->returnPressed();
    });
    /* 解锁按钮 */
    connect(m_lockButton, &DFloatingButton::clicked, this, [this] {
        if (m_user->isNoPasswordLogin()) {
            emit requestCheckAccount(m_user->name());
        } else if (!m_passwordAuth && !m_ukeyAuth && !m_singleAuth) {
            emit m_accountEdit->returnPressed();
        }
    });
}

void AuthWidget::setModel(const SessionBaseModel *model)
{
    m_model = model;
    m_user = model->currentUser();
}

/**
 * @brief 设置当前用户
 * @param user
 */
void AuthWidget::setUser(std::shared_ptr<User> user)
{
    for (const QMetaObject::Connection &connection : qAsConst(m_connectionList)) {
        m_user->disconnect(connection);
    }
    m_connectionList.clear();
    m_user = user;
    m_connectionList << connect(user.get(), &User::avatarChanged, this, &AuthWidget::setAvatar)
                     << connect(user.get(), &User::displayNameChanged, this, &AuthWidget::setName)
                     << connect(user.get(), &User::passwordHintChanged, this, &AuthWidget::setPasswordHint)
                     << connect(user.get(), &User::keyboardLayoutChanged, this, &AuthWidget::setKeyboardType)
                     << connect(user.get(), &User::keyboardLayoutListChanged, this, &AuthWidget::setKeyboardTypeList)
                     << connect(user.get(), &User::limitsInfoChanged, this, &AuthWidget::setLimitsInfo)
                     << connect(m_keyboardTypeWidget, &KbLayoutWidget::setButtonClicked, user.get(), &User::setKeyboardLayout);

    setAvatar(user->avatar());
    setName(user->displayName());
    setPasswordHint(user->passwordHint());
    setKeyboardType(user->keyboardLayout());
    setKeyboardTypeList(user->keyboardLayoutList());
    setLimitsInfo(user->limitsInfo());
    updateExpiredStatus();
}

/**
 * @brief 设置认证类型
 * @param type
 */
void AuthWidget::setAuthType(const int type)
{
    Q_UNUSED(type)
}

/**
 * @brief 设置认证状态
 * @param type
 * @param status
 * @param message
 */
void AuthWidget::setAuthStatus(const int type, const int status, const QString &message)
{
    Q_UNUSED(type)
    Q_UNUSED(status)
    Q_UNUSED(message)
}

/**
 * @brief AuthWidget::checkAuthResult
 * @param type
 * @param state
 */
void AuthWidget::checkAuthResult(const int type, const int status)
{
    Q_UNUSED(type)
    Q_UNUSED(status)
}

/**
 * @brief 设置认证限制信息
 * @param limitsInfo
 */
void AuthWidget::setLimitsInfo(const QMap<int, User::LimitsInfo> *limitsInfo)
{
    User::LimitsInfo limitsInfoTmpU;
    LimitsInfo limitsInfoTmp;

    QMap<int, User::LimitsInfo>::const_iterator i = limitsInfo->constBegin();
    while (i != limitsInfo->end()) {
        limitsInfoTmpU = i.value();
        limitsInfoTmp.locked = limitsInfoTmpU.locked;
        limitsInfoTmp.maxTries = limitsInfoTmpU.maxTries;
        limitsInfoTmp.numFailures = limitsInfoTmpU.numFailures;
        limitsInfoTmp.unlockSecs = limitsInfoTmpU.unlockSecs;
        limitsInfoTmp.unlockTime = limitsInfoTmpU.unlockTime;
        switch (i.key()) {
        case AuthTypeSingle:
            if (m_singleAuth) {
                m_singleAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AuthTypePassword:
            if (m_passwordAuth) {
                m_passwordAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AuthTypeFingerprint:
            if (m_fingerprintAuth) {
                m_fingerprintAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AuthTypeUkey:
            if (m_ukeyAuth) {
                m_ukeyAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AuthTypeFace:
            if (m_faceAuth) {
                m_faceAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AuthTypeIris:
            if (m_irisAuth) {
                m_irisAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AuthTypeActiveDirectory:
            break;
        default:
            qWarning() << "Error! Authentication type is wrong." << i.key();
            break;
        }
        ++i;
    }
}

/**
 * @brief 设置用户头像
 * @param avatar
 */
void AuthWidget::setAvatar(const QString &avatar)
{
    m_userAvatar->setIcon(avatar);
}

/**
 * @brief 设置用户名
 * @param name
 */
void AuthWidget::setName(const QString &name)
{
    m_nameLabel->setText(name);
}

/**
 * @brief 设置用户名字体
 * @param font
 */
void AuthWidget::setNameFont(const QFont &font)
{
    const QString &name = m_user->name();
    if (font != m_nameLabel->font()) {
        m_nameLabel->setFont(font);
    }
    int nameWidth = m_nameLabel->fontMetrics().width(name);
    int labelMaxWidth = width() - 10 * 2;

    if (nameWidth > labelMaxWidth) {
        QString str = m_nameLabel->fontMetrics().elidedText(name, Qt::ElideRight, labelMaxWidth);
        m_nameLabel->setText(str);
    } else {
        m_nameLabel->setText(name);
    }
}

/**
 * @brief 设置密码提示
 * @param hint
 */
void AuthWidget::setPasswordHint(const QString &hint)
{
    if (m_singleAuth) {
        m_singleAuth->setPasswordHint(hint);
    }
    if (m_passwordAuth) {
        m_passwordAuth->setPasswordHint(hint);
    }
}

/**
 * @brief 设置键盘布局类型
 * @param type
 */
void AuthWidget::setKeyboardType(const QString &type)
{
    m_keyboardTypeWidget->setDefault(type);

    m_keyboardTypeBorder->hide();

    if (m_singleAuth) {
        m_singleAuth->setKeyboardButtonInfo(type);
    }
    if (m_passwordAuth) {
        m_passwordAuth->setKeyboardButtonInfo(type);
    }
}

/**
 * @brief 设置键盘布局列表
 * @param list
 */
void AuthWidget::setKeyboardTypeList(const QStringList &list)
{
    m_keyboardTypeWidget->updateButtonList(list);

    const bool visible = list.size() > 1;
    if (m_singleAuth) {
        m_singleAuth->setKeyboardButtonVisible(visible);
    }
    if (m_passwordAuth) {
        m_passwordAuth->setKeyboardButtonVisible(visible);
    }
}

/**
 * @brief 设置解锁按钮的样式
 * @param type
 */
void AuthWidget::setLockButtonType(const int type)
{
    QPalette lockPalatte = m_lockButton->palette();
    switch (type) {
    case SessionBaseModel::RequireRestart:
        m_lockButton->setIcon(QIcon(":/img/bottom_actions/reboot.svg"));
        lockPalatte.setColor(QPalette::Highlight, ShutdownColor);
        m_lockButton->setPalette(lockPalatte);
        break;
    case SessionBaseModel::RequireShutdown:
        m_lockButton->setIcon(QIcon(":/img/bottom_actions/shutdown.svg"));
        lockPalatte.setColor(QPalette::Highlight, ShutdownColor);
        m_lockButton->setPalette(lockPalatte);
        break;
    default:
        if (m_model->appType() == AppTypeLogin) {
            m_lockButton->setIcon(DStyle::SP_ArrowNext);
        } else {
            m_lockButton->setIcon(DStyle::SP_LockElement);
        }
        break;
    }
}

/**
 * @brief 设置账户错误信息
 * @param message
 */
void AuthWidget::setAccountErrorMsg(const QString &message)
{
    m_accountEdit->showAlertMessage(message);
}

/**
 * @brief 更新模糊背景参数
 */
void AuthWidget::updateBlurEffectGeometry()
{
    QRect rect;
    rect.setLeft(0);
    rect.setRight(this->geometry().width());
    rect.setTop(m_userAvatar->geometry().top() + m_userAvatar->height() / 2);
    rect.setBottom(m_lockButton->geometry().top() - 10);
    m_blurEffectWidget->setGeometry(rect);
}

/**
 * @brief 更新键盘布局列表菜单参数
 */
void AuthWidget::updateKeyboardTypeListGeometry()
{
    if (m_passwordAuth) {
        const QPoint &point = mapToGlobal(QPoint(m_blurEffectWidget->geometry().left() + 30, m_blurEffectWidget->geometry().bottom() - 10));
        m_keyboardTypeBorder->move(point.x(), point.y());
    }
}

/**
 * @brief 密码过期提示
 */
void AuthWidget::updateExpiredStatus()
{
    switch (m_user->expiredStatus()) {
    case User::ExpiredNormal:
        m_expiredStatusLabel->clear();
        m_expiredStatusLabel->hide();
        break;
    case User::ExpiredSoon:
        m_expiredStatusLabel->setText(tr("Your password will expire in %n days, please change it timely", "", m_user->expiredDayLeft()));
        m_expiredStatusLabel->show();
        break;
    case User::ExpiredAlready:
        m_expiredStatusLabel->setText(tr("Password expired, please change"));
        m_expiredStatusLabel->show();
        break;
    default:
        break;
    }
}

/**
 * @brief 显示键盘布局菜单
 */
void AuthWidget::showKeyboardList()
{
    if (m_keyboardTypeBorder->isVisible()) {
        m_keyboardTypeBorder->hide();
    } else {
        m_keyboardTypeBorder->setVisible(true);
    }
}

/**
 * @brief 将同步数据的方法注册到单例类
 * @param flag
 * @param function
 */
void AuthWidget::registerSyncFunctions(const QString &flag, std::function<void (QVariant)> function)
{
    m_registerFunctions[flag] = m_frameDataBind->registerFunction(flag, function);
    m_frameDataBind->refreshData(flag);
}

/**
 * @brief 单因数据同步
 * @param value
 */
void AuthWidget::syncSingle(const QVariant &value)
{
    if (m_singleAuth)
        m_singleAuth->setLineEditInfo(value.toString(), AuthPassword::InputText);
}

/**
 * @brief 单因重置密码可见性数据同步
 *
 * @param value
 */
void AuthWidget::syncSingleResetPasswordVisibleChanged(const QVariant &value)
{
    if (m_singleAuth)
        m_singleAuth->setResetPasswordMessageVisible(value.toBool());
}

/**
 * @brief 用户名编辑框数据同步
 * @param value
 */
void AuthWidget::syncAccount(const QVariant &value)
{
    int cursorIndex = m_accountEdit->lineEdit()->cursorPosition();
    m_accountEdit->setText(value.toString());
    m_accountEdit->lineEdit()->setCursorPosition(cursorIndex);
}

/**
 * @brief 密码编辑框数据同步
 * @param value
 */
void AuthWidget::syncPassword(const QVariant &value)
{
    if (m_passwordAuth)
        m_passwordAuth->setLineEditInfo(value.toString(), AuthPassword::InputText);
}

/**
 * @brief 密码重置密码可见性数据同步
 *
 * @param value
 */
void AuthWidget::syncPasswordResetPasswordVisibleChanged(const QVariant &value)
{
    if (m_passwordAuth)
        m_passwordAuth->setResetPasswordMessageVisible(value.toBool());
}

/**
 * @brief UKey 编辑框数据同步
 * @param value
 */
void AuthWidget::syncUKey(const QVariant &value)
{
    if (m_passwordAuth)
        m_ukeyAuth->setLineEditInfo(value.toString(), AuthUKey::InputText);
}

void AuthWidget::hideEvent(QHideEvent *event)
{
    m_keyboardTypeBorder->hide();
    QWidget::hideEvent(event);
}

void AuthWidget::resizeEvent(QResizeEvent *event)
{
    updateBlurEffectGeometry();
    updateKeyboardTypeListGeometry();
    QWidget::resizeEvent(event);
}

void AuthWidget::showEvent(QShowEvent *event)
{
    activateWindow();
    QWidget::showEvent(event);
}

void AuthWidget::reset()
{
    if (m_singleAuth)
        m_singleAuth->reset();
    if (m_passwordAuth)
        m_passwordAuth->reset();
    if (m_fingerprintAuth)
        m_fingerprintAuth->reset();
    if (m_ukeyAuth)
        m_ukeyAuth->reset();
    if (m_faceAuth)
        m_faceAuth->reset();
    if (m_irisAuth)
        m_irisAuth->reset();
}
