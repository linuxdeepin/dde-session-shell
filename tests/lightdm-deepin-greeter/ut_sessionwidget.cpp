#include <gtest/gtest.h>

#include <QTest>
#include <QLabel>

#include "sessionwidget.h"

class TstSessionWidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_sessionwidget = new SessionWidget();
    }
    void TearDown() override
    {
        delete m_sessionwidget;
        m_sessionwidget = nullptr;
    }
public:
    SessionWidget *m_sessionwidget = nullptr;
};

TEST_F(TstSessionWidget, coverage_main)
{
//    ASSERT_NE(m_sessionwidget, nullptr);
    m_sessionwidget->show();
    m_sessionwidget->sessionCount();
//    m_sessionwidget->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier));
//    m_sessionwidget->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier));
//    m_sessionwidget->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier));
//    m_sessionwidget->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier));

}

