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
#include "authfingerprint.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTimer>

using namespace AuthCommon;

AuthFingerprint::AuthFingerprint(QWidget *parent)
    : AuthModule(parent)
    , m_textLabel(new DLabel(this))
{
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
    mainLayout->addWidget(m_authStatus, 0, Qt::AlignRight | Qt::AlignVCenter);
}

/**
 * @brief 初始化信号连接
 */
void AuthFingerprint::initConnections()
{
    /* 解锁时间 */
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
void AuthFingerprint::setAuthResult(const int status, const QString &result)
{
    if (status == m_status) {
        return;
    }
    m_status = status;

    switch (status) {
    case StatusCodeSuccess:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_check.svg");
        m_textLabel->setText(tr("Verification successful"));
        m_showPrompt = true;
        break;
    case StatusCodeFailure: {
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        const int leftTimes = static_cast<int>(m_limitsInfo->maxTries - m_limitsInfo->numFailures);
        if (leftTimes > 1) {
            m_textLabel->setText(tr("Verification failed, %n chances left", "", leftTimes));
        } else if (leftTimes == 1) {
            m_textLabel->setText(tr("Verification failed, only one chance left"));
        }
        m_showPrompt = false;
        emit activeAuth();
        break;
    }
    case StatusCodeCancel:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        // m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodeTimeout:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodeError:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodeVerify:
        setAnimationState(true);
        setAuthStatus(":/misc/images/login_spinner.svg");
        m_textLabel->setText(result);
        break;
    case StatusCodeException:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodePrompt:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        if (m_showPrompt) {
            m_textLabel->setText(tr("Verify your fingerprint"));
        }
        break;
    case StatusCodeStarted:
        // m_textLabel->setText(result);
        break;
    case StatusCodeEnded:
        // m_textLabel->setText(result);
        break;
    case StatusCodeLocked:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_lock.svg");
        m_textLabel->setText(tr("Fingerprint locked, use password please"));
        m_showPrompt = true;
        break;
    case StatusCodeRecover:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        // m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodeUnlocked:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        // m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    default:
        setAnimationState(false);
        setAuthStatus(":/misc/images/login_wait.svg");
        m_textLabel->setText(result);
        qWarning() << "Error! The status of Fingerprint Auth is wrong!" << status << result;
        break;
    }
    update();

}

/**
 * @brief 设置认证动画状态
 *
 * @param start
 */
void AuthFingerprint::setAnimationState(const bool start)
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
 * @brief 更新认证锁定时的文案
 */
void AuthFingerprint::updateUnlockPrompt()
{
    if (m_integerMinutes > 0) {
        m_textLabel->setText(tr("Please try again %n minute(s) later", "", static_cast<int>(m_integerMinutes)));
    } else {
        QTimer::singleShot(1000, this, [=] {
            emit activeAuth();
        });
        qInfo() << "Waiting authentication service...";
    }
    update();
}

void AuthFingerprint::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit activeAuth();
    }
    QWidget::mouseReleaseEvent(event);
}
