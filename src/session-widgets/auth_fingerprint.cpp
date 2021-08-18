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

#include "auth_fingerprint.h"

#include "authcommon.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTimer>

using namespace AuthCommon;

AuthFingerprint::AuthFingerprint(QWidget *parent)
    : AuthModule(parent)
    , m_textLabel(new DLabel(this))
{
    setObjectName(QStringLiteral("AuthFingerprint"));
    setAccessibleName(QStringLiteral("AuthFingerprint"));

    m_type = AuthTypeFingerprint;

    initUI();
    initConnections();
}

/**
 * @brief 初始化界面
 */
void AuthFingerprint::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(27, 0, 10, 0);
    mainLayout->setSpacing(0);
    /* 文案提示 */
    m_textLabel->setText(tr("Verify your fingerprint"));
    mainLayout->addWidget(m_textLabel, 1, Qt::AlignHCenter | Qt::AlignVCenter);
    /* 认证状态 */
    m_authStatus = new DLabel(this);
    setAuthStatusStyle(LOGIN_WAIT);
    mainLayout->addWidget(m_authStatus, 0, Qt::AlignRight | Qt::AlignVCenter);
}

/**
 * @brief 初始化信号连接
 */
void AuthFingerprint::initConnections()
{
    AuthModule::initConnections();
}

/**
 * @brief AuthFingerprint::reset
 */
void AuthFingerprint::reset()
{
    m_textLabel->setText(tr("Verify your fingerprint"));
}

/**
 * @brief 设置认证状态
 *
 * @param status
 * @param result
 */
void AuthFingerprint::setAuthStatus(const int state, const QString &result)
{
    qDebug() << "AuthFingerprint::setAuthResult:" << state << result;
    m_status = state;
    switch (state) {
    case StatusCodeSuccess:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_CHECK);
        m_textLabel->setText(tr("Verification successful"));
        m_showPrompt = true;
        emit authFinished(state);
        break;
    case StatusCodeFailure: {
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        const int leftTimes = static_cast<int>(m_limitsInfo->maxTries - m_limitsInfo->numFailures);
        if (leftTimes > 1) {
            m_textLabel->setText(tr("Verification failed, %n chances left", "", leftTimes));
        } else if (leftTimes == 1) {
            m_textLabel->setText(tr("Verification failed, only one chance left"));
        }
        m_showPrompt = false;
        emit authFinished(state);
        break;
    }
    case StatusCodeCancel:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        m_showPrompt = true;
        break;
    case StatusCodeTimeout:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        m_textLabel->setText(result);
        break;
    case StatusCodeError:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        m_textLabel->setText(result);
        break;
    case StatusCodeVerify:
        setAnimationStatus(true);
        setAuthStatusStyle(LOGIN_SPINNER);
        m_textLabel->setText(result);
        break;
    case StatusCodeException:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        m_textLabel->setText(result);
        break;
    case StatusCodePrompt:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        if (m_showPrompt) {
            m_textLabel->setText(tr("Verify your fingerprint"));
        }
        break;
    case StatusCodeStarted:
        break;
    case StatusCodeEnded:
        break;
    case StatusCodeLocked:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_LOCK);
        m_textLabel->setText(tr("Fingerprint locked, use password please"));
        break;
    case StatusCodeRecover:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        break;
    case StatusCodeUnlocked:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        break;
    default:
        setAnimationStatus(false);
        setAuthStatusStyle(LOGIN_WAIT);
        m_textLabel->setText(result);
        qWarning() << "Error! The status of Fingerprint Auth is wrong!" << state << result;
        break;
    }
    update();
}

/**
 * @brief 设置认证动画状态
 *
 * @param start
 */
void AuthFingerprint::setAnimationStatus(const bool start)
{
    Q_UNUSED(start)
}

/**
 * @brief 更新认证受限信息
 *
 * @param info
 */
void AuthFingerprint::setLimitsInfo(const LimitsInfo &info)
{
    qDebug() << "AuthFingerprint::setLimitsInfo" << info.unlockTime;
    AuthModule::setLimitsInfo(info);
}

/**
 * @brief 更新认证锁定时的文案
 */
void AuthFingerprint::updateUnlockPrompt()
{
    AuthModule::updateUnlockPrompt();
    if (m_integerMinutes == 1) {
        m_textLabel->setText(tr("Please try again 1 minute later"));
    } else if (m_integerMinutes > 1) {
        m_textLabel->setText(tr("Please try again %n minute(s) later", "", static_cast<int>(m_integerMinutes)));
    } else {
        QTimer::singleShot(1000, this, [this] {
            emit activeAuth(m_type);
        });
        qInfo() << "Waiting authentication service...";
    }
    update();
}

void AuthFingerprint::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit activeAuth(m_type);
    }
    QWidget::mouseReleaseEvent(event);
}
