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

#include "login_module.h"

#include <QBoxLayout>
#include <QWidget>

#include <QtWebEngineWidgets/QWebEngineView>

#include <QtWebChannel/QWebChannel>

namespace dss {
namespace module {

LoginModule::LoginModule(QObject *parent)
    : QObject(parent)
    , m_callbackFun(nullptr)
    , m_callbackData(new AuthCallbackData)
    , m_loginWidget(nullptr)
{
    setObjectName(QStringLiteral("LoginModule"));

    m_callbackData->account = "uos";
    m_callbackData->token = "1";
    m_callbackData->result = 0;
}

LoginModule::~LoginModule()
{
    if (m_loginWidget) {
        delete m_loginWidget;
    }
    if (m_callbackData) {
        delete m_callbackData;
    }
}

void LoginModule::init()
{
    initUI();
}

void LoginModule::initUI()
{
    if (m_loginWidget) {
        return;
    }
    m_loginWidget = new QWidget;
    m_loginWidget->setAccessibleName(QStringLiteral("LoginWidget"));
    m_loginWidget->setMinimumSize(280, 280);
    QVBoxLayout *loginLayout = new QVBoxLayout(m_loginWidget);

    QWebEngineView *webView = new QWebEngineView(m_loginWidget);
    QWebEnginePage *webPage = new QWebEnginePage(m_loginWidget);
    QWebChannel *webChannel = new QWebChannel(m_loginWidget);
    webView->setPage(webPage);
    webPage->setWebChannel(webChannel);
    webView->setContextMenuPolicy(Qt::NoContextMenu);

    loginLayout->addWidget(webView);

    webView->load(QUrl("https://wx.qq.com/"));
}

void LoginModule::setAuthFinishedCallback(AuthCallback *callback)
{
    m_callback = callback;
    m_callbackFun = callback->callbackFun;

    // 当认证完成，调用这个方法
    m_callbackFun(m_callbackData, m_callback->app_data);
}

} // namespace module
} // namespace dss
