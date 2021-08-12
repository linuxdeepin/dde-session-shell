#include "hibernatewidget.h"

#include <QPaintEvent>
#include <QTest>

#include <gtest/gtest.h>

class UT_HibernateWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    HibernateWidget *m_widget = nullptr;
};

void UT_HibernateWidget::SetUp()
{
    m_widget = new HibernateWidget();
}
void UT_HibernateWidget::TearDown()
{
    delete m_widget;
}

TEST_F(UT_HibernateWidget, init)
{
    // m_widget->paintEvent(new QPaintEvent(QRect()));
}
