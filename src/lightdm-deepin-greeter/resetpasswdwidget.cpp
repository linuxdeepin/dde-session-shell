/*
* Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     yinjie <yinjie@uniontech.com>
*
* Maintainer: yinjie <yinjie@uniontech.com>
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

#include "resetpasswdwidget.h"
#include "dlineeditex.h"
#include "useravatar.h"
#include "sessionbasemodel.h"
#include "framedatabind.h"

#include <QVBoxLayout>

ResetPasswdWidget::ResetPasswdWidget(QWidget *parent)
    : AuthWidget(parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_passwdEdit(new DLineEditEx(this))
{
    setObjectName("ResetPasswdWidget");
    setAccessibleName("ResetPasswdWidget");

    setGeometry(0, 0, 280, 176);
    setMinimumSize(280, 176);
}

void ResetPasswdWidget::initUI()
{
    AuthWidget::initUI();

    m_passwdEdit->setClearButtonEnabled(false);
    m_passwdEdit->setEchoMode(QLineEdit::Password);
    m_passwdEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwdEdit->lineEdit()->setAlignment(Qt::AlignCenter);

    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->addWidget(m_userAvatar);
    m_mainLayout->addWidget(m_nameLabel, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_accountEdit, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_passwdEdit, 0, Qt::AlignVCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_expiredStatusLabel);
    m_mainLayout->addItem(m_expiredSpacerItem);
    m_mainLayout->addWidget(m_lockButton, 0, Qt::AlignCenter);
}

void ResetPasswdWidget::initConnections()
{
    connect(m_passwdEdit, &DLineEditEx::returnPressed, this, [ this ] {
        if (!m_passwdEdit->text().isEmpty()) {
            emit respondPasswd(m_passwdEdit->text());
            m_passwdEdit->clear();
        }
    });

    /* 输入框数据同步 */
    registerSyncFunctions("RPPasswordText", [ this ] (const QVariant &value ) {
        m_passwdEdit->setText(value.toString());
    });

    /* 输入提示同步 */
    registerSyncFunctions("RPPasswordPlaceHolder", [ this ] (const QVariant &value ) {
        m_passwdEdit->setPlaceholderText(value.toString());
        m_passwdEdit->update();
    });

    connect(m_passwdEdit, &DLineEditEx::textChanged, this, [ this ] (const QString &value) {
        FrameDataBind::Instance()->updateValue("RPPasswordText", value);
        if (!m_passwdEdit->text().isEmpty()) {
            m_passwdEdit->hideAlertMessage();
            m_lockButton->setEnabled(!value.isEmpty());
        }
    });
}

void ResetPasswdWidget::setModel(const SessionBaseModel *model)
{
    AuthWidget::setModel(model);
    initUI();
    initConnections();

    setUser(model->currentUser());
}


void ResetPasswdWidget::setPrompt(const QString &prompt)
{
    m_passwdEdit->setFocus();
    m_passwdEdit->setPlaceholderText(prompt);
    m_passwdEdit->update();
    FrameDataBind::Instance()->updateValue("RPPasswordPlaceHolder", prompt);
}

void ResetPasswdWidget::setMessage(const QString &message)
{
    m_passwdEdit->setAlert(true);
    m_passwdEdit->showAlertMessage(message, 3000);
}
