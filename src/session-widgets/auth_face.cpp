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

#include "auth_face.h"

#include "authcommon.h"

#include <QKeyEvent>
#include <QTimer>
#include <QVBoxLayout>

using namespace AuthCommon;

AuthFace::AuthFace(QWidget *parent)
    : AuthModule(parent)
    , m_aniIndex(-1)
    , m_textLabel(new DLabel(this))
{
    setObjectName(QStringLiteral("AuthFace"));
    setAccessibleName(QStringLiteral("AuthFace"));

    m_type = AuthTypeFace;

    initUI();
    initConnections();
}

/**
 * @brief 初始化界面
 */
void AuthFace::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    /* 认证状态 */
    m_authStatus = new DLabel(this);
    setAuthStatusStyle(AUTH_LOCK);
    mainLayout->addWidget(m_authStatus, 0, Qt::AlignHCenter);
    /* 文案提示 */
    m_textLabel->setText(tr("Face ID"));
    m_textLabel->setWordWrap(true);
    mainLayout->addWidget(m_textLabel, 0, Qt::AlignHCenter);
}

/**
 * @brief 初始化信号连接
 */
void AuthFace::initConnections()
{
    AuthModule::initConnections();
}

/**
 * @brief AuthFace::reset
 */
void AuthFace::reset()
{
    m_textLabel->setText(tr("Face ID"));
}

/**
 * @brief 设置认证状态
 *
 * @param status
 * @param result
 */
void AuthFace::setAuthStatus(const int state, const QString &result)
{
    m_status = state;

    switch (state) {
    case StatusCodeSuccess:
        setAnimationStatus(true);
        m_textLabel->setText(tr("Verification successful"));
        m_showPrompt = true;
        emit authFinished(state);
        break;
    case StatusCodeFailure: {
        setAnimationStatus(true);
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
        m_showPrompt = true;
        break;
    case StatusCodeTimeout:
        setAnimationStatus(false);
        setAuthStatusStyle(AUTH_LOCK);
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodeError:
        setAnimationStatus(false);
        setAuthStatusStyle(AUTH_LOCK);
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodeVerify:
        setAnimationStatus(true);
        setAuthStatusStyle(AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case StatusCodeException:
        setAnimationStatus(false);
        setAuthStatusStyle(AUTH_LOCK);
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case StatusCodePrompt:
        if (m_showPrompt) {
            setAnimationStatus(false);
            setAuthStatusStyle(AUTH_LOCK);
            m_textLabel->setText(tr("Verify your FaceID"));
        }
        break;
    case StatusCodeStarted:
        break;
    case StatusCodeEnded:
        break;
    case StatusCodeLocked:
        setAnimationStatus(false);
        setAuthStatusStyle(AUTH_LOCK);
        m_textLabel->setText(tr("FaceID locked, use password please"));
        m_showPrompt = true;
        break;
    case StatusCodeRecover:
        setAnimationStatus(false);
        setAuthStatusStyle(AUTH_LOCK);
        m_showPrompt = true;
        break;
    case StatusCodeUnlocked:
        setAnimationStatus(false);
        setAuthStatusStyle(AUTH_LOCK);
        m_showPrompt = true;
        break;
    default:
        setAnimationStatus(false);
        setAuthStatusStyle(AUTH_LOCK);
        m_textLabel->setText(result);
        qWarning() << "Error! The status of Face Auth is wrong!" << state << result;
        break;
    }
    update();
}

/**
 * @brief 设置认证动画状态
 *
 * @param start
 */
void AuthFace::setAnimationStatus(const bool start)
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
void AuthFace::setLimitsInfo(const LimitsInfo &info)
{
    qDebug() << "AuthFace::setLimitsInfo" << info.unlockTime;
    AuthModule::setLimitsInfo(info);
}

/**
 * @brief 更新认证锁定时的文案
 */
void AuthFace::updateUnlockPrompt()
{
    AuthModule::updateUnlockPrompt();
    if (m_integerMinutes > 0) {
        m_textLabel->setText(tr("Please try again %n minute(s) later", "", static_cast<int>(m_integerMinutes)));
    } else {
        QTimer::singleShot(1000, this, [this] {
            emit activeAuth(m_type);
        });
        qInfo() << "Waiting authentication service...";
    }
    update();
}

/**
 * @brief 执行动画（认证成功，认证失败，认证中...）
 */
void AuthFace::doAnimation()
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

void AuthFace::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit activeAuth(AuthTypeFace);
    }
    QWidget::mouseReleaseEvent(event);
}
