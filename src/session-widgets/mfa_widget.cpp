/*
* Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     Zhang Qipeng <zhangqipeng@uniontech.com>
*
* Maintainer: Zhang Qipeng <zhangqipeng@uniontech.com>
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

#include "mfa_widget.h"

#include "auth_face.h"
#include "auth_fingerprint.h"
#include "auth_iris.h"
#include "auth_password.h"
#include "auth_ukey.h"
#include "dlineeditex.h"
#include "framedatabind.h"
#include "keyboardmonitor.h"
#include "sessionbasemodel.h"
#include "useravatar.h"

MFAWidget::MFAWidget(QWidget *parent)
    : AuthWidget(parent)
    , m_mainLayout(new QVBoxLayout(this))
{
    setObjectName(QStringLiteral("MFAWidget"));
    setAccessibleName(QStringLiteral("MFAWidget"));

    setGeometry(0, 0, 280, 176);
    setMinimumSize(280, 176);
}

void MFAWidget::initUI()
{
    AuthWidget::initUI();
    /* 用户名输入框 */
    std::function<void(QVariant)> accountChanged = std::bind(&MFAWidget::syncAccount, this, std::placeholders::_1);
    m_registerFunctions["MFAAccount"] = FrameDataBind::Instance()->registerFunction("MFAAccount", accountChanged);
    FrameDataBind::Instance()->refreshData("MFAAccount");

    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(10);
    m_mainLayout->addWidget(m_userAvatar);
    m_mainLayout->addWidget(m_nameLabel, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_accountEdit, 0, Qt::AlignVCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_lockButton, 0, Qt::AlignCenter);
}

void MFAWidget::initConnections()
{
    AuthWidget::initConnections();
    connect(m_model, &SessionBaseModel::authTypeChanged, this, &MFAWidget::setAuthType);
    connect(m_model, &SessionBaseModel::authStatusChanged, this, &MFAWidget::setAuthStatus);
    connect(m_accountEdit, &DLineEditEx::textChanged, this, [this](const QString &value) {
        FrameDataBind::Instance()->updateValue("MFAAccount", value);
    });
}

void MFAWidget::setModel(const SessionBaseModel *model)
{
    AuthWidget::setModel(model);

    initUI();
    initConnections();

    setAuthType(model->getAuthProperty().AuthType);
    setUser(model->currentUser());
}

void MFAWidget::setAuthType(const int type)
{
    qDebug() << "MFAWidget::setAuthType:" << type;
    m_index = 2;
    /* 面容 */
    if (type & AuthTypeFace) {
        initFaceAuth();
        m_index++;
    } else if (m_faceAuth) {
        m_faceAuth->deleteLater();
        m_faceAuth = nullptr;
    }
    /* 虹膜 */
    if (type & AuthTypeIris) {
        initIrisAuth();
        m_index++;
    } else if (m_irisAuth) {
        m_irisAuth->deleteLater();
        m_irisAuth = nullptr;
    }
    /* 指纹 */
    if (type & AuthTypeFingerprint) {
        initFingerprintAuth();
        m_index++;
    } else if (m_fingerprintAuth) {
        m_fingerprintAuth->deleteLater();
        m_fingerprintAuth = nullptr;
    }
    /* Ukey */
    if (type & AuthTypeUkey) {
        initUKeyAuth();
        m_index++;
    } else if (m_ukeyAuth) {
        m_ukeyAuth->deleteLater();
        m_ukeyAuth = nullptr;
    }
    /* 密码 */
    if (type & AuthTypePassword) {
        initPasswdAuth();
        m_index++;
    } else if (m_passwordAuth) {
        m_passwordAuth->deleteLater();
        m_passwordAuth = nullptr;
    }
    /* 账户 */
    if (type == AuthTypeNone) {
        if (m_model->currentUser()->isNoPasswordLogin()) {
            m_lockButton->setEnabled(true);
            m_accountEdit->hide();
            m_nameLabel->show();
        } else {
            m_accountEdit->clear();
            m_accountEdit->show();
            m_nameLabel->hide();
        }
    } else {
        const bool visible = m_model->isServerModel() && m_model->currentType() == SessionBaseModel::LightdmType;
        m_accountEdit->setVisible(visible);
        m_nameLabel->setVisible(!visible);
    }

    updateExpiredStatus();
     updateGeometry();

    if (m_lockButton->isVisible() && m_lockButton->isEnabled()) {
        setFocusProxy(m_lockButton);
    }
    if (m_passwordAuth) {
        setFocusProxy(m_passwordAuth);
    }
    if (m_ukeyAuth) {
        setFocusProxy(m_ukeyAuth);
    }
    if (m_accountEdit->isVisible()) {
        setFocusProxy(m_accountEdit);
    }

    m_widgetList.clear();
    m_widgetList << m_accountEdit << m_ukeyAuth << m_passwordAuth << m_lockButton;

    for (int i = 0; i < m_widgetList.size(); i++) {
        if (m_widgetList[i]) {
            for (int j = i + 1; j < m_widgetList.size(); j++) {
                if (m_widgetList[j]) {
                    setTabOrder(m_widgetList[i]->focusProxy(), m_widgetList[j]->focusProxy());
                    i = j - 1;
                    break;
                }
            }
        }
    }
    setFocus();
}

/**
 * @brief 认证状态
 * @param type    认证类型
 * @param status  认证结果
 * @param message 消息
 */
void MFAWidget::setAuthStatus(const int type, const int status, const QString &message)
{
    qDebug() << "MFAWidget::setAuthStatus:" << type << status << message;
    switch (type) {
    case AuthTypePassword:
        if (m_passwordAuth) {
            m_passwordAuth->setAuthStatus(status, message);
            FrameDataBind::Instance()->updateValue("MFPasswordAuthStatus", status);
            FrameDataBind::Instance()->updateValue("MFPasswordAuthMsg", message);
        }
        break;
    case AuthTypeFingerprint:
        if (m_fingerprintAuth) {
            m_fingerprintAuth->setAuthStatus(status, message);
            FrameDataBind::Instance()->updateValue("MFFingerprintAuthStatus", status);
            FrameDataBind::Instance()->updateValue("MFFingerprintAuthMsg", message);
        }
        break;
    case AuthTypeUkey:
        if (m_ukeyAuth) {
            m_ukeyAuth->setAuthStatus(status, message);
            FrameDataBind::Instance()->updateValue("MFUKeyAuthStatus", status);
            FrameDataBind::Instance()->updateValue("MFUKeyAuthMsg", message);
        }
        break;
    case AuthTypeFace:
        if (m_faceAuth) {
            m_faceAuth->setAuthStatus(status, message);
            FrameDataBind::Instance()->updateValue("MFFaceAuthStatus", status);
            FrameDataBind::Instance()->updateValue("MFFaceAuthMsg", message);
        }
        break;
    case AuthTypeIris:
        if (m_irisAuth) {
            m_irisAuth->setAuthStatus(status, message);
            FrameDataBind::Instance()->updateValue("MFIrisAuthStatus", status);
            FrameDataBind::Instance()->updateValue("MFIrisAuthMsg", message);
        }
        break;
    case AuthTypeAll:
        checkAuthResult(type, status);
        break;
    default:
        break;
    }
}

/**
 * @brief 密码认证
 */
void MFAWidget::initPasswdAuth()
{
    if (m_passwordAuth) {
        m_passwordAuth->reset();
        return;
    }
    m_passwordAuth = new AuthPassword(this);
    m_passwordAuth->setCapsLockVisible(m_capslockMonitor->isCapslockOn());
    m_passwordAuth->setKeyboardButtonInfo(m_user->keyboardLayout());
    m_passwordAuth->setKeyboardButtonVisible(m_user->keyboardLayoutList().size() > 1);
    m_passwordAuth->setPasswordHint(m_user->passwordHint());
    m_mainLayout->insertWidget(m_index, m_passwordAuth);

    connect(m_passwordAuth, &AuthPassword::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AuthTypePassword);
    });
    connect(m_passwordAuth, &AuthPassword::requestAuthenticate, this, [this] {
        const QString &text = m_passwordAuth->lineEditText();
        if (text.isEmpty()) {
            return;
        }
        m_passwordAuth->setAuthStatusStyle(LOGIN_SPINNER);
        m_passwordAuth->setAnimationStatus(true);
        m_passwordAuth->setLineEditEnabled(false);
        emit sendTokenToAuth(m_model->currentUser()->name(), AuthTypePassword, text);
    });
    connect(m_passwordAuth, &AuthPassword::requestShowKeyboardList, this, &MFAWidget::showKeyboardList);
    connect(m_passwordAuth, &AuthPassword::authFinished, this, [this](const int value) {
        checkAuthResult(AuthTypePassword, value);
    });
    connect(m_lockButton, &QPushButton::clicked, m_passwordAuth, &AuthPassword::requestAuthenticate);
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_passwordAuth, &AuthPassword::setCapsLockVisible);

    /* 输入框数据同步 */
    std::function<void(QVariant)> passwordChanged = std::bind(&MFAWidget::syncPassword, this, std::placeholders::_1);
    m_registerFunctions["MFPasswordAuth"] = FrameDataBind::Instance()->registerFunction("MFPasswordAuth", passwordChanged);
    connect(m_passwordAuth, &AuthPassword::lineEditTextChanged, this, [this](const QString &value) {
        FrameDataBind::Instance()->updateValue("MFPasswordAuth", value);
    });
    FrameDataBind::Instance()->refreshData("MFPasswordAuth");

    connect(m_passwordAuth, &AuthPassword::requestChangeFocus, this, &MFAWidget::updateFocusPosition);
    connect(m_passwordAuth, &AuthPassword::focusChanged, this, [this](bool focus) {
        if (!focus) {
            m_keyboardTypeBorder->setVisible(false);
        }
    });
}

/**
 * @brief 指纹认证
 */
void MFAWidget::initFingerprintAuth()
{
    if (m_fingerprintAuth) {
        m_fingerprintAuth->reset();
        return;
    }
    m_fingerprintAuth = new AuthFingerprint(this);
    m_mainLayout->insertWidget(m_index, m_fingerprintAuth);

    connect(m_fingerprintAuth, &AuthFingerprint::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AuthTypeFingerprint);
    });
    connect(m_fingerprintAuth, &AuthFingerprint::authFinished, this, [this](const int value) {
        checkAuthResult(AuthTypePassword, value);
    });
}

/**
 * @brief Ukey认证
 */
void MFAWidget::initUKeyAuth()
{
    if (m_ukeyAuth) {
        m_ukeyAuth->reset();
        return;
    }
    m_ukeyAuth = new AuthUKey(this);
    m_ukeyAuth->setCapsLockVisible(m_capslockMonitor->isCapslockOn());
    m_mainLayout->insertWidget(m_index, m_ukeyAuth);

    connect(m_ukeyAuth, &AuthUKey::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AuthTypeUkey);
    });
    connect(m_ukeyAuth, &AuthUKey::requestAuthenticate, this, [this] {
        const QString &text = m_ukeyAuth->lineEditText();
        if (text.isEmpty()) {
            return;
        }
        m_ukeyAuth->setAuthStatusStyle(LOGIN_SPINNER);
        m_ukeyAuth->setAnimationStatus(true);
        m_ukeyAuth->setLineEditEnabled(false);
        emit sendTokenToAuth(m_model->currentUser()->name(), AuthTypeUkey, text);
    });
    connect(m_lockButton, &QPushButton::clicked, m_ukeyAuth, &AuthUKey::requestAuthenticate);
    connect(m_ukeyAuth, &AuthUKey::authFinished, this, [this](const bool status) {
        checkAuthResult(AuthTypeUkey, status);
    });
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_ukeyAuth, &AuthUKey::setCapsLockVisible);

    /* 输入框数据同步 */
    std::function<void(QVariant)> PINChanged = std::bind(&MFAWidget::syncUKey, this, std::placeholders::_1);
    m_registerFunctions["MFUKeyAuth"] = FrameDataBind::Instance()->registerFunction("MFUKeyAuth", PINChanged);
    connect(m_ukeyAuth, &AuthUKey::lineEditTextChanged, this, [this](const QString &value) {
        FrameDataBind::Instance()->updateValue("MFUKeyAuth", value);
        if (m_model->getAuthProperty().PINLen > 0 && value.size() >= m_model->getAuthProperty().PINLen) {
            emit m_ukeyAuth->requestAuthenticate();
        }
    });
    FrameDataBind::Instance()->refreshData("MFUKeyAuth");

    connect(m_ukeyAuth, &AuthUKey::requestChangeFocus, this, &MFAWidget::updateFocusPosition);
}

/**
 * @brief 人脸认证
 */
void MFAWidget::initFaceAuth()
{
    if (m_faceAuth) {
        m_faceAuth->reset();
        return;
    }
    m_faceAuth = new AuthFace(this);
    m_mainLayout->insertWidget(m_index, m_faceAuth);

    connect(m_faceAuth, &AuthFace::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AuthTypeFace);
    });
    connect(m_faceAuth, &AuthFace::authFinished, this, [this](const bool status) {
        checkAuthResult(AuthTypeFace, status);
    });
}

/**
 * @brief 虹膜认证
 */
void MFAWidget::initIrisAuth()
{
    if (m_irisAuth) {
        m_irisAuth->reset();
        return;
    }
    m_irisAuth = new AuthIris(this);
    m_mainLayout->insertWidget(m_index, m_irisAuth);

    connect(m_irisAuth, &AuthIris::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AuthTypeIris);
    });
    connect(m_irisAuth, &AuthIris::authFinished, this, [this](const bool status) {
        checkAuthResult(AuthTypeIris, status);
    });
}

/**
 * @brief 检查多因子认证结果
 *
 * @param type
 * @param succeed
 */
void MFAWidget::checkAuthResult(const int type, const int status)
{
    if (type == AuthTypePassword && status == StatusCodeSuccess) {
        if (m_fingerprintAuth && m_fingerprintAuth->authStatus() == StatusCodeFailure) {
            m_fingerprintAuth->reset();
        }
    }
    if (status == StatusCodeCancel) {
        if (m_ukeyAuth) {
            m_ukeyAuth->setAuthStatus(StatusCodeCancel, "Cancel");
        }
        if (m_passwordAuth) {
            m_passwordAuth->setAuthStatus(StatusCodeCancel, "Cancel");
        }
        if (m_fingerprintAuth) {
            m_fingerprintAuth->setAuthStatus(StatusCodeCancel, "Cancel");
        }
    }
}

void MFAWidget::updateFocusPosition()
{
    AuthModule *module = static_cast<AuthModule *>(sender());
    if (module == m_passwordAuth) {
        if (m_ukeyAuth && m_ukeyAuth->authStatus() == StatusCodePrompt) {
            setFocusProxy(m_ukeyAuth);
        } else {
            setFocusProxy(m_lockButton);
        }
    } else {
        if (m_passwordAuth && m_passwordAuth->isEnabled()) {
            setFocusProxy(m_passwordAuth);
        } else {
            setFocusProxy(m_lockButton);
        }
    }
    setFocus();
}
