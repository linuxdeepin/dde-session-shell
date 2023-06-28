// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#include "backgroundhandlerThread.h"
#include "public_func.h"

#include <QImageReader>

BackgroundHandlerThread::BackgroundHandlerThread(QObject *parent)
    : QThread(parent)
{

}

void BackgroundHandlerThread::setBackgroundInfo(const QString &path, const QSize &size, const qreal &devicePixelRatioF)
{
    m_path = path;
    m_targetSize = size;
    m_devicePixelRatioF = devicePixelRatioF;
}

void BackgroundHandlerThread::run()
{
    auto pixmap = handleBackground(m_path, m_targetSize, m_devicePixelRatioF);
    emit backgroundHandled(pixmap);
}

QPixmap BackgroundHandlerThread::handleBackground(const QString &path, const QSize &size, const qreal &devicePixelRatioF)
{
    QPixmap pixmap;
    loadPixmap(path, pixmap);

    pixmap = pixmap.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    pixmap = pixmap.copy(QRect((pixmap.width() - size.width()) / 2,
                               (pixmap.height() - size.height()) / 2,
                               size.width(),
                               size.height()));

    // draw pix to widget, so pix need set pixel ratio from qwidget devicepixelratioF
    pixmap.setDevicePixelRatio(devicePixelRatioF);

    return pixmap;
}