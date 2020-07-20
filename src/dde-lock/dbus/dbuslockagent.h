#ifndef DBUSLOCKAGENT_H
#define DBUSLOCKAGENT_H

#include <QObject>

class SessionBaseModel;
class DBusLockAgent : public QObject
{
    Q_OBJECT
public:
    explicit DBusLockAgent(QObject *parent = nullptr);
    void setModel(SessionBaseModel *const model);

    void Show();
    void ShowUserList();
    void ShowAuth(bool active);
    void Suspend(bool enable);
    void Hibernate(bool enable);
    /**
     * @brief 自动切换到此用户的TTY并显示锁屏程序。此功能虽然为域管开发，但非域管情景也可以使用
     * 
     */
    void SwitchTTYAndShow();

private:
    void SwitchTTY();
    void InternalShow();

private:
    SessionBaseModel *m_model;
};

#endif // DBUSLOCKAGENT_H
