// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DLINEEDITEX_H
#define DLINEEDITEX_H

#include <DLineEdit>
#include <QEvent>

DWIDGET_USE_NAMESPACE

class QVariantAnimation;
class QPropertyAnimation;
class LoadSlider : public QWidget
{
public:
    LoadSlider(QWidget *parent = nullptr);

public:
    void setLoadSliderColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QColor m_loadSliderColor;
};

class DLineEditEx : public DLineEdit
{
    Q_OBJECT
public:
    DLineEditEx(QWidget *parent = nullptr);

public slots:
    void startAnimation();
    void stopAnimation();
    void setEnableTableKeyEvent(bool enable);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initAnimation();
    void setPlaceholderTextFont(const QFont &font);

private:
    LoadSlider *m_loadSlider;
    QPropertyAnimation *m_animation;
    bool m_enableTableKeyEvent;
};

#endif // DLINEEDITEX_H
