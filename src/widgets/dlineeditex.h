/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zorowk <near.kingzero@gmail.com>
 *
 * Maintainer: zorowk <near.kingzero@gmail.com>
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

#ifndef DLINEEDITEX_H
#define DLINEEDITEX_H

#include <DLineEdit>

DWIDGET_USE_NAMESPACE

class QVariantAnimation;
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

class DLineEditEx : public DLineEdit
{
    Q_OBJECT
public:
    DLineEditEx(QWidget *parent = nullptr);

public slots:
    void startAnimation();
    void stopAnimation();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initAnimation();

private:
    LoadSlider *m_loadSlider;
    QPropertyAnimation *m_animation;
};

#endif // DLINEEDITEX_H
