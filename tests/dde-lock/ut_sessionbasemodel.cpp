#include "sessionbasemodel.h"
#include "userinfo.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_SessionBaseModel : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModel;
};

void UT_SessionBaseModel::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
}

void UT_SessionBaseModel::TearDown()
{
    delete m_sessionBaseModel;
}

TEST_F(UT_SessionBaseModel, buttonClicked)
{
    std::shared_ptr<NativeUser> nativeUser(new NativeUser("/com/deepin/daemon/Accounts/User"+QString::number((getuid()))));
    ASSERT_TRUE(m_sessionBaseModel);
    EXPECT_EQ(m_sessionBaseModel->currentType(), SessionBaseModel::AuthType::LockType);
    m_sessionBaseModel->setCurrentUser(nativeUser);
    EXPECT_EQ(m_sessionBaseModel->currentUser(), nativeUser);

    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::None);
    m_sessionBaseModel->setPowerAction(SessionBaseModel::PowerAction::RequireNormal);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), SessionBaseModel::PowerAction::RequireNormal);

    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::None);
    m_sessionBaseModel->setPowerAction(SessionBaseModel::PowerAction::RequireShutdown);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), SessionBaseModel::PowerAction::RequireShutdown);

    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::None);
    m_sessionBaseModel->setPowerAction(SessionBaseModel::PowerAction::RequireRestart);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), SessionBaseModel::PowerAction::RequireRestart);

    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::None);
    m_sessionBaseModel->setPowerAction(SessionBaseModel::PowerAction::RequireSuspend);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), SessionBaseModel::PowerAction::RequireSuspend);

    m_sessionBaseModel->onPowerActionChanged(SessionBaseModel::PowerAction::None);
    m_sessionBaseModel->setPowerAction(SessionBaseModel::PowerAction::RequireHibernate);
    EXPECT_EQ(m_sessionBaseModel->powerAction(), SessionBaseModel::PowerAction::RequireHibernate);


    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::NoStatus);
    m_sessionBaseModel->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), SessionBaseModel::ModeStatus::PasswordMode);

    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::NoStatus);
    m_sessionBaseModel->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), SessionBaseModel::ModeStatus::ConfirmPasswordMode);

    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::NoStatus);
    m_sessionBaseModel->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), SessionBaseModel::ModeStatus::UserMode);

    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::NoStatus);
    m_sessionBaseModel->setCurrentModeState(SessionBaseModel::ModeStatus::SessionMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), SessionBaseModel::ModeStatus::SessionMode);

    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::NoStatus);
    m_sessionBaseModel->setCurrentModeState(SessionBaseModel::ModeStatus::PowerMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), SessionBaseModel::ModeStatus::PowerMode);

    m_sessionBaseModel->onStatusChanged(SessionBaseModel::ModeStatus::NoStatus);
    m_sessionBaseModel->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    EXPECT_EQ(m_sessionBaseModel->currentModeState(), SessionBaseModel::ModeStatus::ShutDownMode);

    bool ishasSwap = m_sessionBaseModel->hasSwap();
    m_sessionBaseModel->onHasSwapChanged(!ishasSwap);
    m_sessionBaseModel->setHasSwap(!ishasSwap);
    EXPECT_EQ(m_sessionBaseModel->hasSwap(), !ishasSwap);

    bool isshow = m_sessionBaseModel->isShow();
    m_sessionBaseModel->visibleChanged(!isshow);
    m_sessionBaseModel->setIsShow(!isshow);
    EXPECT_EQ(m_sessionBaseModel->isShow(), !isshow);

    bool sleep = m_sessionBaseModel->canSleep();
    m_sessionBaseModel->canSleepChanged(!sleep);
    m_sessionBaseModel->setCanSleep(!sleep);
    EXPECT_EQ(m_sessionBaseModel->canSleep(), !sleep);

    bool UserSwitchButton = m_sessionBaseModel->allowShowUserSwitchButton();
    m_sessionBaseModel->setAllowShowUserSwitchButton(!UserSwitchButton);
    m_sessionBaseModel->allowShowUserSwitchButtonChanged(!UserSwitchButton);
    EXPECT_EQ(m_sessionBaseModel->allowShowUserSwitchButton(), !UserSwitchButton);

    bool alwaysUserSwitchButton = m_sessionBaseModel->alwaysShowUserSwitchButton();
    m_sessionBaseModel->setAlwaysShowUserSwitchButton(!alwaysUserSwitchButton);
    EXPECT_EQ(m_sessionBaseModel->alwaysShowUserSwitchButton(), !alwaysUserSwitchButton);

    bool isServer = m_sessionBaseModel->isServerModel();
    m_sessionBaseModel->setIsServerModel(!isServer);
    EXPECT_EQ(m_sessionBaseModel->isServerModel(), !isServer);

    bool isLock = m_sessionBaseModel->isLockNoPassword();
    m_sessionBaseModel->setIsLockNoPassword(!isLock);
    EXPECT_EQ(m_sessionBaseModel->isLockNoPassword(), !isLock);

    bool isBlack = m_sessionBaseModel->isBlackMode();
    m_sessionBaseModel->setIsBlackModel(!isBlack);
    EXPECT_EQ(m_sessionBaseModel->isBlackMode(), !isBlack);

    bool isHibernate = m_sessionBaseModel->isHibernateMode();
    m_sessionBaseModel->setIsHibernateModel(!isHibernate);
    m_sessionBaseModel->HibernateModeChanged(!isHibernate);
    EXPECT_EQ(m_sessionBaseModel->isHibernateMode(), !isHibernate);

    int listSize = m_sessionBaseModel->userListSize();
    m_sessionBaseModel->onUserListSizeChanged(++listSize);
    m_sessionBaseModel->setUserListSize(++listSize);
    EXPECT_EQ(m_sessionBaseModel->userListSize(), listSize);

    m_sessionBaseModel->setSessionKey("AAAAAA");
    m_sessionBaseModel->setHasVirtualKB(true);
    m_sessionBaseModel->setHasVirtualKB(false);
}
