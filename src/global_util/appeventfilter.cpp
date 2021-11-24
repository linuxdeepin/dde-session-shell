/*
 * Copyright (C) 2018 ~ 2021 Uniontech Technology Co., Ltd.
 *
 * Author:     fanpengcheng <fanpengcheng@uniontech.com.so>
 *
 * Maintainer: fanpengcheng <fanpengcheng@uniontech.com.so>
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
#include "appeventfilter.h"

AppEventFilter::AppEventFilter(QObject *parent) : QObject(parent)
{

}

bool AppEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    //如果有鼠标或者键盘事件,则认为用户处于活跃状态
    if (event->type() >= QEvent::MouseButtonPress && event->type() <= QEvent::KeyRelease){
        Q_EMIT userIsActive();
    }

    return false;
}
