/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lixin <lixin_cm@deepin.com>
 *
 * Maintainer: lixin <lixin_cm@deepin.com>
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

#include "userloginwidget.h"
#include "lockpasswordwidget.h"
#include "framedatabind.h"
#include "userinfo.h"
#include "src/widgets/useravatar.h"
#include "src/widgets/loginbutton.h"
#include "src/global_util/constants.h"
#include "src/widgets/otheruserinput.h"

#include <DFontSizeManager>
#include <DPalette>

#include <QVBoxLayout>

static const int BlurRectRadius = 15;
static const int WidgetsSpacing = 10;

UserLoginWidget::UserLoginWidget(QWidget *parent)
    : QWidget(parent)
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_userAvatar(new UserAvatar(this))
    , m_nameLbl(new QLabel(this))
    , m_passwordEdit(new DPasswdEditAnimated(this))
    , m_lockPasswordWidget(new LockPasswordWidget)
    , m_otherUserInput(new OtherUserInput(this))
    , m_lockButton(new DFloatingButton(tr("Login")))
    , m_showType(NormalType)
    , m_isLock(false)
    , m_isLogin(false)
{
    initUI();
    initConnect();
}

//重置控件的状态
void UserLoginWidget::resetAllState()
{
    m_passwordEdit->lineEdit()->clear();
    m_passwordEdit->abortAuth();
}

void UserLoginWidget::grabKeyboard()
{
    if (m_passwordEdit->isVisible()) {
        m_passwordEdit->lineEdit()->grabKeyboard();
        return;
    }

    if (m_otherUserInput->isVisible()) {
        m_passwordEdit->lineEdit()->releaseKeyboard();
        m_otherUserInput->grabKeyboard();
        return;
    }

    if (m_lockButton->isVisible()) {
        m_lockButton->grabKeyboard();
        m_lockButton->setFocus();
        return;
    }
}

//密码连续输入错误5次，设置提示信息
void UserLoginWidget::setFaildMessage(const QString &message)
{
    m_passwordEdit->abortAuth();

    if (m_isLock) {
        m_lockPasswordWidget->setMessage(message);
        return;
    }

    m_passwordEdit->lineEdit()->setPlaceholderText(message);
}

//密码输入错误,设置错误信息
void UserLoginWidget::setFaildTipMessage(const QString &message)
{
    if (message.isEmpty()) {
        m_passwordEdit->hideAlert();
    } else {
        m_passwordEdit->showAlert(message);
    }
}

//设置窗体显示模式
void UserLoginWidget::setWidgetShowType(UserLoginWidget::WidgetShowType showType)
{
    m_showType = showType;
    updateUI();
}

//更新窗体控件显示
void UserLoginWidget::updateUI()
{
    m_lockPasswordWidget->hide();
    m_otherUserInput->hide();
    switch (m_showType) {
    case NoPasswordType: {
        bool isNopassword = true;
        if (m_authType == SessionBaseModel::LockType) {
            isNopassword = false;
        }
        m_passwordEdit->abortAuth();
        m_passwordEdit->setVisible(!isNopassword && !m_isLock);
        m_lockPasswordWidget->setVisible(m_isLock);

        m_lockButton->show();
        break;
    }
    case NormalType: {
        m_passwordEdit->abortAuth();
        m_passwordEdit->setVisible(!m_isLock);
        m_lockButton->show();
        m_lockPasswordWidget->setVisible(m_isLock);
        break;
    }
    case IDAndPasswordType: {
        m_passwordEdit->hide();
        m_otherUserInput->show();
        m_otherUserInput->setFocus();
        m_lockButton->show();
        break;
    }
    case UserFrameType: {
        m_passwordEdit->hide();
        m_lockButton->hide();
        break;
    }
    case UserFrameLoginType: {
        m_passwordEdit->hide();
        m_lockButton->hide();
        break;
    }
    default:
        break;
    }
}

//设置密码输入框不可用
void UserLoginWidget::disablePassword(bool disable, uint lockNum)
{
    m_isLock = disable;
    m_passwordEdit->lineEdit()->setDisabled(disable);
    m_passwordEdit->setVisible(!disable);
    m_lockPasswordWidget->setVisible(disable);

    if (disable) {
        setFaildMessage(tr("Please try again %n minute(s) later", "", lockNum));
    } else {
        if (isVisible()) {
            grabKeyboard();
        }
    }
}

void UserLoginWidget::updateAuthType(SessionBaseModel::AuthType type)
{
    m_authType = type;
}

void UserLoginWidget::refreshBlurEffectPosition()
{
    QRect rect = m_userLayout->geometry();
    rect.setTop(rect.top() + m_userAvatar->height() / 2 + m_userLayout->margin());

    m_blurEffectWidget->setGeometry(rect);
}

//窗体resize事件,更新阴影窗体的位置
void UserLoginWidget::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, &UserLoginWidget::refreshBlurEffectPosition);

    return QWidget::resizeEvent(event);
}

void UserLoginWidget::showEvent(QShowEvent *event)
{
    updateUI();

    return QWidget::showEvent(event);
}

void UserLoginWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit clicked();
}

void UserLoginWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_isSelected)
        return;
    QPainter painter(this);
    //选中时，在窗体底端居中，绘制92*4尺寸的圆角矩形，样式数据来源于设计图
    painter.setPen(QColor(255, 255, 255, 76));
    painter.setBrush(QColor(255, 255, 255, 76));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRoundedRect(QRect(width() / 2 - 46, height() - 4, 92, 4), 2, 2);
}

//初始化窗体控件
void UserLoginWidget::initUI()
{
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);

    QPalette palette = m_nameLbl->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLbl->setPalette(palette);
    m_nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_nameLbl, DFontSizeManager::T2);
    m_nameLbl->setAlignment(Qt::AlignCenter);

    //暂时先将键盘布局按钮\解锁按钮等隐藏
    m_passwordEdit->setKeyboardButtonEnable(false);
    m_passwordEdit->setEyeButtonEnable(false);
    m_passwordEdit->setSubmitButtonEnable(false);
    m_passwordEdit->setContentsMargins(0, 0, 0, 0);
    m_passwordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);

    m_lockPasswordWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_otherUserInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_passwordEdit->setVisible(true);
    m_passwordEdit->setFocus();
    m_passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    this->setFocusProxy(m_passwordEdit->lineEdit());

    m_userLayout = new QVBoxLayout;
    m_userLayout->setMargin(WidgetsSpacing);
    m_userLayout->setSpacing(WidgetsSpacing);

    m_userLayout->addWidget(m_userAvatar, 0, Qt::AlignHCenter);
    m_userLayout->addWidget(m_nameLbl, 0, Qt::AlignHCenter);
    m_userLayout->addWidget(m_otherUserInput);
    m_userLayout->addWidget(m_passwordEdit);
    m_userLayout->addWidget(m_lockPasswordWidget);
    m_lockPasswordWidget->hide();

    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurEffectWidget->setBlurRectXRadius(BlurRectRadius);
    m_blurEffectWidget->setBlurRectYRadius(BlurRectRadius);

    m_lockButton->setFocusPolicy(Qt::StrongFocus);

    m_lockLayout = new QVBoxLayout;
    m_lockLayout->setMargin(0);
    m_lockLayout->setSpacing(0);
    m_lockLayout->addWidget(m_lockButton, 0, Qt::AlignHCenter);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addStretch();
    mainLayout->addLayout(m_userLayout);
    mainLayout->addLayout(m_lockLayout);
    //此处插入18的间隔，是为了在登录和锁屏多用户切换时，绘制选中的样式（一个92*4的圆角矩形，距离阴影下边间隔14像素）
    mainLayout->addSpacing(18);
    mainLayout->addStretch();

    setLayout(mainLayout);
    updateUI();
}

//初始化槽函数连接
void UserLoginWidget::initConnect()
{
    connect(m_passwordEdit->lineEdit(), &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue(tr("Password"), value);
    });
    connect(m_passwordEdit, &DPasswdEditAnimated::submit, this, [ = ](const QString & passwd) {
        if (passwd.isEmpty()) return;
        emit requestAuthUser(passwd);
    });

    connect(m_lockButton, &QPushButton::clicked, this, [ = ] {
        QString password = m_passwordEdit->lineEdit()->text();
        if (password.isEmpty() && m_showType != NoPasswordType) return;
        emit requestAuthUser(password);
    });
    connect(m_userAvatar, &UserAvatar::clicked, this, &UserLoginWidget::clicked);
}

//设置用户名
void UserLoginWidget::setName(const QString &name)
{
    m_nameLbl->setText(name);
}

//设置用户头像
void UserLoginWidget::setAvatar(const QString &avatar)
{
    m_userAvatar->setIcon(avatar);
}

//设置用户头像尺寸
void UserLoginWidget::setUserAvatarSize(const AvatarSize &avatarSize)
{
    if (avatarSize == AvatarSmallSize) {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarSmallSize);
    } else if (avatarSize == AvatarNormalSize) {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarNormalSize);
    } else {
        m_userAvatar->setAvatarSize(m_userAvatar->AvatarLargeSize);
    }
    m_userAvatar->setFixedSize(avatarSize, avatarSize);
}

void UserLoginWidget::setWidgetWidth(int width)
{
    this->setFixedWidth(width);
}

void UserLoginWidget::setIsLogin(bool isLogin)
{
    m_isLogin = isLogin;
}

bool UserLoginWidget::getIsLogin()
{
    return  m_isLogin;
}

bool UserLoginWidget::getSelected()
{
    return  m_isSelected;
}

void UserLoginWidget::setSelected(bool isSelected)
{
    m_isSelected = isSelected;
    update();
}
