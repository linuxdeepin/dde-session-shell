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

#ifndef AUTHMODULE_H
#define AUTHMODULE_H

#include <DLabel>
#include <QWidget>

DWIDGET_USE_NAMESPACE

struct LimitsInfo {
    bool locked;        // 认证锁定状态 --- true: 锁定  false: 解锁
    uint maxTries;      // 最大重试次数
    uint numFailures;   // 失败次数，一直累加
    uint unlockSecs;    // 本次锁定总解锁时间（秒），不会随着时间推移减少
    QString unlockTime; // 解锁时间（本地时间）

    void operator=(const LimitsInfo &info);
};

class AuthModule : public QWidget
{
    Q_OBJECT
public:
    enum TextType {
        AlertText,
        InputText,
        PlaceHolderText
    };
    Q_ENUM(TextType)

    explicit AuthModule(QWidget *parent = nullptr);
    ~AuthModule() override;

    inline int authStatus() const { return m_status; }
    inline int authType() const { return m_type; }

    virtual void setAnimationState(const bool start);
    virtual void setAuthResult(const int status, const QString &result);
    void setAuthStatus(const QString &path);
    virtual void setLimitsInfo(const LimitsInfo &info);

signals:
    void activeAuth();
    void authFinished(const int);
    void requestAuthenticate();
    void unlockTimeChanged();

protected:
    virtual void updateUnlockPrompt() { }
    void updateUnlockTime();

protected:
    int m_inputType;          // 认证信息输入设备类型
    int m_status;             // 认证状态
    int m_type;               // 认证类型
    bool m_showPrompt;        // 是否显示默认提示文案
    uint m_integerMinutes;    // 认证剩余解锁的整数分钟
    LimitsInfo *m_limitsInfo; // 认证限制相关信息
    DLabel *m_authStatus;     // 认证状态图标
    QTimer *m_unlockTimer;    // 认证解锁定时器
};

#endif // AUTHMODULE_H
