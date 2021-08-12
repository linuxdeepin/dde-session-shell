#include "rounditembutton.h"
#include "shutdownwidget.h"

#include <DIconButton>

#include <QSignalSpy>
#include <QTest>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

class UT_ShutdownWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModels;
    ShutdownWidget *m_ShutdownWidget;
};

void UT_ShutdownWidget::SetUp()
{
    m_sessionBaseModels = new SessionBaseModel(SessionBaseModel::AuthType::LightdmType);
    m_ShutdownWidget = new ShutdownWidget();
    m_ShutdownWidget->setModel(m_sessionBaseModels);
}

void UT_ShutdownWidget::TearDown()
{
    delete m_sessionBaseModels;
    delete m_ShutdownWidget;
}

TEST_F(UT_ShutdownWidget, buttonClicked)
{
    RoundItemButton item;
    RoundItemButton *button;
    QSignalSpy spy(&item, SIGNAL(clicked()));
    button = m_ShutdownWidget->findChild<RoundItemButton *>("RequireShutDownButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));

    button = m_ShutdownWidget->findChild<RoundItemButton *>("RequireRestartButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));

    button = m_ShutdownWidget->findChild<RoundItemButton *>("RequireSuspendButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));

    button = m_ShutdownWidget->findChild<RoundItemButton *>("RequireHibernateButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));

    button = m_ShutdownWidget->findChild<RoundItemButton *>("RequireLockButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));

    button = m_ShutdownWidget->findChild<RoundItemButton *>("RequireLogoutButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));

    button = m_ShutdownWidget->findChild<RoundItemButton *>("RequireSwitchUserButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
}
