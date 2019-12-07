/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lixin <lixin_cm@deepin.com>
 *
 * Maintainer: lixin <lixin_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "userlogininfo.h"
#include "userinfo.h"
#include "userloginwidget.h"
#include "userexpiredwidget.h"
#include "sessionbasemodel.h"
#include "userframelist.h"
#include "src/global_util/constants.h"

UserLoginInfo::UserLoginInfo(SessionBaseModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_userLoginWidget(new UserLoginWidget)
    , m_userExpiredWidget(new UserExpiredWidget)
    , m_userFrameList(new UserFrameList)
{
    m_userLoginWidget->setWidgetWidth(DDESESSIONCC::PASSWDLINEEIDT_WIDTH);
    m_userExpiredWidget->setFixedWidth(DDESESSIONCC::PASSWDLINEEIDT_WIDTH);
    m_userFrameList->setModel(model);
    initConnect();
}

void UserLoginInfo::setUser(std::shared_ptr<User> user)
{
    for (auto connect : m_currentUserConnects) {
        m_user->disconnect(connect);
    }

    m_currentUserConnects.clear();
    m_currentUserConnects << connect(user.get(), &User::lockChanged, this, &UserLoginInfo::userLockChanged);
    m_currentUserConnects << connect(user.get(), &User::avatarChanged, m_userLoginWidget, &UserLoginWidget::setAvatar);
    m_currentUserConnects << connect(user.get(), &User::displayNameChanged, m_userLoginWidget, &UserLoginWidget::setName);
    m_currentUserConnects << connect(user.get(), &User::kbLayoutListChanged, m_userLoginWidget, &UserLoginWidget::updateKBLayout, Qt::UniqueConnection);
    m_currentUserConnects << connect(user.get(), &User::currentKBLayoutChanged, m_userLoginWidget, &UserLoginWidget::setDefaultKBLayout, Qt::UniqueConnection);

    m_user = user;

    m_userLoginWidget->setName(user->displayName());
    m_userLoginWidget->setAvatar(user->avatarPath());
    m_userLoginWidget->setUserAvatarSize(UserLoginWidget::AvatarLargeSize);
    m_userLoginWidget->updateAuthType(m_model->currentType());
    m_userLoginWidget->disablePassword(user.get()->isLock(), user->lockNum());
    m_userLoginWidget->setKBLayoutList(user->kbLayoutList());
    m_userLoginWidget->setDefaultKBLayout(user->currentKBLayout());

    m_userExpiredWidget->setName(user->displayName());
    m_userExpiredWidget->updateAuthType(m_model->currentType());


    if (m_model->isServiceAccountLogin()) {
        m_userLoginWidget->setWidgetShowType(UserLoginWidget::IDAndPasswordType);
    } else {
        if (m_user->isNoPasswdGrp()) {
            m_userLoginWidget->setWidgetShowType(UserLoginWidget::NoPasswordType);
        } else {
            m_userLoginWidget->setWidgetShowType(UserLoginWidget::NormalType);
        }
    }
}

void UserLoginInfo::initConnect()
{
    //UserLoginWidget
    connect(m_userLoginWidget, &UserLoginWidget::requestAuthUser, this, [ = ](const QString & account, const QString & password) {
        if (m_model->errorType() == SessionBaseModel::PasswordExpired && m_model->currentType() == SessionBaseModel::LightdmType) {
            emit passwordExpired();
        } else {
            if (!password.isEmpty()) {
                if (m_model->isServiceAccountLogin() && !account.isEmpty()) {
                    static_cast<ADDomainUser *>(m_model->currentUser().get())->setUserName(account);
                }
                emit requestAuthUser(password);
            }
        }
    });
    connect(m_model, &SessionBaseModel::authFaildMessage, m_userLoginWidget, &UserLoginWidget::setFaildMessage);
    connect(m_model, &SessionBaseModel::authFaildTipsMessage, m_userLoginWidget, &UserLoginWidget::setFaildTipMessage);
    connect(m_model, &SessionBaseModel::authFinished, this, [ = ](bool success) {
        if (success && !m_userLoginWidget.isNull()) {
            m_userLoginWidget->resetAllState();
        }
    });
    connect(m_userLoginWidget, &UserLoginWidget::requestUserKBLayoutChanged, this, [ = ](const QString & value) {
        emit requestSetLayout(m_user, value);
    });
    connect(m_userExpiredWidget, &UserExpiredWidget::changePasswordFinished, this, [ = ] {
        m_userLoginWidget->resetAllState();
        m_model->setErrorType(SessionBaseModel::ErrorType::None);
        emit changePasswordFinished();
    });

    //UserFrameList
    connect(m_userFrameList, &UserFrameList::clicked, this, &UserLoginInfo::hideUserFrameList);
    connect(m_userFrameList, &UserFrameList::requestSwitchUser, this, &UserLoginInfo::receiveSwitchUser);
    connect(m_model, &SessionBaseModel::abortConfirmChanged, this, &UserLoginInfo::abortConfirm);
}

void UserLoginInfo::abortConfirm(bool abort)
{
    if (!abort) {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        m_model->setPowerAction(SessionBaseModel::PowerAction::RequireNormal);
    }

    m_shutdownAbort = abort;
    m_userLoginWidget->ShutdownPrompt(m_model->powerAction());
}

UserLoginWidget *UserLoginInfo::getUserLoginWidget()
{
    return m_userLoginWidget;
}

UserFrameList *UserLoginInfo::getUserFrameList()
{
    return m_userFrameList;
}

UserExpiredWidget *UserLoginInfo::getUserExpiredWidget()
{
    return m_userExpiredWidget;
}

void UserLoginInfo::hideKBLayout()
{
    m_userLoginWidget->hideKBLayout();
}

void UserLoginInfo::userLockChanged(bool disable)
{
    m_userLoginWidget->disablePassword(disable, m_user->lockNum());
}

void UserLoginInfo::receiveSwitchUser(std::shared_ptr<User> user)
{
    if (m_user != user) {
        m_userLoginWidget->clearPassWord();

        abortConfirm(false);
    }

    emit requestSwitchUser(user);
}
