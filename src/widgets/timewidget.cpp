/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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

#include "timewidget.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QFontDatabase>
#include <QSettings>

TimeWidget::TimeWidget(QWidget *parent)
    : QWidget(parent)
    , m_timedateInter(new Timedate("com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate", QDBusConnection::sessionBus(), this))
    , m_weekdayFormat("dddd")
    , m_shortDateFormat("yyyy-MM-dd")
    , m_shortTimeFormat("hh:mm")
    , m_timeFormat("yyyy-MM-dd dddd")
{
    QFont timeFont;
    timeFont.setFamily("Noto Sans CJK SC-Thin");

    m_timeLabel = new QLabel;
    timeFont.setWeight(QFont::ExtraLight);
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    QPalette palette = m_timeLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_timeLabel->setPalette(palette);
    DFontSizeManager::instance()->bind(m_timeLabel, DFontSizeManager::T1);

    m_dateLabel = new QLabel;
    m_dateLabel->setAlignment(Qt::AlignCenter);
    palette = m_dateLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_dateLabel->setPalette(palette);
    DFontSizeManager::instance()->bind(m_dateLabel, DFontSizeManager::T6);

    refreshTime();

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000);
    m_refreshTimer->start();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_timeLabel);
    vLayout->addWidget(m_dateLabel);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(vLayout);

    connect(m_refreshTimer, &QTimer::timeout, this, &TimeWidget::refreshTime);

    connect(m_timedateInter, &Timedate::WeekdayFormatChanged, this, &TimeWidget::setWeekdayFormatType);
    connect(m_timedateInter, &Timedate::ShortDateFormatChanged, this, &TimeWidget::setShortDateFormat);
    connect(m_timedateInter, &Timedate::ShortTimeFormatChanged, this, &TimeWidget::setShortTimeFormat);

    setWeekdayFormatType(m_timedateInter->weekdayFormat());
    setShortDateFormat(m_timedateInter->shortDateFormat());
    setShortTimeFormat(m_timedateInter->shortTimeFormat());
}

void TimeWidget::set24HourFormat(bool use24HourFormat)
{
    m_use24HourFormat = use24HourFormat;
    refreshTime();
}

void TimeWidget::updateLocale(const QLocale &locale)
{
    m_locale = locale;
    refreshTime();
}

void TimeWidget::refreshTime()
{
    if (m_use24HourFormat) {
        m_timeLabel->setText(QDateTime::currentDateTime().toString(m_shortTimeFormat));
    } else {
        QString format = m_shortTimeFormat;
        format.append(" AP");
        m_timeLabel->setText(QDateTime::currentDateTime().toString(format));
    }

    m_dateLabel->setText(m_locale.toString(QDateTime::currentDateTime(), m_timeFormat));
}

/**
 * @brief TimeWidget::setWeekdayFormatType 根据类型来设置周显示格式
 * @param type 自定义类型
 */
void TimeWidget::setWeekdayFormatType(int type)
{
    switch (type) {
    case 0: m_weekdayFormat = "dddd";  break;
    case 1: m_weekdayFormat = "ddd"; break;
    default: m_weekdayFormat = "dddd"; break;
    }
    m_timeFormat = m_shortDateFormat.append(" ");
    m_timeFormat.append(m_weekdayFormat);
    refreshTime();
}

/**
 * @brief TimeWidget::setShortDateFormat 根据类型来设置短日期显示格式
 * @param type 自定义格式
 */
void TimeWidget::setShortDateFormat(int type)
{
    switch (type) {
    case 0: m_shortDateFormat = "yyyy/M/d";  break;
    case 1: m_shortDateFormat = "yyyy-M-d"; break;
    case 2: m_shortDateFormat = "yyyy.M.d"; break;
    case 3: m_shortDateFormat = "yyyy/MM/dd"; break;
    case 4: m_shortDateFormat = "yyyy-MM-dd"; break;
    case 5: m_shortDateFormat = "yyyy.MM.dd"; break;
    case 6: m_shortDateFormat = "yy/M/d"; break;
    case 7: m_shortDateFormat = "yy-M-d"; break;
    case 8: m_shortDateFormat = "yy.M.d"; break;
    default: m_shortDateFormat = "yyyy-MM-dd"; break;
    }
    m_timeFormat = m_shortDateFormat.append(" ");
    m_timeFormat.append(m_weekdayFormat);
    refreshTime();
}

/**
 * @brief TimeWidget::setShortTimeFormat 根据类型来设置短时间显示格式
 * @param type
 */
void TimeWidget::setShortTimeFormat(int type)
{
    switch (type) {
    case 0: m_shortTimeFormat = "h:m"; break;
    case 1: m_shortTimeFormat = "hh:mm";  break;
    case 2: m_shortTimeFormat = "h:m"; break;
    case 3: m_shortTimeFormat = "hh:mm";  break;
    default: m_shortTimeFormat = "hh:mm"; break;
    }
    refreshTime();
}
