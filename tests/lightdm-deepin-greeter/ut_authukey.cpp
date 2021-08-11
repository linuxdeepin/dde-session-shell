#include "authukey.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QPaintEvent>

class UT_AuthUKey : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthUKey *m_widget = nullptr;
};

void UT_AuthUKey::SetUp()
{
    m_widget = new AuthUKey();

}
void UT_AuthUKey::TearDown()
{
    delete m_widget;
}


TEST_F(UT_AuthUKey, init)
{
}
