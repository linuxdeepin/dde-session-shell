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
#include "src/widgets/loginbutton.h"
#include "src/global_util/constants.h"
#include "src/session-widgets/framedatabind.h"

#include <DFontSizeManager>
#include <DPalette>
#include "dhidpihelper.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QImage>

static const int BlurRectRadius = 15;
static const int WidgetsSpacing = 10;

UserExpiredWidget::UserExpiredWidget(QWidget *parent)
    : QWidget(parent)
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_nameLbl(new QLabel(this))
    , m_oldPasswordEdit(new DLineEdit(this))
    , m_passwordEdit(new DLineEdit(this))
    , m_confirmPasswordEdit(new DLineEdit(this))
    , m_lockButton(new DFloatingButton(DStyle::SP_ArrowNext))
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
    m_oldPasswordEdit->lineEdit()->clear();
    m_passwordEdit->lineEdit()->setPlaceholderText(QString());
    m_passwordEdit->lineEdit()->clear();
    m_passwordEdit->lineEdit()->setPlaceholderText(QString());
    m_confirmPasswordEdit->lineEdit()->clear();
    m_confirmPasswordEdit->lineEdit()->setPlaceholderText(QString());
    if (m_authType == SessionBaseModel::LightdmType) {
        m_lockButton->setIcon(DStyle::SP_ArrowNext);
    } else {
        m_lockButton->setIcon(DStyle::SP_UnlockElement);
    }
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

void UserExpiredWidget::onOtherPageOldPasswordChanged(const QVariant &value)
{
    int cursorIndex =  m_oldPasswordEdit->lineEdit()->cursorPosition();
    m_oldPasswordEdit->setText(value.toString());
    m_oldPasswordEdit->lineEdit()->setCursorPosition(cursorIndex);
}

void UserExpiredWidget::refreshBlurEffectPosition()
{
    QRect rect = m_userLayout->geometry();
    rect.setTop(rect.top() + m_userLayout->margin());

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
    QPalette palette = m_nameLbl->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLbl->setPalette(palette);
    m_nameLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_nameLbl, DFontSizeManager::T2);
    m_nameLbl->setAlignment(Qt::AlignCenter);

    setFocusProxy(m_oldPasswordEdit->lineEdit());
    m_oldPasswordEdit->lineEdit()->setPlaceholderText(tr("Old password"));
    m_oldPasswordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_oldPasswordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_oldPasswordEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_oldPasswordEdit->setFocusPolicy(Qt::StrongFocus);
    m_oldPasswordEdit->setEchoMode(QLineEdit::Password);
    m_oldPasswordEdit->setClearButtonEnabled(false);

    m_passwordEdit->lineEdit()->setPlaceholderText(tr("New password"));
    m_passwordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_passwordEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setClearButtonEnabled(false);
    m_passwordEdit->lineEdit()->setFocus();

    m_confirmPasswordEdit->lineEdit()->setPlaceholderText(tr("Repeat password"));
    m_confirmPasswordEdit->lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    m_confirmPasswordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_confirmPasswordEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_confirmPasswordEdit->setFocusPolicy(Qt::StrongFocus);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setClearButtonEnabled(false);

    m_userLayout = new QVBoxLayout;
    m_userLayout->setMargin(WidgetsSpacing);
    m_userLayout->setSpacing(WidgetsSpacing);

    m_nameLayout = new QHBoxLayout;
    m_nameLayout->setMargin(10);
    m_nameLayout->setSpacing(5);
    m_nameLayout->addWidget(m_nameLbl);
    m_nameFrame = new QFrame;
    m_nameFrame->setLayout(m_nameLayout);
    m_userLayout->addWidget(m_nameFrame, 0, Qt::AlignHCenter);

    m_userLayout->addWidget(m_oldPasswordEdit);
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

    setTabOrder(m_oldPasswordEdit->lineEdit(), m_passwordEdit->lineEdit());
    setTabOrder(m_passwordEdit->lineEdit(), m_confirmPasswordEdit->lineEdit());
    setTabOrder(m_confirmPasswordEdit->lineEdit(), m_lockButton);
}

//初始化槽函数连接
void UserExpiredWidget::initConnect()
{
    connect(m_oldPasswordEdit->lineEdit(), &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue("UserOldPassword", value);
    });

    connect(m_passwordEdit->lineEdit(), &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue("UserPassword", value);
    });

    connect(m_confirmPasswordEdit->lineEdit(), &QLineEdit::textChanged, this, [ = ](const QString & value) {
        FrameDataBind::Instance()->updateValue("UserConfimPassword", value);
    });

    connect(m_oldPasswordEdit, &DLineEdit::returnPressed, this, &UserExpiredWidget::onChangePassword);
    connect(m_passwordEdit, &DLineEdit::returnPressed, this, &UserExpiredWidget::onChangePassword);
    connect(m_confirmPasswordEdit, &DLineEdit::returnPressed, this, &UserExpiredWidget::onChangePassword);
    connect(m_lockButton, &QPushButton::clicked, this,  &UserExpiredWidget::onChangePassword);

    QMap<QString, int> registerFunctionIndexs;
    std::function<void (QVariant)> oldPasswordChanged = std::bind(&UserExpiredWidget::onOtherPageOldPasswordChanged, this, std::placeholders::_1);
    registerFunctionIndexs["UserOldPassword"] = FrameDataBind::Instance()->registerFunction("UserOldPassword", oldPasswordChanged);
    std::function<void (QVariant)> confirmPaswordChanged = std::bind(&UserExpiredWidget::onOtherPageConfirmPasswordChanged, this, std::placeholders::_1);
    registerFunctionIndexs["UserConfimPassword"] = FrameDataBind::Instance()->registerFunction("UserConfimPassword", confirmPaswordChanged);
    std::function<void (QVariant)> passwordChanged = std::bind(&UserExpiredWidget::onOtherPagePasswordChanged, this, std::placeholders::_1);
    registerFunctionIndexs["UserPassword"] = FrameDataBind::Instance()->registerFunction("UserPassword", passwordChanged);
    connect(this, &UserExpiredWidget::destroyed, this, [ = ] {
        for (auto it = registerFunctionIndexs.constBegin(); it != registerFunctionIndexs.constEnd(); ++it)
        {
            FrameDataBind::Instance()->unRegisterFunction(it.key(), it.value());
        }
    });

    QTimer::singleShot(0, this, [ = ] {
        FrameDataBind::Instance()->refreshData("UserOldPassword");
        FrameDataBind::Instance()->refreshData("UserPassword");
        FrameDataBind::Instance()->refreshData("UserConfimPassword");
    });
}

void UserExpiredWidget::setDisplayName(const QString &name)
{
    m_showName = name;
    m_nameLbl->setText(name);
}

void UserExpiredWidget::setUserName(const QString &name)
{
    m_userName = name;
}

void UserExpiredWidget::updateNameLabel()
{
    int width = m_nameLbl->fontMetrics().width(m_showName);
    int labelMaxWidth = this->width() - 3 * m_nameLayout->spacing();

    if (width > labelMaxWidth) {
        QString str = m_nameLbl->fontMetrics().elidedText(m_showName, Qt::ElideRight, labelMaxWidth);
        m_nameLbl->setText(str);
    }
}

void UserExpiredWidget::updateAuthType(SessionBaseModel::AuthType type)
{
    m_authType = type;
    if (m_authType == SessionBaseModel::LightdmType) {
        m_lockButton->setIcon(DStyle::SP_ArrowNext);
    }
}


void UserExpiredWidget::onChangePassword()
{
    const QString old_pass = m_oldPasswordEdit->text();
    const QString new_pass = m_passwordEdit->text();
    const QString confirm = m_confirmPasswordEdit->text();

    if (errorFilter(old_pass, new_pass, confirm)) {
#define TIMEOUT 1000

        QProcess process;
        process.start("su", {m_userName});
        process.setReadChannel(QProcess::StandardError);

        if (!process.waitForStarted(TIMEOUT)) {
            qWarning() << "failed on start 'su'";
            return;
        }

        qDebug() << QString::fromLocal8Bit(process.readAllStandardError());
        process.write(old_pass.toLocal8Bit());
        process.write("\n");
        process.waitForReadyRead(TIMEOUT);
        const QString errorString = QString::fromLocal8Bit(process.readLine());
        qDebug() << errorString;
        qDebug() << QString::fromLocal8Bit(process.readAllStandardError());
        process.write(old_pass.toLocal8Bit());
        process.write("\n");
        process.waitForReadyRead(TIMEOUT);
        qDebug() << QString::fromLocal8Bit(process.readAllStandardError());
        process.write(new_pass.toLocal8Bit());
        process.write("\n");
        process.waitForReadyRead(TIMEOUT);
        qDebug() << QString::fromLocal8Bit(process.readAllStandardError());
        process.write(confirm.toLocal8Bit());
        process.write("\n");
        process.waitForFinished(TIMEOUT);

        if (process.exitCode() != 0) {
            process.terminate();
            m_confirmPasswordEdit->showAlertMessage(tr("Failed to change your password"));
            qDebug() << "password modify failed: " << errorString;
        } else {
            emit changePasswordFinished();
            qDebug() << "password modify success";
        }

        if (process.state() == QProcess::Running) process.kill();
    }
}

bool UserExpiredWidget::errorFilter(const QString &old_pass, const QString &new_pass, const QString &confirm)
{
    if (old_pass.isEmpty() || new_pass.isEmpty() || confirm.isEmpty()) {

        if (old_pass.isEmpty()) {
            m_oldPasswordEdit->lineEdit()->setFocus();
            m_oldPasswordEdit->showAlertMessage(tr("Please enter the old password"));
            return false;
        }

        if (new_pass.isEmpty()) {
            m_passwordEdit->lineEdit()->setFocus();
            m_passwordEdit->showAlertMessage(tr("Please enter the new password"));
            return false;
        }

        if (confirm.isEmpty()) {
            m_confirmPasswordEdit->lineEdit()->setFocus();
            m_confirmPasswordEdit->showAlertMessage(tr("Please repeat the new password"));
            return false;
        }
    } else {
        if (new_pass != confirm) {
            m_confirmPasswordEdit->lineEdit()->setFocus();
            m_confirmPasswordEdit->showAlertMessage(tr("Passwords do not match"));
            return false;
        }
    }

    return true;
}
