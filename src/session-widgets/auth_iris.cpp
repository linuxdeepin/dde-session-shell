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

#include "auth_iris.h"

#include "authcommon.h"

#include <QKeyEvent>
#include <QTimer>
#include <QVBoxLayout>

using namespace AuthCommon;

AuthIris::AuthIris(QWidget *parent)
    : AuthModule(parent)
    , m_aniIndex(-1)
    , m_textLabel(new DLabel(this))
{
    setObjectName(QStringLiteral("AuthIris"));
    setAccessibleName(QStringLiteral("AuthIris"));

    m_type = AuthTypeIris;

    initUI();
    initConnections();
}

/**
 * @brief 初始化界面
 */
void AuthIris::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(27, 0, 10, 0);
    mainLayout->setSpacing(0);
    /* 文案提示 */
    m_textLabel->setText(tr("Iris ID"));
    m_textLabel->setWordWrap(true);
    mainLayout->addWidget(m_textLabel, 1, Qt::AlignHCenter);
    /* 认证状态 */
    m_authStatusLabel = new DLabel(this);
    m_authStatusLabel->installEventFilter(this);
    setAuthStatusStyle(LOGIN_WAIT);
    mainLayout->addWidget(m_authStatusLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
}

/**
 * @brief 初始化信号连接
 */
void AuthIris::initConnections()
{
    AuthModule::initConnections();
}

/**
 * @brief AuthFace::reset
 */
void AuthIris::reset()
{
    m_textLabel->setText(tr("Iris ID"));
    if (m_authStatusLabel) {
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_authStatusLabel->show();
    }
}

/**
 * @brief 设置认证状态
 *
 * @param status
 * @param result
 */
void AuthIris::setAuthStatus(const int state, const QString &result)
{
    m_status = state;
    switch (state) {
    case StatusCodeSuccess:
        if (isMFA())
            setAuthStatusStyle(LOGIN_CHECK);
        else
            setAnimationStatus(true);
        m_textLabel->setText(tr("Verification successful"));
        m_showPrompt = true;
        emit authFinished(state);
        emit retryButtonVisibleChanged(false);
        break;
    case StatusCodeFailure: {
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_RETRY : AUTH_LOCK);
        m_showPrompt = false;
        const int leftTimes = static_cast<int>(m_limitsInfo->maxTries - m_limitsInfo->numFailures);
        if (leftTimes > 1) {
            m_textLabel->setText(tr("Verification failed, %n chances left", "", leftTimes));
        } else if (leftTimes == 1) {
            m_textLabel->setText(tr("Verification failed, only one chance left"));
        }
        emit authFinished(state);
        emit retryButtonVisibleChanged(true);
        break;
    }
    case StatusCodeCancel:
        setAnimationStatus(false);
        m_showPrompt = true;
        break;
    case StatusCodeTimeout:
    case StatusCodeError:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_SPINNER : AUTH_LOCK);
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodeVerify:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case StatusCodeException:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodePrompt:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        break;
    case StatusCodeStarted:
        m_textLabel->setText(tr("Verify your IrisID"));
        break;
    case StatusCodeEnded:
        break;
    case StatusCodeLocked:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_LOCK : AUTH_LOCK);
        m_textLabel->setText(tr("IrisID locked, use password please"));
        m_showPrompt = true;
        if (DDESESSIONCC::SingleAuthFactor == m_authFactorType)
            emit retryButtonVisibleChanged(false);
        break;
    case StatusCodeRecover:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_showPrompt = true;
        break;
    case StatusCodeUnlocked:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_showPrompt = true;
        break;
    default:
        setAnimationStatus(false);
        setAuthStatusStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        qWarning() << "Error! The status of Iris Auth is wrong!" << state << result;
        break;
    }
    update();
}

/**
 * @brief 设置认证动画状态
 *
 * @param start
 */
void AuthIris::setAnimationStatus(const bool start)
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
void AuthIris::setLimitsInfo(const LimitsInfo &info)
{
    AuthModule::setLimitsInfo(info);
}

/**
 * @brief 更新认证锁定时的文案
 */
void AuthIris::updateUnlockPrompt()
{
    AuthModule::updateUnlockPrompt();
    if (m_integerMinutes > 0) {
        m_textLabel->setText(tr("Please try again %n minute(s) later", "", static_cast<int>(m_integerMinutes)));
    } else {
        QTimer::singleShot(1000, this, [this] {
            emit activeAuth(AuthTypeIris);
        });
        qInfo() << "Waiting authentication service...";
    }
    update();
}

/**
 * @brief 执行动画（认证成功，认证失败，认证中...）
 */
void AuthIris::doAnimation()
{
    if (m_status == StatusCodeSuccess) {
        if (m_aniIndex > 10) {
            m_aniTimer->stop();
            emit authFinished(StatusCodeSuccess);
        } else {
            setAuthStatusStyle(QStringLiteral(":/misc/images/unlock/unlock_%1.svg").arg(m_aniIndex++));
        }
    }
}

bool AuthIris::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_authStatusLabel
            && QEvent::MouseButtonRelease == event->type()
            && !m_isAuthing
            && StatusCodeLocked != m_status
            && DDESESSIONCC::MultiAuthFactor == m_authFactorType) {
        emit activeAuth(AuthTypeIris);
    }

    return false;
}

void AuthIris::setAuthFactorType(AuthFactorType authFactorType)
{
    if (DDESESSIONCC::SingleAuthFactor == authFactorType)
        layout()->setContentsMargins(10, 0, 10, 0);

    AuthModule::setAuthFactorType(authFactorType);
}

