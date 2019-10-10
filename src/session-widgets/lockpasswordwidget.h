#ifndef LOCKPASSWORDWIDGET_H
#define LOCKPASSWORDWIDGET_H

#include <QWidget>
#include <QLabel>

class LockPasswordWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LockPasswordWidget(QWidget *parent = nullptr);

    void setMessage(const QString &message);
    void setLockIconVisible(bool on) { lockLbl->setVisible(on); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_message;
    QLabel *lockLbl;
};

#endif // LOCKPASSWORDWIDGET_H
