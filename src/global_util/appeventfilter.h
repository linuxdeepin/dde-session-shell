#ifndef APPEVENTFILTER_H
#define APPEVENTFILTER_H

#include <QObject>
#include <QEvent>

class AppEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit AppEventFilter(QObject *parent = nullptr);

    virtual bool eventFilter(QObject *watched, QEvent *event);
signals:
    void userIsActive();
};

#endif // APPEVENTFILTER_H
