// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "user_name_widget.h"
#include "public_func.h"
#include "dconfig_helper.h"
#include "constants.h"

#include <QHBoxLayout>
#include <QPixmap>

#include <DHiDPIHelper>
#include <DFontSizeManager>
#include <qpalette.h>

const int USER_PIC_HEIGHT = 16;

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace DDESESSIONCC;

UserNameWidget::UserNameWidget(bool respondFontSizeChange, bool showDisplayName, QWidget *parent)
    : QWidget(parent)
    , m_userPicLabel(nullptr)
    , m_fullNameLabel(nullptr)
    , m_displayNameLabel(nullptr)
    , m_showUserName(false)
    , m_showDisplayName(showDisplayName)
    , m_respondFontSizeChange(respondFontSizeChange)
{
    initialize();
}

void UserNameWidget::initialize()
{
    setAccessibleName("UserNameWidget");
    QVBoxLayout *vLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;

    m_userPicLabel = new DLabel(this);
    m_userPicLabel->setAlignment(Qt::AlignVCenter);
    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/img/user_name.svg");
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_userPicLabel->setAccessibleName(QStringLiteral("CapsStateLabel"));
    m_userPicLabel->setPixmap(pixmap);
    m_userPicLabel->setFixedSize(USER_PIC_HEIGHT, USER_PIC_HEIGHT);

    m_fullNameLabel = new DLabel(this);
    m_fullNameLabel->setAlignment(Qt::AlignVCenter);
    QPalette palette = m_fullNameLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_fullNameLabel->setPalette(palette);

    // 设置字体大小
    bool ok;
    int fontSize = DConfigHelper::instance()->getConfig(FULL_NAME_FONT, 5).toInt(&ok);
    if (!ok || fontSize < 0 || fontSize > 9 || !m_respondFontSizeChange) fontSize = DFontSizeManager::T6;
    DFontSizeManager::instance()->bind(m_fullNameLabel, static_cast<DFontSizeManager::SizeType>(fontSize));

    if (m_showDisplayName) {
        m_displayNameLabel = new DLabel(this);
        m_displayNameLabel->setAccessibleName(QStringLiteral("NameLabel"));
        m_displayNameLabel->setTextFormat(Qt::TextFormat::PlainText);
        m_displayNameLabel->setAlignment(Qt::AlignCenter);
        m_displayNameLabel->setTextFormat(Qt::TextFormat::PlainText);
        DFontSizeManager::instance()->bind(m_displayNameLabel, DFontSizeManager::T2);
        palette = m_displayNameLabel->palette();
        palette.setColor(QPalette::WindowText, Qt::white);
        m_displayNameLabel->setPalette(palette);
        vLayout->addWidget(m_displayNameLabel);
    }

    vLayout->addLayout(hLayout);
    hLayout->addStretch();
    hLayout->addWidget(m_userPicLabel);
    hLayout->addWidget(m_fullNameLabel);
    hLayout->addStretch();

    setLayout(vLayout);

    m_showUserName = DConfigHelper::instance()->getConfig(SHOW_USER_NAME, false).toBool();
    m_fullNameLabel->setVisible(m_showUserName);
    m_userPicLabel->setVisible(m_showUserName);

    DConfigHelper::instance()->bind(this, FULL_NAME_FONT, &UserNameWidget::onDConfigPropertyChanged);
    DConfigHelper::instance()->bind(this, SHOW_USER_NAME, &UserNameWidget::onDConfigPropertyChanged);
}

void UserNameWidget::updateUserName(const QString &userName)
{
    if (userName == m_userNameStr)
        return;

    m_userNameStr = userName;
    updateUserNameWidget();
    updateDisplayNameWidget();
}

void UserNameWidget::updateUserNameWidget()
{
    const int nameWidth = m_fullNameLabel->fontMetrics().boundingRect(m_fullNameStr).width();
    const int labelMaxWidth = width() - 20;

    if (nameWidth > labelMaxWidth) {
        const QString &str = m_fullNameLabel->fontMetrics().elidedText(m_fullNameStr, Qt::ElideRight, labelMaxWidth);
        m_fullNameLabel->setText(str);
    } else {
        m_fullNameLabel->setText(m_fullNameStr);
    }
}

void UserNameWidget::updateFullName(const QString &fullName)
{
    if (fullName == m_fullNameStr)
        return;

    m_fullNameStr = fullName;
    updateUserNameWidget();
    updateDisplayNameWidget();
}

/**
 * @brief 如果在DConfig中配置了showUserName为true，那么displayName显示用户名，
 * 否则显示全名，如果全名为空，则显示用户名
 *
 */
void UserNameWidget::updateDisplayNameWidget()
{
    if (!m_displayNameLabel)
        return;

    QString displayName = m_showUserName ? m_userNameStr : (m_fullNameStr.isEmpty() ? m_userNameStr : m_fullNameStr);
    int nameWidth = m_displayNameLabel->fontMetrics().boundingRect(displayName).width();
    int labelMaxWidth = width() - 20;

    if (nameWidth > labelMaxWidth) {
        QString str = m_displayNameLabel->fontMetrics().elidedText(displayName, Qt::ElideRight, labelMaxWidth);
        m_displayNameLabel->setText(str);
    } else {
        m_displayNameLabel->setText(displayName);
    }
}

void UserNameWidget::resizeEvent(QResizeEvent *event)
{
    updateUserNameWidget();
    updateDisplayNameWidget();

    QWidget::resizeEvent(event);
}

int UserNameWidget::heightHint() const
{
    int height = 0;
    if (m_userPicLabel)
        height += USER_PIC_HEIGHT;
    if (m_fullNameLabel)
        height += m_fullNameLabel->fontMetrics().height();
    if (m_displayNameLabel)
        height += m_displayNameLabel->fontMetrics().height();

    return height;
}

void UserNameWidget::onDConfigPropertyChanged(const QString &key, const QVariant &value, QObject *objPtr)
{
    auto obj = qobject_cast<UserNameWidget*>(objPtr);
    if (!obj)
        return;

    if (key == SHOW_USER_NAME) {
        obj->m_showUserName = value.toBool();
        obj->m_fullNameLabel->setVisible(obj->m_showUserName);
        obj->m_userPicLabel->setVisible(obj->m_showUserName);
        obj->updateUserNameWidget();
        obj->updateDisplayNameWidget();
    } else if (key == FULL_NAME_FONT && obj->m_respondFontSizeChange) {
        bool ok;
        const int fontSize = value.toInt(&ok);
        if (ok && fontSize > 0 && fontSize < 9)
            DFontSizeManager::instance()->bind(obj->m_fullNameLabel, static_cast<DFontSizeManager::SizeType>(fontSize));
        else
            qWarning() << "Top tip text font format error";
    }
}
