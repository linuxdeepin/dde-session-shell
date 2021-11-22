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
    , m_aniIndex(-1)
    , m_textLabel(new DLabel(this))
{
    setObjectName(QStringLiteral("AuthFingerprint"));
    setAccessibleName(QStringLiteral("AuthFingerprint"));

    m_type = AT_Fingerprint;

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
    m_authStatusLabel = new DLabel(this);
    setAuthStatusStyle(LOGIN_WAIT);
    mainLayout->addWidget(m_authStatusLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
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
    case AS_Success:
        if (isMFA())
            setAuthStatusStyle(LOGIN_CHECK);
        else
            setAnimationStatus(true);
        m_textLabel->setText(tr("Verification successful"));
        m_showPrompt = true;
        emit authFinished(state);
        break;
    case AS_Failure: {
        setAnimationStatus(false);
        if (isMFA())
            setAuthStatusStyle(LOGIN_WAIT);
        m_showPrompt = false;
        const int leftTimes = static_cast<int>(m_limitsInfo->maxTries - m_limitsInfo->numFailures);
        if (leftTimes > 1) {
            m_textLabel->setText(tr("Verification failed, %n chances left", "", leftTimes));
        } else if (leftTimes == 1) {
            m_textLabel->setText(tr("Verification failed, only one chance left"));
        }
        emit authFinished(state);
        break;
    }
    case AS_Cancel:
        setAnimationStatus(false);
        if (isMFA())
            setAuthStatusStyle(LOGIN_WAIT);
        m_showPrompt = true;
        break;
    case AS_Timeout:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AS_Error:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AS_Verify:
        setAnimationStatus(true);
        setAuthStatusStyle(isMFA() ? LOGIN_SPINNER : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AS_Exception:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AS_Prompt:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        if (m_showPrompt) {
            m_textLabel->setText(tr("Verify your fingerprint"));
        }
        break;
    case AS_Started:
        break;
    case AS_Ended:
        break;
    case AS_Locked:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_LOCK : AUTH_LOCK);
        m_textLabel->setText(tr("Fingerprint locked, use password please"));
        break;
    case AS_Recover:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        break;
    case AS_Unlocked:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        break;
    default:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
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
    m_aniIndex = 1;

    if (start) {
        m_aniTimer->start(20);
    } else {
        m_aniTimer->stop();
    }
}

/**
 * @brief 更新认证受限信息
 *
 * @param info
 */
void AuthFingerprint::setLimitsInfo(const LimitsInfo &info)
{
    AuthModule::setLimitsInfo(info);
}

void AuthFingerprint::setAuthFactorType(AuthFactorType authFactorType)
{
    if (DDESESSIONCC::SingleAuthFactor == authFactorType)
        layout()->setContentsMargins(10, 0, 10, 0);

    AuthModule::setAuthFactorType(authFactorType);
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

/**
 * @brief 执行动画（认证成功，认证失败，认证中...）
 */
void AuthFingerprint::doAnimation()
{
    if (m_status == AS_Success) {
        if (m_aniIndex > 10) {
            m_aniTimer->stop();
            emit authFinished(AS_Success);
        } else {
            setAuthStatusStyle(QStringLiteral(":/misc/images/unlock/unlock_%1.svg").arg(m_aniIndex++));
        }
    }
}
