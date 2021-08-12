#include "dpasswordeditex.h"

#include <gtest/gtest.h>

class UT_DPasswordEditEx : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DPasswordEditEx *m_lineEdit;
};

void UT_DPasswordEditEx::SetUp()
{
    m_lineEdit = new DPasswordEditEx;
}

void UT_DPasswordEditEx::TearDown()
{
    delete m_lineEdit;
}

TEST_F(UT_DPasswordEditEx, BasicTest)
{
    m_lineEdit->inputDone();
    m_lineEdit->showLoadSlider();
    m_lineEdit->hideLoadSlider();
    m_lineEdit->setShowKB(true);
    m_lineEdit->capslockStatusChanged(true);
    m_lineEdit->receiveUserKBLayoutChanged("cn");
    m_lineEdit->setKBLayoutList(QStringList());
    m_lineEdit->onTextChanged("cn");
}
