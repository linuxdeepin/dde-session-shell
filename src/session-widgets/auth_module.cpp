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

#include "auth_module.h"

#include <DHiDPIHelper>

#include <QDateTime>
#include <QTimer>
#include <QtMath>

void LimitsInfo::operator=(const LimitsInfo &info)
{
    locked = info.locked;
    maxTries = info.maxTries;
    numFailures = info.numFailures;
    unlockSecs = info.unlockSecs;
    unlockTime = info.unlockTime;
}

AuthModule::AuthModule(const AuthCommon::AuthType type, QWidget *parent)
    : QWidget(parent)
    , m_status(AuthCommon::AS_None)
    , m_type(type)
    , m_showPrompt(true)
    , m_limitsInfo(new LimitsInfo())
    , m_aniTimer(new QTimer(this))
    , m_unlockTimer(new QTimer(this))
    , m_showAuthStatus(true)
    , m_isAuthing(false)
    , m_authFactorType(DDESESSIONCC::SingleAuthFactor)
{
    m_limitsInfo->locked = false;
    m_limitsInfo->maxTries = 0;
    m_limitsInfo->numFailures = 0;
    m_limitsInfo->unlockSecs = 0;
    m_limitsInfo->unlockTime = QString("0001-01-01T00:00:00Z");

    m_unlockTimer->setSingleShot(false);
    m_unlockTimer->setInterval(1000);
}

AuthModule::~AuthModule()
{
    delete m_limitsInfo;
}

/**
 * @brief 初始化信号连接
 */
void AuthModule::initConnections()
{
    /* 认证解锁时间 */
    connect(m_unlockTimer, &QTimer::timeout, this, [ this ] {
        updateIntegerMinutes();
        if (m_integerMinutes <= 0)
            m_unlockTimer->stop();
        updateUnlockPrompt();
    });
    /* 解锁动画 */
    connect(m_aniTimer, &QTimer::timeout, this, &AuthModule::doAnimation);
}

/**
 * @brief 设置认证动画状态，由派生类重载，自定义动画样式。
 *
 * @param start
 */
void AuthModule::setAnimationStatus(const bool start)
{
    Q_UNUSED(start)
}

/**
 * @brief 设置认证的状态，由派生类重载，自定义认证状态显示样式。
 *
 * @param status
 * @param result
 */
void AuthModule::setAuthStatus(const int status, const QString &result)
{
    Q_UNUSED(result)

    if (AuthCommon::AS_Started == status)
        m_isAuthing = true;
    else if (AuthCommon::AS_Ended == status)
        m_isAuthing = false;
}

/**
 * @brief 设置认证状态图标的样式
 *
 * @param path
 */
void AuthModule::setAuthStatusStyle(const QString &path)
{
    if (!m_authStatusLabel)
        return;

    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(path);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_authStatusLabel->setPixmap(pixmap);
}

/**
 * @brief 设置认证受限信息
 *
 * @param info
 */
void AuthModule::setLimitsInfo(const LimitsInfo &info)
{
    *m_limitsInfo = info;
    updateUnlockTime();
}

void AuthModule::setShowAuthStatus(bool showAuthStatus)
{
    m_showAuthStatus = showAuthStatus;
}

void AuthModule::setAuthStatueVisible(bool visible)
{
    if (m_authStatusLabel)
        m_authStatusLabel->setVisible(visible);
}

/**
 * @brief 更新认证剩余解锁时间提示
 */
void AuthModule::updateUnlockPrompt()
{
    qInfo() << m_type << "has" << m_integerMinutes << "minutes left to unlock.";
}

/**
 * @brief 更新认证锁定后的解锁时间
 */
void AuthModule::updateUnlockTime()
{
    if (QDateTime::fromString(m_limitsInfo->unlockTime, Qt::ISODateWithMs) <= QDateTime::currentDateTime()) {
        m_integerMinutes = 0;
        m_unlockTimer->stop();
        return;
    }
    updateIntegerMinutes();
    updateUnlockPrompt();
    m_unlockTimer->start();
}

void AuthModule::updateIntegerMinutes()
{
    if (QDateTime::fromString(m_limitsInfo->unlockTime, Qt::ISODateWithMs) > QDateTime::currentDateTime()) {
        qreal intervalSeconds = QDateTime::fromString(m_limitsInfo->unlockTime, Qt::ISODateWithMs).toLocalTime().toTime_t()
                               - QDateTime::currentDateTimeUtc().toTime_t();
        m_integerMinutes = static_cast<uint>(qCeil(intervalSeconds / 60));
    } else {
        m_integerMinutes = 0;
    }
}

void AuthModule::setAuthStatusLabel(DLabel *label)
{
    if (!label)
        return;

    if (m_authStatusLabel) {
        delete m_authStatusLabel;
        m_authStatusLabel = nullptr;
    }

    /* 认证状态 */
    m_authStatusLabel = label;
    setAuthStatusStyle(AUTH_LOCK);
}

void AuthModule::setAuthFactorType(AuthFactorType authFactorType)
{
    m_authFactorType = authFactorType;
}
