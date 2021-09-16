#include "kblayoutwidget.h"

#include <QTest>

#include <gtest/gtest.h>

class UT_LayoutButton : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    LayoutButton *m_layoutButton;
};

class UT_KbLayoutWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    KbLayoutWidget *m_kbLayoutWidget;
};

void UT_LayoutButton::SetUp()
{
    m_layoutButton = new LayoutButton("");
}

void UT_LayoutButton::TearDown()
{
    delete m_layoutButton;
}

TEST_F(UT_LayoutButton, BasicTest)
{
    m_layoutButton->updateStyleSelf();
    m_layoutButton->OnlyMeChecked(false);
    m_layoutButton->setButtonChecked(true);
}

void UT_KbLayoutWidget::SetUp()
{
    m_kbLayoutWidget = new KbLayoutWidget();
}

void UT_KbLayoutWidget::TearDown()
{
    delete m_kbLayoutWidget;
}

TEST_F(UT_KbLayoutWidget, BasicTest)
{
    // m_kbLayoutWidget->setButtonsChecked("");
    m_kbLayoutWidget->setListItemChecked(0);
    m_kbLayoutWidget->updateButtonList(QStringList());
    m_kbLayoutWidget->setDefault("cn;");
}
