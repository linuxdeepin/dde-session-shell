/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *             zorowk <near.kingzero@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *             zorowk <near.kingzero@gmail.com>
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

#ifndef KEYBOARDPLANTFORM_X11_H
#define KEYBOARDPLANTFORM_X11_H

#include "keyboardplatform.h"

typedef struct _XDisplay Display;

class KeyboardPlantformX11 : public KeyBoardPlatform
{
    Q_OBJECT
public:
    KeyboardPlantformX11(QObject *parent = nullptr);

    bool isCapslockOn() override;
    bool isNumlockOn() override;
    bool setNumlockStatus(const bool &on) override;
    void run() override;

private:
    int listen(Display *display);
    static int xinput_version(Display *display);
    static void select_events(Display* display);
};

#endif // KEYBOARDPLANTFORM_X11_H
