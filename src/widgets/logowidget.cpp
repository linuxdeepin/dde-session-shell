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

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QTextCodec>
#include <QPalette>
#include <QDebug>
#include <QSettings>
#include <DSysInfo>
#include "public_func.h"
#include "util_updateui.h"

#include "logowidget.h"

DCORE_USE_NAMESPACE

#define PIXMAP_WIDTH 128
#define PIXMAP_HEIGHT 132 /* SessionBaseWindow */

const QPixmap systemLogo(const QSize& size)
{
    return loadPixmap(DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Transparent, ":img/logo.svg"), size);
}

LogoWidget::LogoWidget(QWidget* parent)
    : QFrame(parent)
{
    m_locale = QLocale::system().name();
    initUI();
}

void LogoWidget::initUI() {
//    setFixedSize(240, 40);

    m_logoLabel = new QLabel();

    m_logoLabel->setPixmap(
                []() -> QPixmap {
                    const QPixmap& p = systemLogo(QSize());
                    const bool result = p.width() < PIXMAP_WIDTH && p.height() < PIXMAP_HEIGHT;
                    return result
                    ? p
                    : systemLogo(QSize(PIXMAP_WIDTH, PIXMAP_HEIGHT));
                }());

    m_logoLabel->setObjectName("Logo");
    //修复社区版deepin的显示不全的问题 2020/04/11
    m_logoLabel->setScaledContents(true);

    m_logoVersionLabel = new DLabel;
    m_logoVersionLabel->setObjectName("LogoVersion");
#ifdef SHENWEI_PLATFORM
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::white);
    m_logoVersionLabel->setPalette(pe);
#endif
    this->setObjectName("LogoWidget");

    //设置版本号的默认字体
    QFont font(m_logoVersionLabel->font());
    font.setFamily("Noto Sans CJK SC-Thin");
    m_logoVersionLabel->setFont(font);

    m_logoLayout = new QHBoxLayout;
    m_logoLayout->setMargin(0);
    m_logoLayout->setSpacing(0);
    m_logoLayout->addSpacing(48);
    m_logoLayout->addWidget(m_logoLabel, 0, Qt::AlignBottom);
    m_logoLayout->addWidget(m_logoVersionLabel, 0, Qt::AlignTop);
    m_logoLayout->addStretch();

    setLayout(m_logoLayout);

    QString systemVersion = getVersion();
    m_logoVersionLabel->setText(systemVersion);
    adjustSize();

    updateStyle(":/skin/login.qss", m_logoVersionLabel);
}

QString LogoWidget::getVersion() {
    QString version;
    if (DSysInfo::isDeepin()) {
        version = QString("%1 %2").arg(DSysInfo::majorVersion())
                                  .arg(DSysInfo::uosEditionName(m_locale));
    } else {
        version = QString("%1 %2").arg(DSysInfo::productVersion())
                                  .arg(DSysInfo::productTypeString());
    }

    return version;
}

LogoWidget::~LogoWidget()
{
}

void LogoWidget::updateLocale(const QString &locale)
{
    m_locale = locale;
    m_logoVersionLabel->setText(getVersion());
    adjustSize();
}

void LogoWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    //使用系统名称图标一半高度做为版本信息字体大小
    QFont font(m_logoVersionLabel->font());
    int fontSize = m_logoLabel->height() / 2;
    font.setPixelSize(fontSize);
    m_logoVersionLabel->setFont(font);
}
