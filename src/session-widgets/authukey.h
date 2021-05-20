#ifndef AUTHUKEY_H
#define AUTHUKEY_H

#include "authmodule.h"

#include <DPushButton>

class DLineEditEx;
DWIDGET_USE_NAMESPACE

class AuthUKey : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthUKey(QWidget *parent = nullptr);
    QString lineEditText() const;

signals:
    void focusChanged(const bool);
    void lineEditTextChanged(const QString &); // 数据同步
    void requestChangeFocus();                 // 切换焦点
    void requestShowKeyboardList();            // 显示键盘布局列表

public slots:
    void setAuthResult(const int status, const QString &result) override;
    void setAnimationState(const bool start) override;
    void setCapsStatus(const bool isCapsOn);
    void setLimitsInfo(const LimitsInfo &info) override;
    void setLineEditInfo(const QString &text, const TextType type);
    void setNumLockStatus(const QString &path);
    void setKeyboardButtonInfo(const QString &text);
    void setKeyboardButtonVisible(const bool visible);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void initConnections();
    void updateUnlockPrompt() override;

private:
    DLabel *m_capsStatus;          // 大小写状态
    DLabel *m_numLockStatus;       // 数字键盘状态
    DLineEditEx *m_lineEdit;       // PIN 输入框
    DPushButton *m_keyboardButton; // 键盘布局按钮
};

#endif // AUTHUKEY_H
