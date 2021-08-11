#include "lockworker.h"
#include "sessionbasemodel.h"

#include <QSignalSpy>
#include <lockframe.h>

#include <gtest/gtest.h>

class UT_LockFrame : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    SessionBaseModel *m_sessionBaseModel;
    LockWorker *m_lockWorker;
    LockFrame *m_lockFrame;
};

void UT_LockFrame::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
    m_lockWorker = new LockWorker(m_sessionBaseModel);
    m_lockFrame = new LockFrame(m_sessionBaseModel);
}

void UT_LockFrame::TearDown()
{
    delete m_sessionBaseModel;
    delete m_lockWorker;
    delete m_lockFrame;
}

TEST_F(UT_LockFrame, init)
{
    m_lockWorker->onDisplayErrorMsg("aaaa");
    m_lockWorker->onDisplayTextInfo("ssssss");
    m_lockWorker->onPasswordResult("ddddd");
    m_lockWorker->initConnections();
//    m_lockWorker->doPowerAction(SessionBaseModel::PowerAction::RequireSuspend);
//    m_lockWorker->doPowerAction(SessionBaseModel::PowerAction::RequireHibernate);
//    m_lockWorker->doPowerAction(SessionBaseModel::PowerAction::RequireRestart);
//    m_lockWorker->doPowerAction(SessionBaseModel::PowerAction::RequireShutdown);
//    m_lockWorker->doPowerAction(SessionBaseModel::PowerAction::RequireLock);
//    m_lockWorker->doPowerAction(SessionBaseModel::PowerAction::RequireLogout);
//    m_lockWorker->doPowerAction(SessionBaseModel::PowerAction::RequireSwitchSystem);
//    m_lockWorker->doPowerAction(SessionBaseModel::PowerAction::RequireSwitchUser);

    std::shared_ptr<User> user_ptr = std::make_shared<NativeUser>("");
    m_lockWorker->setCurrentUser(user_ptr);
    m_lockWorker->authUser("aaa");
    m_lockWorker->createAuthentication("aaa");
    m_lockWorker->destoryAuthentication("aaa");
}

TEST_F(UT_LockFrame, frame)
{
    m_lockFrame->cancelShutdownInhibit();
    m_lockFrame->shutdownInhibit(SessionBaseModel::PowerAction::RequireNormal, false);
    m_lockFrame->handlePoweroffKey();
    m_lockFrame->hideEvent(new QHideEvent);
    m_lockFrame->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_PowerOff, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_NumLock, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_TouchpadOn, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_TouchpadOff, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_TouchpadToggle, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_CapsLock, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_VolumeDown, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_VolumeUp, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_VolumeMute, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_MonBrightnessUp, Qt::NoModifier));
    m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_MonBrightnessDown, Qt::NoModifier));
    m_lockFrame->showUserList();
    m_lockFrame->showShutdown();
    emit m_sessionBaseModel->prepareForSleep(true);
}
