/*
 * Copyright (C) 2018 ~ 2028 Uniontech Technology Co., Ltd.
 *
 * Author:     zorowk <pengwenhao@uniontech.com>
 *
 * Maintainer: zorowk <pengwenhao@uniontech.com>
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

#include <QtTest>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QDBusArgument>

#include <com_deepin_daemon_display.h>

#include "unit_test.h"

UnitTest::UnitTest()
{
    qDBusRegisterMetaType<ScreenRect>();
}

UnitTest::~UnitTest()
{

}

/**
 * @brief UnitTest::authenticate_test  
 */
void UnitTest::authenticate_test()
{

}

QTEST_APPLESS_MAIN(UnitTest)

#include "unit_test.moc"
 
