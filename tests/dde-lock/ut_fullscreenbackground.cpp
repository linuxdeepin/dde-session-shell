#include "fullscreenbackground.h"

#include <gtest/gtest.h>

class UT_FullscreenBackground : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    FullscreenBackground *m_background;
};

void UT_FullscreenBackground::SetUp()
{
    m_background = new FullscreenBackground();
}

void UT_FullscreenBackground::TearDown()
{
    delete m_background;
}

TEST_F(UT_FullscreenBackground, BasicTest)
{
    m_background->setContentVisible(true);
    m_background->contentVisible();

    // m_background->setIsBlackMode(false);
    // m_background->setIsBlackMode(true);

    // m_background->setIsHibernateMode();

    m_background->enableEnterEvent(true);

    m_background->updateBackground("/usr/share/backgrounds/default_background.jpg");
    m_background->updateBlurBackground("/usr/share/backgrounds/default_background.jpg");
}
