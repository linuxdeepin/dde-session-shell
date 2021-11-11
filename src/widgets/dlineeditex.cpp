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

#include "dlineeditex.h"

#include <QPainter>
#include <QVariantAnimation>
#include <QPropertyAnimation>

LoadSlider::LoadSlider(QWidget *parent)
    : QWidget(parent)
    , m_loadSliderColor(Qt::gray)
{
}

void LoadSlider::setLoadSliderColor(const QColor &color)
{
    m_loadSliderColor = color;
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

DLineEditEx::DLineEditEx(QWidget *parent)
    : DLineEdit(parent)
    , m_loadSlider(new LoadSlider(this))
    , m_animation(new QPropertyAnimation(m_loadSlider, "pos", m_loadSlider))
{
    setObjectName(QStringLiteral("DLineEditEx"));
    setAccessibleName(QStringLiteral("DLineEditEx"));

    initAnimation();

#if 0 // FIXME:不生效,改为在eventFilter过滤InputMethodQuery事件
    // 在锁屏或登录界面，输入类型的控件不要唤出输入法
    setAttribute(Qt::WA_InputMethodEnabled, false);
    this->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
#endif
    this->lineEdit()->installEventFilter(this);
    this->installEventFilter(this);
}

/**
 * @brief 初始化动画
 */
void DLineEditEx::initAnimation()
{
    m_loadSlider->setGeometry(0, -40, 40, height());
    m_loadSlider->setLoadSliderColor(QColor(255, 255, 255, 90));
    m_loadSlider->setAccessibleName("LoadSlider");
    m_animation->setDuration(1000);
    m_animation->setLoopCount(-1);
    m_animation->setEasingCurve(QEasingCurve::Linear);
    m_animation->targetObject();
}

/**
 * @brief 显示动画
 */
void DLineEditEx::startAnimation()
{
    if (m_animation->state() == QAbstractAnimation::Running) {
        return;
    }
    m_loadSlider->show();
    m_loadSlider->resize(40, height());
    m_animation->setStartValue(QPoint(0 - 40, 0));
    m_animation->setEndValue(QPoint(width(), 0));
    m_animation->start();
}

/**
 * @brief 隐藏动画
 */
void DLineEditEx::stopAnimation()
{
    if (m_animation->state() == QAbstractAnimation::Stopped) {
        return;
    }
    m_loadSlider->hide();
    m_animation->stop();
}

/**
 * @brief 重写 QLineEdit paintEvent 函数，实现当文本设置居中后，holderText 仍然显示的需求
 *
 * @param event
 */
void DLineEditEx::paintEvent(QPaintEvent *event)
{
    if (lineEdit()->hasFocus() && lineEdit()->alignment() == Qt::AlignCenter
        && !lineEdit()->placeholderText().isEmpty() && lineEdit()->text().isEmpty()) {
        QPainter pa(this);
        QPalette pal = palette();
        QColor col = pal.text().color();
        col.setAlpha(128);
        QPen oldpen = pa.pen();
        pa.setPen(col);
        QTextOption option;
        option.setAlignment(Qt::AlignCenter);
        pa.drawText(lineEdit()->rect(), lineEdit()->placeholderText(), option);
    }
    QWidget::paintEvent(event);
}

bool DLineEditEx::eventFilter(QObject *watched, QEvent *event)
{
    // 禁止输入法
    if ((watched == this || watched == this->lineEdit())
            && event->type() == QEvent::InputMethodQuery) {
        return true;
    }

    return DLineEdit::eventFilter(watched,event);
}
