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

#include "authcommon.h"
#include "authsingle.h"
#include "dlineeditex.h"

#include <DHiDPIHelper>
#include <QKeyEvent>
#include <QTimer>

using namespace AuthCommon;

AuthSingle::AuthSingle(QWidget *parent)
    : AuthModule(parent)
    , m_capsStatus(new DLabel(this))
    , m_lineEdit(new DLineEditEx(this))
    , m_keyboardButton(new DPushButton(this))
{
    initUI();
    initConnections();

    m_lineEdit->installEventFilter(this);
    setFocusProxy(m_lineEdit);
}

/**
 * @brief 初始化界面
 */
void AuthSingle::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_lineEdit->setClearButtonEnabled(false);
    m_lineEdit->setEchoMode(QLineEdit::Password);
    m_lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_lineEdit->lineEdit()->setAlignment(Qt::AlignCenter);

    QHBoxLayout *passwordLayout = new QHBoxLayout(m_lineEdit->lineEdit());
    passwordLayout->setContentsMargins(0, 0, 10, 0);
    passwordLayout->setSpacing(5);
    /* 键盘布局按钮 */
    m_keyboardButton->setContentsMargins(0, 0, 0, 0);
    m_keyboardButton->setFocusPolicy(Qt::NoFocus);
    m_keyboardButton->setCursor(Qt::ArrowCursor);
    m_keyboardButton->setFlat(true);
    passwordLayout->addWidget(m_keyboardButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    /* 大小写状态 */
    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/misc/images/caps_lock.svg");
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_capsStatus->setPixmap(pixmap);
    passwordLayout->addWidget(m_capsStatus, 1, Qt::AlignRight | Qt::AlignVCenter);
    /* 认证状态 */
    passwordLayout->addWidget(m_authStatus, 0, Qt::AlignRight | Qt::AlignVCenter);

    mainLayout->addWidget(m_lineEdit);
}

/**
 * @brief 初始化信号连接
 */
void AuthSingle::initConnections()
{
    /* 键盘布局按钮 */
    connect(m_keyboardButton, &QPushButton::clicked, this, &AuthSingle::requestShowKeyboardList);
    /* 密码输入框 */
    connect(m_lineEdit, &DLineEditEx::lineEditTextHasFocus, this, [this](const bool focus) {
        if (focus) {
            m_authStatus->hide();
        } else {
            m_authStatus->show();
        }
        emit focusChanged(focus);
    });
    connect(m_lineEdit, &DLineEditEx::textChanged, this, &AuthSingle::lineEditTextChanged);
    connect(m_lineEdit, &DLineEditEx::returnPressed, this, [this] {
        if (!m_lineEdit->text().isEmpty()) {
            setAnimationState(true);
            m_lineEdit->clearFocus();
            m_lineEdit->setFocusPolicy(Qt::NoFocus);
            emit requestAuthenticate();
        }
    });
    /* 认证解锁时间 */
    connect(m_unlockTimer, &QTimer::timeout, this, [this] {
        if (m_integerMinutes > 0) {
            m_integerMinutes--;
        }
        if (m_integerMinutes == 0) {
            m_unlockTimer->stop();
        }
        updateUnlockPrompt();
    });
}

/**
 * @brief 设置认证状态
 *
 * @param status
 * @param result
 */
void AuthSingle::setAuthResult(const int status, const QString &result)
{
    if (status == m_status) {
        return;
    }
    m_status = status;

    switch (status) {
    case StatusCodeSuccess:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_check.svg");
        m_lineEdit->clear();
        m_lineEdit->setFocusPolicy(Qt::NoFocus);
        m_lineEdit->clearFocus();
        setLineEditInfo(result, PlaceHolderText);
        emit authFinished(StatusCodeSuccess);
        break;
    case StatusCodeFailure: {
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        m_lineEdit->clear();
        m_lineEdit->setFocusPolicy(Qt::StrongFocus);
        m_lineEdit->setFocus();
        setLineEditInfo(result, AlertText);
        m_showPrompt = false;
        emit authFinished(StatusCodeFailure);
        emit activeAuth();
        break;
    }
    case StatusCodeCancel:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        break;
    case StatusCodeTimeout:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        setLineEditInfo(result, AlertText);
        break;
    case StatusCodeError:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        setLineEditInfo(result, AlertText);
        break;
    case StatusCodeVerify:
        setAnimationState(true);
        setAuthStatus(":/misc/images/login_spinner.svg");
        break;
    case StatusCodeException:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        setLineEditInfo(result, AlertText);
        break;
    case StatusCodePrompt:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        m_lineEdit->clear();
        m_lineEdit->setFocusPolicy(Qt::StrongFocus);
        m_lineEdit->setFocus();
        setLineEditInfo(result, PlaceHolderText);
        break;
    case StatusCodeStarted:
        break;
    case StatusCodeEnded:
        break;
    case StatusCodeLocked:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_lock.svg");
        m_lineEdit->clear();
        m_lineEdit->setFocusPolicy(Qt::NoFocus);
        m_lineEdit->clearFocus();
        break;
    case StatusCodeRecover:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        break;
    case StatusCodeUnlocked:
        setAuthStatus(":/misc/images/login_wait.svg");
        break;
    default:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        setLineEditInfo(result, AlertText);
        qWarning() << "Error! The status of Password Auth is wrong!" << status << result;
        break;
    }
    update();
}

/**
 * @brief 设置认证动画状态
 *
 * @param start
 */
void AuthSingle::setAnimationState(const bool start)
{
    if (start) {
        m_lineEdit->startAnimation();
    } else {
        m_lineEdit->stopAnimation();
    }
}

/**
 * @brief 设置大小写图标状态
 *
 * @param isCapsOn
 */
void AuthSingle::setCapsStatus(const bool isCapsOn)
{
    if (isCapsOn) {
        m_capsStatus->show();
    } else {
        m_capsStatus->hide();
    }
}

/**
 * @brief 更新认证受限信息
 *
 * @param info
 */
void AuthSingle::setLimitsInfo(const LimitsInfo &info)
{
    if (info.locked && info.locked != m_limitsInfo->locked && info.unlockTime != m_limitsInfo->unlockTime) {
        m_limitsInfo->locked = info.locked;
        m_limitsInfo->unlockTime = info.unlockTime;
        setAuthResult(StatusCodeLocked, QString(""));
        updateUnlockTime();
    }
    m_limitsInfo->maxTries = info.maxTries;
    m_limitsInfo->numFailures = info.numFailures;
    m_limitsInfo->unlockSecs = info.unlockSecs;
}

/**
 * @brief 设置输入框中的文案
 *
 * @param text
 * @param type
 */
void AuthSingle::setLineEditInfo(const QString &text, const TextType type)
{
    switch (type) {
    case AlertText:
        m_lineEdit->showAlertMessage(text, this, 3000);
        m_lineEdit->setAlert(true);
        break;
    case InputText: {
        const int cursorPos = m_lineEdit->lineEdit()->cursorPosition();
        m_lineEdit->setText(text);
        m_lineEdit->lineEdit()->setCursorPosition(cursorPos);
        break;
    }
    case PlaceHolderText:
        m_lineEdit->setPlaceholderText(text);
        break;
    }
}

/**
 * @brief 设置数字键盘状态
 *
 * @param path
 */
void AuthSingle::setNumLockStatus(const QString &path)
{
    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(path);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_numLockStatus->setPixmap(pixmap);
}

/**
 * @brief 设置键盘布局按钮显示的文案
 *
 * @param text
 */
void AuthSingle::setKeyboardButtonInfo(const QString &text)
{
    if (!m_keyboardButton->isVisible()) {
        return;
    }
    QString tmpText = text;
    tmpText = tmpText.split(";").first();
    /* special mark in Japanese */
    if (tmpText.contains("/")) {
        tmpText = tmpText.split("/").last();
    }

    QFont font = m_lineEdit->font();
    font.setPixelSize(32);
    font.setWeight(QFont::DemiBold);
    int word_size = QFontMetrics(font).height();
    qreal device_ratio = this->devicePixelRatioF();
    QImage image(QSize(word_size, word_size) * device_ratio, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    image.setDevicePixelRatio(device_ratio);

    QPainter painter(&image);
    painter.setFont(font);
    painter.setPen(Qt::white);

    QRect image_rect = image.rect();
    QRect r(image_rect.left(), image_rect.top(), word_size, word_size);
    painter.drawText(r, Qt::AlignCenter, tmpText);

    m_keyboardButton->setIcon(QIcon(QPixmap::fromImage(image)));
}

/**
 * @brief 设置键盘布局按钮的显示状态
 *
 * @param visible
 */
void AuthSingle::setKeyboardButtonVisible(const bool visible)
{
    m_keyboardButton->setVisible(visible);
}

/**
 * @brief 获取输入框中的文字
 *
 * @return QString
 */
QString AuthSingle::lineEditText() const
{
    return m_lineEdit->text();
}

/**
 * @brief 更新认证锁定时的文案
 */
void AuthSingle::updateUnlockPrompt()
{
    if (m_integerMinutes > 0) {
        m_lineEdit->setPlaceholderText(tr("Please try again %n minute(s) later", "", static_cast<int>(m_integerMinutes)));
    } else {
        QTimer::singleShot(1000, this, [this] {
            emit activeAuth();
        });
        qInfo() << "Waiting authentication service...";
    }
}

bool AuthSingle::eventFilter(QObject *watched, QEvent *event)
{
    if (qobject_cast<DLineEditEx *>(watched) == m_lineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Cut)
            || keyEvent->matches(QKeySequence::Copy)
            || keyEvent->matches(QKeySequence::Paste)) {
            return true;
        }
    }
    return false;
}
