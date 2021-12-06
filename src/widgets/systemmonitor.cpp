/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#include "systemmonitor.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>

SystemMonitor::SystemMonitor(QWidget *parent)
    : QWidget(parent)
    , m_icon(new QLabel(this))
    , m_text(new QLabel(this))
    , m_state(Leave)
{
    setObjectName(QStringLiteral("SystemMonitor"));
    setAccessibleName(QStringLiteral("SystemMonitor"));

    setMinimumHeight(40);

    initUI();
}

void SystemMonitor::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 0, 10, 0);

    m_icon->setPixmap(QPixmap(":/img/deepin-system-monitor.svg"));
    m_text->setText(tr("Start system monitor"));

    mainLayout->addWidget(m_icon, 0, Qt::AlignVCenter | Qt::AlignRight);
    mainLayout->addWidget(m_text, 1, Qt::AlignVCenter | Qt::AlignLeft);
}

void SystemMonitor::setState(const State state)
{
    m_state = state;
    update();
}

void SystemMonitor::enterEvent(QEvent *event)
{
    m_state = Enter;
    update();

    QWidget::enterEvent(event);
}

void SystemMonitor::leaveEvent(QEvent *event)
{
    m_state = Leave;
    update();

    QWidget::leaveEvent(event);
}

void SystemMonitor::mouseReleaseEvent(QMouseEvent *event)
{
    m_state = Release;
    emit requestShowSystemMonitor();
    update();

    QWidget::mouseReleaseEvent(event);
}

void SystemMonitor::mousePressEvent(QMouseEvent *event)
{
    m_state = Press;
    update();

    QWidget::mousePressEvent(event);
}

void SystemMonitor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        m_state = Press;
    }
    update();

    QWidget::keyReleaseEvent(event);
}

void SystemMonitor::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit requestShowSystemMonitor();
        m_state = Release;
    }
    update();

    QWidget::keyReleaseEvent(event);
}

void SystemMonitor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);

    switch (m_state) {
    case Enter:
        painter.setBrush(QColor(0, 0, 0, 75));
        break;
    case Leave:
        painter.setBrush(QColor(0, 0, 0, 0));
        break;
    case Release:
        painter.setBrush(QColor(0, 0, 0, 75));
        break;
    case Press:
        painter.setBrush(QColor(0, 0, 0, 105));
        break;
    }
    painter.drawRoundedRect(QRect(1, 1, width() - 2, height() - 2), 10, 10);

    QWidget::paintEvent(event);
}
