/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef INHIBITWARNVIEW_H
#define INHIBITWARNVIEW_H

#include "warningview.h"
#include "src/widgets/rounditembutton.h"
#include "src/session-widgets/sessionbasemodel.h"

#include <QWidget>

class QPushButton;
class InhibitorRow : public QWidget
{
    Q_OBJECT
public:
    InhibitorRow(QString who, QString why, const QIcon &icon = QIcon(), QWidget *parent = nullptr);
    ~InhibitorRow() override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

class InhibitWarnView : public WarningView
{
    Q_OBJECT

public:
    explicit InhibitWarnView(SessionBaseModel::PowerAction inhibitType, QWidget *parent = nullptr);
    ~InhibitWarnView() override;

    struct InhibitorData {
        QString who;
        QString why;
        QString mode;
        quint32 pid;
        QString icon;
    };

    void setInhibitorList(const QList<InhibitorData> & list);
    void setInhibitConfirmMessage(const QString &text);
    void setAcceptReason(const QString &reason) override;
    void setAction(const SessionBaseModel::PowerAction action);
    void setAcceptVisible(const bool acceptable);

    void toggleButtonState() Q_DECL_OVERRIDE;
    void buttonClickHandle() Q_DECL_OVERRIDE;

    SessionBaseModel::PowerAction inhibitType() const;

protected:
    bool focusNextPrevChild(bool next) Q_DECL_OVERRIDE;
    void setCurrentButton(const ButtonType btntype) Q_DECL_OVERRIDE;

signals:
    void cancelled() const;
    void actionInvoked(const SessionBaseModel::PowerAction action) const;

private:
    void onOtherPageDataChanged(const QVariant &value);

private:
    SessionBaseModel::PowerAction m_action;

    QList<QWidget*> m_inhibitorPtrList;
    QVBoxLayout *m_inhibitorListLayout = nullptr;
    QLabel *m_confirmTextLabel = nullptr;
    QPushButton *m_acceptBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_currentBtn;
    int m_dataBindIndex;
    SessionBaseModel::PowerAction m_inhibitType;
};

#endif // INHIBITWARNVIEW_H
