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

DPasswordEditEx::DPasswordEditEx(QWidget *parent)
    : DLineEdit(parent)
{
    //设置密码框文本显示模式
    setEchoMode(QLineEdit::Password);
    //禁用DLineEdit类的删除按钮
    setClearButtonEnabled(false);
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
