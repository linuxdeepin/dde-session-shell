// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buttonbox.h"

#include <DStyleHelper>
#include <DStyleOption>

#include <QPainter>
#include <QButtonGroup>
#include <QHBoxLayout>

const int IconSize = 26;
const int Radius = 10;

ButtonBoxButton::ButtonBoxButton(const QIcon& icon, const QString &text, QWidget *parent)
    : QAbstractButton(parent)
    , m_isChecked(false)
    , m_radius(0)
    , m_leftRoundEnabled(false)
    , m_rightRoundEnabled(false)
{
    QAbstractButton::setIcon(icon);
    QAbstractButton::setText(text);
    connect(this, &QAbstractButton::toggled, this, [ this ]( bool checked) {
        if (m_isChecked == checked)
            return;

        m_isChecked = checked;
        update();
    });
}

ButtonBoxButton::ButtonBoxButton(DStyle::StandardPixmap iconType, const QString &text, QWidget *parent)
    :QAbstractButton (parent)
{
    QAbstractButton::setIcon(DStyleHelper(style()).standardIcon(iconType, nullptr, this));
    QAbstractButton::setText(text);
}

void ButtonBoxButton::setIcon(const QIcon &icon)
{
    QAbstractButton::setIcon(icon);
}

void ButtonBoxButton::setIcon(DStyle::StandardPixmap iconType)
{
    QAbstractButton::setIcon(DStyleHelper(style()).standardIcon(iconType, nullptr, this));
}

void ButtonBoxButton::setRadius(int radius)
{
    m_radius = radius;
    update();
}

void ButtonBoxButton::setLeftRoundedEnabled(bool enabled)
{
    if (m_leftRoundEnabled == enabled)
        return;

    m_leftRoundEnabled = enabled;
    update();
}

void ButtonBoxButton::setRightRoundedEnabled(bool enabled)
{
    if (m_rightRoundEnabled == enabled)
        return;

    m_rightRoundEnabled = enabled;
    update();
}

void ButtonBoxButton::setChecked(bool checked)
{
    if (m_isChecked == checked)
        return;

    m_isChecked = checked;
    update();
}

void ButtonBoxButton::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    QColor color = this->palette().color(QPalette::Button);

    p.setClipping(true);

    QPainterPath path;
    if (m_leftRoundEnabled || m_rightRoundEnabled) {

        // 先绘制圆角矩形
        path.addRoundedRect(rect(), m_radius, m_radius);
        if (m_leftRoundEnabled) {

            // 填充矩形右边两个角的圆角
            path.addRect(width() - m_radius, 0, m_radius, m_radius);                       // 填充右上角
            path.addRect(width() - m_radius, height() - m_radius, m_radius, m_radius);     // 填充右下角
        } else {

            // 填充矩形左边两个角的圆角
            path.addRect(0, height() - m_radius, m_radius, m_radius);                      // 填充左上角
            path.addRect(0, 0, m_radius, m_radius);                                        // 填充左下角
        }
    } else {
        path.addRect(rect());
    }
    path.setFillRule(Qt::WindingFill);

    p.setClipPath(path);

    QStyleOption opt;
    opt.initFrom(this);
    if (opt.state & QStyle::StateFlag::State_MouseOver) {
        color.setAlphaF(0.1);
        p.setBrush(color);
    }

    if (m_isChecked) {
        p.setBrush(this->palette().highlight());
    }

    p.drawRect(this->rect());
    p.setClipping(false);

    //绘制图标
    int x = this->rect().x() + rect().width() / 2 - IconSize / 2;
    int y = this->rect().y() + rect().height() / 2 - IconSize / 2;
    p.drawPixmap(x, y, icon().pixmap(IconSize, IconSize));
    QWidget::paintEvent(event);
}

//ButtonBox类
ButtonBox::ButtonBox(QWidget *parent)
    : QWidget(parent)
    , m_group(new QButtonGroup(this))
    , m_layout(new QHBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
}

void ButtonBox::setButtonList(const QList<ButtonBoxButton *> &list, bool checkable)
{
    if(list.isEmpty())
        return;

    for (QAbstractButton *button : m_group->buttons()) {
        m_group->removeButton(button);
        m_layout->removeWidget(button);
    }

    for (int i = 0; i < list.count(); ++i) {
        ButtonBoxButton *button = list.at(i);
        if (!button)
            continue;

        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_layout->addWidget(button);
        m_layout->setSpacing(0);
        m_group->addButton(button);
        button->setCheckable(checkable);
    }

    list.first()->setLeftRoundedEnabled(true);
    list.last()->setRightRoundedEnabled(true);
    list.first()->setRadius(Radius);
    list.last()->setRadius(Radius);
}

void ButtonBox::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(palette().color(QPalette::Button));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(this->rect(), Radius, Radius);
    QWidget::paintEvent(event);
}
