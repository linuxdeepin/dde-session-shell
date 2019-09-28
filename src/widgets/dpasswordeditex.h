/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lixin <lixin_cm@deepin.com>
 *
 * Maintainer: lixin <lixin_cm@deepin.com>
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

#ifndef DPASSWORDEDITEX_H
#define DPASSWORDEDITEX_H

#include <DLineEdit>
#include <DClipEffectWidget>

DWIDGET_USE_NAMESPACE

class QPropertyAnimation;
class LoadSlider : public QWidget
{
public:
    LoadSlider(QWidget *parent = nullptr);

public:
    QColor loadSliderColor() const;
    void setLoadSliderColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QColor m_loadSliderColor;
};

class QPushButton;
class DPasswordEditEx : public DLineEdit
{
    Q_OBJECT
public:
    DPasswordEditEx(QWidget *parent = nullptr);
    void addButton();

signals:
    void toggleKBLayoutWidget();

public Q_SLOTS:
    void inputDone();
    void showLoadSlider();
    void hideLoadSlider();
    void capslockStatusChanged(bool on);
    void receiveUserKBLayoutChanged(const QString &layout);
    void setKBLayoutList(QStringList kbLayoutList);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    void initAnimation();
    QImage generateImageFromString(const QString &name);

private:
    LoadSlider *m_loadSlider;
    QPropertyAnimation *m_loadSliderAnim;
    DClipEffectWidget *m_clipEffectWidget;
    QStringList m_KBLayoutList;
    QPushButton *m_KBButton;                           //键盘布局Button
    QPushButton *m_capsButton;                         //大小写锁定Button

    bool m_loadAnimEnable;
    bool m_isLoading;
    const int LoadSliderWidth = 40;
};

#endif // DPASSWORDEDITEX_H
