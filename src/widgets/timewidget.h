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

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <com_deepin_daemon_timedate.h>

#include <DFontSizeManager>

#include <QWidget>
#include <QLabel>
#include <QTimer>

using Timedate = com::deepin::daemon::Timedate;

DWIDGET_USE_NAMESPACE

class TimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimeWidget(QWidget *parent = nullptr);
    void set24HourFormat(bool use24HourFormat);
    void updateLocale(const QLocale &locale);

private:
    void refreshTime();

private Q_SLOTS:
    void setWeekdayFormatType(int type);
    void setShortDateFormat(int type);
    void setShortTimeFormat(int type);

private:
    QLabel *m_timeLabel;
    QLabel *m_dateLabel;

    QTimer *m_refreshTimer;
    bool m_use24HourFormat;
    QLocale m_locale;

    Timedate *m_timedateInter;
    QString m_weekdayFormat;
    QString m_shortDateFormat;
    QString m_shortTimeFormat;
    QString m_timeFormat;
};

#endif // TIMEWIDGET_H
