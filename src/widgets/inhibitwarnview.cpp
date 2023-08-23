// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inhibitwarnview.h"

#include <QHBoxLayout>

#include <DStyle>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

const int ButtonWidth = 200;
const int ButtonHeight = 64;
const QSize iconSize = QSize(24, 24);

InhibitorRow::InhibitorRow(const QString &who, const QString &why, const QIcon &icon, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    QLabel *whoLabel = new QLabel(who);
    QLabel *whyLabel = new QLabel("- " + why);

    layout->addStretch();

    if (!icon.isNull()) {
        QLabel *iconLabel = new QLabel(this);
        QPixmap pixmap = icon.pixmap(topLevelWidget()->windowHandle(), QSize(48, 48));
        iconLabel->setPixmap(pixmap);
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(iconLabel);
    }

    layout->addWidget(whoLabel);
    layout->addWidget(whyLabel);
    layout->addStretch();
    this->setFixedHeight(ButtonHeight);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setLayout(layout);
}

InhibitorRow::~InhibitorRow()
{

}

void InhibitorRow::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 25));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRoundedRect(this->rect(), 18, 18);
}

InhibitWarnView::InhibitWarnView(SessionBaseModel::PowerAction inhibitType, QWidget *parent)
    : WarningView(parent)
    , m_inhibitType(inhibitType)
    , m_acceptBtn(new InhibitButton(this))
    , m_cancelBtn(new InhibitButton(this))
{
    QIcon acceptIcon = QIcon::fromTheme(":/img/inhibitview/shutdown.svg");

    m_acceptBtn->setObjectName("AcceptButton");
    m_acceptBtn->setFixedSize(ButtonWidth, ButtonHeight);
    m_acceptBtn->setFocusPolicy(Qt::StrongFocus);
    m_acceptBtn->setNormalPixmap(acceptIcon.pixmap(iconSize * devicePixelRatioF()));
    m_acceptBtn->setHoverPixmap(acceptIcon.pixmap(iconSize * devicePixelRatioF()));

    QIcon iconCancelNormal = QIcon::fromTheme(":/img/inhibitview/cancel_normal.svg");
    QIcon iconCancelHover = QIcon::fromTheme(":/img/inhibitview/cancel_hover.svg");

    m_cancelBtn->setNormalPixmap(iconCancelNormal.pixmap(iconSize * devicePixelRatioF()));
    m_cancelBtn->setHoverPixmap(iconCancelHover.pixmap(iconSize * devicePixelRatioF()));

    m_cancelBtn->setObjectName("CancelButton");
    m_cancelBtn->setFixedSize(ButtonWidth, ButtonHeight);
    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFocusPolicy(Qt::StrongFocus);

    m_confirmTextLabel = new QLabel;
    m_inhibitorListLayout = new QVBoxLayout;

    m_confirmTextLabel->setText("The reason of inhibit.");
    m_confirmTextLabel->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(m_confirmTextLabel, DFontSizeManager::T5);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addStretch();
    centralLayout->addLayout(m_inhibitorListLayout);
    centralLayout->addSpacing(50);
    centralLayout->addWidget(m_confirmTextLabel);
    centralLayout->addSpacing(70);
    centralLayout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter);
    centralLayout->addSpacing(15);
    centralLayout->addWidget(m_acceptBtn, 0, Qt::AlignHCenter);
    centralLayout->addStretch();

    setLayout(centralLayout);

    connect(m_cancelBtn, &InhibitButton::clicked, this, &InhibitWarnView::cancelled);
    connect(m_acceptBtn, &InhibitButton::clicked, this, &InhibitWarnView::actionInvoked);

    setFocusPolicy(Qt::ClickFocus);
}

InhibitWarnView::~InhibitWarnView()
{

}

void InhibitWarnView::setInhibitorList(const QList<InhibitorData> &list)
{
    for (QWidget *widget : m_inhibitorPtrList) {
        m_inhibitorListLayout->removeWidget(widget);
        widget->deleteLater();
    }
    m_inhibitorPtrList.clear();

    for (const InhibitorData &inhibitor : list) {
        QIcon icon;

        if (inhibitor.icon.isEmpty() && inhibitor.pid) {
            QFileInfo executable_info(QFile::readLink(QString("/proc/%1/exe").arg(inhibitor.pid)));

            if (executable_info.exists()) {
                icon = QIcon::fromTheme(executable_info.fileName());
            }
        } else {
            icon = QIcon::fromTheme(inhibitor.icon, QIcon::fromTheme("application-x-desktop"));
        }

        if (icon.isNull()) {
            icon = QIcon::fromTheme("application-x-desktop");
        }

        QWidget *inhibitorWidget = new InhibitorRow(inhibitor.who, inhibitor.why, icon, this);

        m_inhibitorPtrList.append(inhibitorWidget);
        m_inhibitorListLayout->addWidget(inhibitorWidget, 0, Qt::AlignHCenter);
    }
}

void InhibitWarnView::setInhibitConfirmMessage(const QString &text)
{
    m_confirmTextLabel->setText(text);
}

void InhibitWarnView::setAcceptReason(const QString &reason)
{
    m_acceptBtn->setText(reason);
}

void InhibitWarnView::setAcceptVisible(const bool acceptable)
{
    m_acceptBtn->setVisible(acceptable);
}

bool InhibitWarnView::focusNextPrevChild(bool next)
{
    if (!next) {
        qWarning() << "focus handling error, nextPrevChild is False";
        return WarningView::focusNextPrevChild(next);
    }

    return WarningView::focusNextPrevChild(next);
}

void InhibitWarnView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down || event->key() == Qt::Key_Tab) {
        QWidget *currentFocusWidget = focusWidget();
        QWidget *nextFocusWidget = nullptr;

        if (currentFocusWidget == m_cancelBtn) {
            nextFocusWidget = m_acceptBtn;
        } else if (currentFocusWidget == m_acceptBtn) {
            nextFocusWidget = m_cancelBtn;
        }

        if (nextFocusWidget) {
            nextFocusWidget->setFocus();
        } else {
            m_cancelBtn->setFocus();
        }
    }

    QWidget::keyPressEvent(event);
}
