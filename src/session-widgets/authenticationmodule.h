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
    /* 后续通过输入类型区分构建 widget 的样式 */
    enum InputType {
        InputDefault = 0, // 默认输入方式
        InputKeyboard,    // 键盘输入
        InputFinger,      // 指纹和指静脉
        InputCameraFace,  // 人脸摄像头
        InputCameraIris   // 虹膜摄像头
    };
    Q_ENUM(InputType)

    enum AuthType {
        AuthTypePassword = 1 << 0,        // 密码
        AuthTypeFingerprint = 1 << 1,     // 指纹
        AuthTypeFace = 1 << 2,            // 人脸
        AuthTypeActiveDirectory = 1 << 3, // AD域
        AuthTypeUkey = 1 << 4,            // ukey
        AuthTypeFingerVein = 1 << 5,      // 指静脉
        AuthTypeIris = 1 << 6,            // 虹膜
        AuthTypePIN = 1 << 7,             // PIN
        AuthTypeAll = -1
    };
    Q_ENUM(AuthType)

    enum TextType {
        AlertText,
        InputText,
        PlaceHolderText
    };
    Q_ENUM(TextType)

    enum AuthStatus {
        StatusCodeSuccess = 0, // 成功，此次认证的最终结果
        StatusCodeFailure,     // 失败，此次认证的最终结果
        StatusCodeCancel,      // 取消，当认证没有给出最终结果时，调用 End 会出发 Cancel 信号
        StatusCodeTimeout,     // 超时
        StatusCodeError,       // 错误
        StatusCodeVerify,      // 验证中
        StatusCodeException,   // 设备异常，当前认证会被 End
        StatusCodePrompt,      // 设备提示
        StatusCodeStarted,     // 认证已启动，调用 Start 之后，每种成功开启都会发送此信号
        StatusCodeEnded,       // 认证已结束，调用 End 之后，每种成功关闭的都会发送此信号，当某种认证类型被锁定时，也会触发此信号
        StatusCodeLocked,      // 认证已锁定，当认证类型锁定时，触发此信号。该信号不会给出锁定等待时间信息
        StatusCodeRecover,     // 设备恢复，需要调用 Start 重新开启认证，对应 StatusCodeException
        StatusCodeUnlocked     // 认证解锁，对应 StatusCodeLocked
    };
    Q_ENUM(AuthStatus)

    struct LimitsInfo {
        bool locked = false;                         // 账户锁定状态 --- true: 锁定  false: 解锁
        uint maxTries = 0;                           // 最大重试次数
        uint numFailures = 0;                        // 失败次数，一直累加
        uint unlockSecs = 0;                         // 本次锁定总解锁时间（秒），不会随着时间推移减少
        QString unlockTime = "0001-01-01T00:00:00Z"; // 解锁时间（本地时间）
    };

    explicit AuthenticationModule(const AuthType type, QWidget *parent = nullptr);
    ~AuthenticationModule();

    QString lineEditText() const;
    AuthStatus getAuthStatus() {return m_status;}
    void setKeyboardButtonVisible(const bool visible);
    void setKeyboardButtontext(const QString &text);

signals:
    void activateAuthentication();
    void authFinished(const AuthType &type, const AuthStatus &status);
    void lineEditTextChanged(const QString &);
    void lineEditTextHasFocus(bool focus);
    void unlockTimeChanged();
    void requestAuthenticate();
    void requestShowKeyboardList();

public slots:
    void setAuthResult(const AuthStatus &status, const QString &result);
    void setAnimationState(const bool start);
    void setAuthStatus(const QString &path);
    void setCapsStatus(const bool isCapsOn);
    void setLimitsInfo(const LimitsInfo &info);
    void setLineEditInfo(const QString &text, const TextType type);
    void setNumLockStatus(const QString &path);
    void setKeyboardButtonInfo(const QString &text);
    void setText(const QString &text);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void init();
    void updateUnlockTime();

private:
    InputType m_inputType;         // 输入设备类型
    AuthType m_authType;           // 认证类型
    DLabel *m_authStatus;          // 认证状态
    DLabel *m_capsStatus;          // 大小写状态
    DLabel *m_numLockStatus;       // 数字键盘状态
    DLabel *m_textLabel;           // 中间文字标签
    DLineEditEx *m_lineEdit;       // 输入框
    DPushButton *m_keyboardButton; // 键盘布局按钮
    LimitsInfo *m_limitsInfo;      // 认证限制信息
    AuthStatus m_status;           // 当前认证的认证状态
    QTimer *m_unlockTimer;         // 账户限制计时器
    uint m_integerMinutes;
    bool m_showPrompt;
    QString m_iconText;
};

#endif // AUTHENTICATIONMODULE_H
