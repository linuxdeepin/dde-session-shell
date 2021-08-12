#include "authpassword.h"

#include <QPaintEvent>
#include <QTest>

#include <gtest/gtest.h>

class UT_AuthPassword : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthPassword *m_widget = nullptr;
};

void UT_AuthPassword::SetUp()
{
    m_widget = new AuthPassword();
}
void UT_AuthPassword::TearDown()
{
    delete m_widget;
}

TEST_F(UT_AuthPassword, init)
{
    m_widget->setLimitsInfo(LimitsInfo());

    QString str;
    for (int i = 0; i < 13; i++) {
        m_widget->setAuthResult(i, str);
    }

    m_widget->setLineEditInfo("", AuthModule::AlertText);
    m_widget->setLineEditInfo("", AuthModule::InputText);
    m_widget->setLineEditInfo("", AuthModule::PlaceHolderText);

    //    m_widget->setNumLockState("");
    m_widget->setKeyboardButtonInfo("");
    m_widget->setKeyboardButtonVisible(true);
    m_widget->lineEditText();
    m_widget->updateUnlockPrompt();
}
