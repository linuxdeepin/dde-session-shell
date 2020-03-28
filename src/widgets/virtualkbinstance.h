#ifndef VIRTUALKBINSTANCE_H
#define VIRTUALKBINSTANCE_H

#include <functional>

#include <QWidget>
#include <QObject>


class VirtualKBInstance : public QObject
{
    Q_OBJECT
public:
    static VirtualKBInstance &Instance();
    QWidget *virtualKBWidget();

    void init(QWidget *parent = nullptr);

    void debug(const QString &type, const QString  &info);

signals:
    void initFinished();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    VirtualKBInstance(QObject *parent = nullptr);
    VirtualKBInstance(const VirtualKBInstance &) = delete;

private:
    QWidget *m_virtualKBWidget;
};

#endif // VIRTUALKBINSTANCE_H
