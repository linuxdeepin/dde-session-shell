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

#ifndef AUTHSINGLE_H
#define AUTHSINGLE_H

#include "auth_module.h"

#include <DIconButton>
#include <DPushButton>

class DLineEditEx;
class AuthSingle : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthSingle(QWidget *parent = nullptr);

    void reset();
    QString lineEditText() const;

    void setAnimationStatus(const bool start) override;
    void setAuthStatus(const int status, const QString &result) override;
    void setCapsLockVisible(const bool on);
    void setKeyboardButtonInfo(const QString &text);
    void setKeyboardButtonVisible(const bool visible);
    void setLimitsInfo(const LimitsInfo &info) override;
    void setLineEditEnabled(const bool enable);
    void setLineEditInfo(const QString &text, const TextType type);
    void setPasswordHint(const QString &hint);

signals:
    void focusChanged(const bool);
    void lineEditTextChanged(const QString &); // 数据同步
    void requestShowKeyboardList();            // 显示键盘布局列表

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void initConnections();

    void updateUnlockPrompt();
    void showPasswordHint();
    void setPasswordHintBtnVisible(const bool isVisible);

private:
    DLabel *m_capsLock;             // 大小写状态
    DLineEditEx *m_lineEdit;        // 输入框
    DPushButton *m_keyboardBtn;     // 键盘布局按钮
    DIconButton *m_passwordHintBtn; // 密码提示按钮
    QString m_passwordHint;         // 密码提示
};

#endif // AUTHSINGLE_H
