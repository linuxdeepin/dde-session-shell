// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transparentbutton.h"

#include <DStyleOptionButton>

TransparentButton::TransparentButton(QWidget *parent)
    : DFloatingButton (parent)
{

};

void TransparentButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // 按钮不可用时颜色还是使用活动色，然后需要40%透明
    DStylePainter p(this);
    DStyleOptionButton opt;
    initStyleOption(&opt);
    if (isEnabled()) {
        p.setOpacity(1.0);
    } else {
        p.setOpacity(0.4);
    }
    p.drawControl(DStyle::CE_IconButton, opt);
};
