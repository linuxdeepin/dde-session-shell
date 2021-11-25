/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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
#include "kblayoutlistview.h"

#include <DStyle>

KBLayoutListView::KBLayoutListView(const QString &language, QWidget *parent)
    : DListView(parent)
    , m_xkbParse(new XkbParser(this))
    , m_buttonModel(new QStandardItemModel(this))
    , m_curLanguage(language)
{
    initUI();
}

KBLayoutListView::~KBLayoutListView()
{
}

void KBLayoutListView::initUI()
{
    QPalette pal = palette();
    pal.setColor(DPalette::Base, QColor(235, 235, 235, 0.05 * 255));
    pal.setColor(QPalette::Active, QPalette::Highlight, QColor(235, 235, 235, 0.15 * 255));
    setPalette(pal);

    setFrameShape(QFrame::NoFrame);
    setProperty("CheckAccessibleName", false);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setResizeMode(QListView::Adjust);
    setViewportMargins(0, 0, 0, 0);
    setItemSpacing(0);
    setItemSize(QSize(200, 34));

    QMargins itemMargins(this->itemMargins());
    itemMargins.setLeft(8);
    this->setItemMargins(itemMargins);

    setModel(m_buttonModel);

    connect(this, &KBLayoutListView::clicked, this, &KBLayoutListView::onItemClick);
}

void KBLayoutListView::updateButtonState(const QString &name)
{
    for (int i = 0; i < m_buttonModel->rowCount(); i++) {
        auto item = static_cast<DStandardItem *>(m_buttonModel->item(i));
        auto action = item->actionList(Qt::Edge::LeftEdge).first();
        if (item->text() != m_curLanguage) {
            action->setIcon(QIcon());
            update(item->index());
            continue;
        }

        QIcon icon = qobject_cast<DStyle *>(style())->standardIcon(DStyle::SP_MarkElement);
        action->setIcon(icon);
        setCurrentIndex(item->index());
        update(item->index());

        QString kbd = m_buttons[m_kbdParseList.indexOf(name)];
        emit itemClicked(kbd);
    }
}

void KBLayoutListView::updateButtonList(const QStringList &buttons)
{
    if (buttons == m_buttons)
        return;

    m_buttons = buttons;
    m_kbdParseList = m_xkbParse->lookUpKeyboardList(m_buttons);

    if (m_kbdParseList.isEmpty())
        m_kbdParseList = buttons;

    // 获取当前输入法名称-全名,而不是简写的英文字符串
    if (!m_xkbParse->lookUpKeyboardList(QStringList(m_curLanguage)).isEmpty())
        m_curLanguage = m_xkbParse->lookUpKeyboardList(QStringList(m_curLanguage)).at(0);

    m_buttonModel->clear();

    for (int i = 0; i < m_kbdParseList.size(); i++)
        addItem(m_kbdParseList[i]);

    updateButtonState(m_curLanguage);

    resize(width(), DDESESSIONCC::LAYOUTBUTTON_HEIGHT * m_kbdParseList.count());
}

void KBLayoutListView::onItemClick(const QModelIndex &index)
{
    const QString &name = index.data().toString();
    m_curLanguage = name;
    updateButtonState(name);
}

void KBLayoutListView::addItem(const QString &name)
{
    DStandardItem *item = new DStandardItem(name);
    item->setFontSize(DFontSizeManager::T6);
    QSize iconSize(12, 10);
    auto leftAction = new DViewItemAction(Qt::AlignVCenter, iconSize, iconSize, true);
    QIcon icon;
    if (name != m_curLanguage) {
        icon = QIcon();
    } else {
        icon = qobject_cast<DStyle *>(style())->standardIcon(DStyle::SP_MarkElement);
    }

    leftAction->setIcon(icon);
    item->setActionList(Qt::Edge::LeftEdge, { leftAction });
    m_buttonModel->appendRow(item);
}
