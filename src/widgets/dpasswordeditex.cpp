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

#include "dpasswordeditex.h"

#include <QPainter>
#include <QPropertyAnimation>

LoadSlider::LoadSlider(QWidget *parent)
    : QWidget(parent)
    , m_loadSliderColor(Qt::gray)
{
}

void LoadSlider::setLoadSliderColor(const QColor &color)
{
    m_loadSliderColor = color;
    update();
}

void LoadSlider::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QLinearGradient grad(0, height() / 2, width(), height() / 2);
    grad.setColorAt(0.0, Qt::transparent);
    grad.setColorAt(1.0, m_loadSliderColor);
    painter.fillRect(0, 1, width(), height() - 2, grad);

    QWidget::paintEvent(event);
}

DPasswordEditEx::DPasswordEditEx(QWidget *parent)
    : DLineEdit(parent)
{
    //设置密码框文本显示模式
    setEchoMode(QLineEdit::Password);
    //禁用DLineEdit类的删除按钮
    setClearButtonEnabled(false);

    //初始化动画
    m_loadSlider = new LoadSlider(this);
    m_loadSlider->hide();
    m_loadSliderAnim = new QPropertyAnimation(m_loadSlider, "pos", m_loadSlider);
    m_loadSliderAnim->setDuration(1000);
    m_loadSliderAnim->setLoopCount(-1);
    m_loadSliderAnim->setEasingCurve(QEasingCurve::Linear);

    m_loadAnimEnable = true;
    m_isLoading = false;

    connect(this,  &DPasswordEditEx::returnPressed, this, &DPasswordEditEx::inputDone);
}

void DPasswordEditEx::inputDone()
{
    this->hideAlertMessage();

    QString input = this->text();
    if (input.length() > 0) {
        showLoadSlider();
    }
}

void DPasswordEditEx::showLoadSlider()
{
    if (m_loadAnimEnable) {
        if (!m_isLoading) {
            m_isLoading = true;
            m_loadSlider->show();
            m_loadSlider->setGeometry(0, 0, LoadSliderWidth, this->height());
            m_loadSliderAnim->setStartValue(QPoint(0 - LoadSliderWidth, 0));
            m_loadSliderAnim->setEndValue(QPoint(this->width(), 0));
            m_loadSliderAnim->start();
        }
    }
}

void DPasswordEditEx::hideLoadSlider()
{
    if (m_isLoading) {
        m_isLoading = false;
        m_loadSliderAnim->stop();
        m_loadSlider->hide();
    }
}

//重写QLineEdit paintEvent函数，实现当文本设置居中后，holderText仍然显示的需求
void DPasswordEditEx::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    if (hasFocus() && alignment() == Qt::AlignCenter && !placeholderText().isEmpty() && text().isEmpty()) {
        QPainter pa(this);
        QPalette pal = palette();
        QColor col = pal.text().color();
        col.setAlpha(128);
        QPen oldpen = pa.pen();
        pa.setPen(col);
        QTextOption option;
        option.setAlignment(Qt::AlignCenter);
        pa.drawText(rect(), placeholderText(), option);
    }
}
