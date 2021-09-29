/*
 * Copyright (C) 2021 ~ 2031 Deepin Technology Co., Ltd.
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

#ifndef AUTHENTICATIONMODULE_H
#define AUTHENTICATIONMODULE_H

#include <DIconButton>
#include <DLabel>
#include <DPushButton>

#include <QEvent>
#include <QLabel>
#include <QWidget>

DWIDGET_USE_NAMESPACE
class DLineEditEx;
class AuthenticationModule : public QWidget
{
    Q_OBJECT
public:
    enum TextType {
        AlertText,
        InputText,
        PlaceHolderText
    };
    Q_ENUM(TextType)

    struct LimitsInfo {
        bool locked = false;                         // 账户锁定状态 --- true: 锁定  false: 解锁
        uint maxTries = 0;                           // 最大重试次数
        uint numFailures = 0;                        // 失败次数，一直累加
        uint unlockSecs = 0;                         // 本次锁定总解锁时间（秒），不会随着时间推移减少
        QString unlockTime = "0001-01-01T00:00:00Z"; // 解锁时间（本地时间）
    };

    explicit AuthenticationModule(const int type, QWidget *parent = nullptr);
    ~AuthenticationModule();

    QString lineEditText() const;
    int getAuthStatus() const { return m_status; }
    void setKeyboardButtonVisible(const bool visible);
    void setKeyboardButtontext(const QString &text);
    void setPasswordHint(const QString &hint);

signals:
    void activateAuthentication();
    void authFinished(const int type, const int status);
    void lineEditTextChanged(const QString &);
    void lineEditTextHasFocus(bool focus);
    void unlockTimeChanged();
    void requestAuthenticate();
    void requestChangeFocus();
    void requestShowKeyboardList();

public slots:
    void setAuthResult(const int status, const QString &result);
    void setAnimationState(const bool start);
    void setAuthStatus(const QString &path);
    void setCapsStatus(const bool isCapsOn);
    void setLimitsInfo(const LimitsInfo &info);
    void setLineEditInfo(const QString &text, const TextType type);
    void setNumLockStatus(const QString &path);
    void setKeyboardButtonInfo(const QString &text);
    void setText(const QString &text);
    bool lineEditIsEnable();
    void setLineEditBkColor(const bool value);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void init();
    void updateUnlockTime();

private:
    int m_inputType;               // 输入设备类型
    int m_authType;                // 认证类型
    DLabel *m_authStatus;          // 认证状态
    DLabel *m_capsStatus;          // 大小写状态
    DLabel *m_numLockStatus;       // 数字键盘状态
    DLabel *m_textLabel;           // 中间文字标签
    DLineEditEx *m_lineEdit;       // 输入框
    DPushButton *m_keyboardButton; // 键盘布局按钮
    DIconButton *m_passwordHintBtn; // 密码提示按钮
    LimitsInfo *m_limitsInfo;      // 认证限制信息
    int m_status;                  // 当前认证的认证状态
    QTimer *m_unlockTimer;         // 账户限制计时器
    QTimer *m_unlockTimerTmp;      // 账户限制计时器
    uint m_integerMinutes;
    bool m_showPrompt;
    QString m_iconText;
    QString m_passwordHint;
};

#endif // AUTHENTICATIONMODULE_H
