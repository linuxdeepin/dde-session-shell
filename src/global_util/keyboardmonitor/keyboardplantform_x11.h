// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBOARDPLANTFORM_X11_H
#define KEYBOARDPLANTFORM_X11_H

#include "keyboardplatform.h"

typedef struct _XDisplay Display;

class KeyboardPlatformX11 : public KeyBoardPlatform
{
    Q_OBJECT
public:
    KeyboardPlatformX11(QObject *parent = nullptr);

    bool isCapsLockOn() override;
    bool isNumLockOn() override;
    bool setNumLockStatus(const bool &on) override;
    void run() override;

private:
    int listen(Display *display);
    static int xinput_version(Display *display);
    static void select_events(Display* display);
};

#endif // KEYBOARDPLANTFORM_X11_H
