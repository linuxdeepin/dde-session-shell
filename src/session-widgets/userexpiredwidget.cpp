/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zorowk <near.kingzero@gmail.com>
 *
 * Maintainer: zorowk <near.kingzero@gmail.com>
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

#include "userexpiredwidget.h"
#include "framedatabind.h"
#include "userinfo.h"
#include "src/widgets/useravatar.h"
#include "src/widgets/loginbutton.h"
#include "src/global_util/constants.h"
#include "src/session-widgets/framedatabind.h"
#include "src/widgets/keyboardmonitor.h"

#include <DFontSizeManager>
#include <DPalette>
#include "dhidpihelper.h"

#include <QVBoxLayout>
#include <QAction>
#include <QImage>

static const int BlurRectRadius = 15;
static const int WidgetsSpacing = 10;

UserExpiredWidget::UserExpiredWidget(QWidget *parent)
    : QWidget(parent)
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_userAvatar(new UserAvatar(this))
    , m_nameLbl(new QLabel(this))
    , m_passwordEdit(new DLineEdit(this))
    , m_confirmPasswordEdit(new DLineEdit(this))
    , m_lockButton(new DFloatingButton(DStyle::SP_EditElement))
    , m_capslockMonitor(KeyboardMonitor::instance())
{
    initUI();
    initConnect();
}

UserExpiredWidget::~UserExpiredWidget()
{

}

//重置控件的状态
void UserExpiredWidget::resetAllState()
{
    m_passwordEdit->lineEdit()->clear();
    m_passwordEdit->lineEdit()->setPlaceholderText(QString());
    m_confirmPasswordEdit->lineEdit()->clear();
    m_confirmPasswordEdit->lineEdit()->setPlaceholderText(QString());
    m_lockButton->setIcon(DStyle::SP_EditElement);
}

void UserExpiredWidget::setFaildTipMessage(const QString &message)
{
    if (message.isEmpty()) {
        m_passwordEdit->hideAlertMessage();
    } else if (m_passwordEdit->isVisible()) {
        m_passwordEdit->showAlertMessage(message, -1);
        m_passwordEdit->lineEdit()->selectAll();
    }
}

void UserExpiredWidget::onOtherPageConfirmPasswordChanged(const QVariant &value)
{
    int cursorIndex =  m_confirmPasswordEdit->lineEdit()->cursorPosition();
    m_confirmPasswordEdit->setText(value.toString());
    m_confirmPasswordEdit->lineEdit()->setCursorPosition(cursorIndex);
}

void UserExpiredWidget::onOtherPagePasswordChanged(const QVariant &value)
{
    int cursorIndex =  m_passwordEdit->lineEdit()->cursorPosition();
    m_passwordEdit->setText(value.toString());
    m_passwordEdit->lineEdit()->setCursorPosition(cursorIndex);
}

void UserExpiredWidget::refreshBlurEffectPosition()
{
    QRect rect = m_userLayout->geometry();
    rect.setTop(rect.top() + m_userAvatar->height() / 2 + m_userLayout->margin());

    m_blurEffectWidget->setGeometry(rect);
}

//窗体resize事件,更新阴影窗体的位置
void UserExpiredWidget::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, &UserExpiredWidget::refreshBlurEffectPosition);

    return QWidget::resizeEvent(event);
}

void UserExpiredWidget::showEvent(QShowEvent *event)
{
    refreshBlurEffectPosition();
    updateNameLabel();
    return QWidget::showEvent(event);
}

void UserExpiredWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);

    m_passwordEdit->hideAlertMessage();
    m_confirmPasswordEdit->hideAlertMessage();
}

//初始化窗体控件
void UserExpiredWidget::initUI()
{
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);
    m_userAvatar->setFocusPolicy(Qt::NoFocus);

    m_capslockMonitor->start(QThread::LowestPriority);

    QPalette palette = m_nameLbl->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLbl->setPalette(palette);
    m_nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_nameLbl, DFontSizeManager::T2);
    m_nameLbl->setAlignment(Qt::AlignCenter);

    setFocusProxy(m_passwordEdit->lineEdit());
    m_passwordEdit->lineEdit()->setPlaceholderText(tr("new password"));
    m_passwordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_passwordEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setClearButtonEnabled(false);
    m_passwordEdit->lineEdit()->setFocus();

    m_confirmPasswordEdit->lineEdit()->setPlaceholderText(tr("confirm password"));
    m_confirmPasswordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_confirmPasswordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_confirmPasswordEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_confirmPasswordEdit->setFocusPolicy(Qt::StrongFocus);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setClearButtonEnabled(false);

    m_userLayout = new QVBoxLayout;
    m_userLayout->setMargin(WidgetsSpacing);
    m_userLayout->setSpacing(WidgetsSpacing);

    m_userLayout->addWidget(m_userAvatar, 0, Qt::AlignHCenter);

    m_nameLayout = new QHBoxLayout;
    m_nameLayout->setMargin(0);
    m_nameLayout->setSpacing(5);
    m_nameLayout->addWidget(m_nameLbl);
    m_nameFrame = new QFrame;
    m_nameFrame->setLayout(m_nameLayout);
    m_userLayout->addWidget(m_nameFrame, 0, Qt::AlignHCenter);

    m_userLayout->addWidget(m_passwordEdit);
    m_userLayout->addWidget(m_confirmPasswordEdit);

    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurEffectWidget->setMaskAlpha(76);
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

    setTabOrder(m_passwordEdit->lineEdit(), m_confirmPasswordEdit->lineEdit());
    setTabOrder(m_confirmPasswordEdit->lineEdit(), m_lockButton);
}

//初始化槽函数连接
void UserExpiredWidget::initConnect()
{
    connect(m_passwordEdit->lineEdit(), &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue("UserLoginPassword", value);
    });

    connect(m_passwordEdit->lineEdit(), &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue("UserLoginConfimAccount", value);
    });

    connect(m_passwordEdit, &DLineEdit::returnPressed, this, &UserExpiredWidget::onChangePassword);
    connect(m_confirmPasswordEdit, &DLineEdit::returnPressed, this, &UserExpiredWidget::onChangePassword);
    connect(m_lockButton, &QPushButton::clicked, this,  &UserExpiredWidget::onChangePassword);

    //大小写锁定状态改变
    //connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_passwordEdit, &DPasswordEditEx::capslockStatusChanged);

    QMap<QString, int> registerFunctionIndexs;
    std::function<void (QVariant)> confirmPaswordChanged = std::bind(&UserExpiredWidget::onOtherPageConfirmPasswordChanged, this, std::placeholders::_1);
    registerFunctionIndexs["UserLoginConfimAccount"] = FrameDataBind::Instance()->registerFunction("UserLoginAccount", confirmPaswordChanged);
    std::function<void (QVariant)> passwordChanged = std::bind(&UserExpiredWidget::onOtherPagePasswordChanged, this, std::placeholders::_1);
    registerFunctionIndexs["UserLoginPassword"] = FrameDataBind::Instance()->registerFunction("UserLoginPassword", passwordChanged);
    connect(this, &UserExpiredWidget::destroyed, this, [ = ] {
        for (auto it = registerFunctionIndexs.constBegin(); it != registerFunctionIndexs.constEnd(); ++it)
        {
            FrameDataBind::Instance()->unRegisterFunction(it.key(), it.value());
        }
    });

    QTimer::singleShot(0, this, [ = ] {
        FrameDataBind::Instance()->refreshData("UserLoginConfimAccount");
        FrameDataBind::Instance()->refreshData("UserLoginPassword");
    });
}

void UserExpiredWidget::setName(const QString &name)
{
    m_name = name;
    m_nameLbl->setText(name);
}

void UserExpiredWidget::setAvatar(const QString &avatar)
{
    m_userAvatar->setIcon(avatar);
}

void UserExpiredWidget::setUserAvatarSize(const AvatarSize &avatarSize)
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

void UserExpiredWidget::updateNameLabel()
{
    int width = m_nameLbl->fontMetrics().width(m_name);
    int labelMaxWidth = this->width() - 3 * m_nameLayout->spacing();

    if (width > labelMaxWidth) {
        QString str = m_nameLbl->fontMetrics().elidedText(m_name, Qt::ElideRight, labelMaxWidth);
        m_nameLbl->setText(str);
    }
}

void UserExpiredWidget::onChangePassword()
{
    const QString new_pass = m_passwordEdit->text();
    const QString confirm = m_confirmPasswordEdit->text();

    if (errorFilter(new_pass, confirm)) {
        emit requestChangePassword(new_pass);
    }
}

bool UserExpiredWidget::errorFilter(const QString &new_pass, const QString &confirm)
{
    if (new_pass.isEmpty() || confirm.isEmpty()) {
        m_confirmPasswordEdit->showAlertMessage(tr("please enter the password"));

        if (new_pass.isEmpty())
            m_passwordEdit->lineEdit()->setFocus();
        else
            m_confirmPasswordEdit->lineEdit()->setFocus();
        return false;
    } else {
        if (new_pass != confirm) {
            m_confirmPasswordEdit->lineEdit()->clear();
            m_confirmPasswordEdit->lineEdit()->setFocus();
            m_confirmPasswordEdit->showAlertMessage(tr("passwords do not match"));
            return false;
        }
    }

    return true;
}
