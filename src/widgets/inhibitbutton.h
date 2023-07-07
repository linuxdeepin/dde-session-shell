// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INHIBITBUTTON_H
#define INHIBITBUTTON_H

#include <QWidget>
#include <QIcon>

class QLabel;
class InhibitButton : public QWidget
{
    Q_OBJECT
public:
    enum State {
        Enter = 0X0,
        Leave = 0x1,
        Press = 0x2,
        Release = 0x4
    };
    InhibitButton(QWidget *parent);
    ~InhibitButton();

    void setText(const QString &text);
    QString text();
    void setNormalPixmap(const QPixmap &normalPixmap);
    QPixmap normalPixmap();
    void setHoverPixmap(const QPixmap &hoverPixmap);
    QPixmap hoverPixmap();

Q_SIGNALS:
    void clicked();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    State m_state;
    QLabel *m_iconLabel;
    QLabel *m_textLabel;
    QPixmap m_normalPixmap;
    QPixmap m_hoverPixmap;
};

#endif // INHIBITBUTTON_H
