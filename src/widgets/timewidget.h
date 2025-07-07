// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <DFontSizeManager>

#include <QWidget>
#include <QLabel>
#include <QTimer>

DWIDGET_USE_NAMESPACE

class TimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimeWidget(QWidget *parent = nullptr);
    inline bool get24HourFormat() const { return m_use24HourFormat; }
    void set24HourFormat(bool use24HourFormat);
    void updateLocale(const QLocale &locale);
    QSize sizeHint() const override;

#ifdef ENABLE_DSS_SNIPE
    void updateLocale(const QString &locale, const QString &shortTimeFormat = "", const QString &shortDateFormat = "");
#endif // ENABLE_DSS_SNIPE

public Q_SLOTS:
    void setWeekdayFormatType(int type);
    void setShortDateFormat(int type);
    void setShortTimeFormat(int type);

private:
    void refreshTime();

private:
    QLabel *m_timeLabel;
    QLabel *m_dateLabel;

    QTimer *m_refreshTimer;
    bool m_use24HourFormat;
    QLocale m_locale;

    int m_weekdayIndex = 0;
    int m_shortDateIndex = 0;
    int m_shortTimeIndex = 0;

#ifdef ENABLE_DSS_SNIPE
    QString m_shortTimeFormat;
    QString m_shortDateFormat;
#endif // ENABLE_DSS_SNIPE
};

#endif // TIMEWIDGET_H
