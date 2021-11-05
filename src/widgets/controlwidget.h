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

#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <dtkwidget_global.h>

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>

#include <DFloatingButton>

namespace dss {
namespace module {
class BaseModuleInterface;
}
} // namespace dss

DWIDGET_USE_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DArrowRectangle;
DWIDGET_END_NAMESPACE

class MediaWidget;
class QHBoxLayout;
class QPropertyAnimation;
class QLabel;
class QMenu;

class FlotingButton : public DFloatingButton
{
    Q_OBJECT
public:
    explicit FlotingButton(QWidget *parent = nullptr)
        : DFloatingButton(parent) {
        installEventFilter(this);
    }
    explicit FlotingButton(QStyle::StandardPixmap iconType = static_cast<QStyle::StandardPixmap>(-1), QWidget *parent = nullptr)
        : DFloatingButton(iconType, parent) {
        installEventFilter(this);
    }
    explicit FlotingButton(DStyle::StandardPixmap iconType = static_cast<DStyle::StandardPixmap>(-1), QWidget *parent = nullptr)
        : DFloatingButton(iconType, parent) {
        installEventFilter(this);
    }
    explicit FlotingButton(const QString &text, QWidget *parent = nullptr)
        : DFloatingButton(text, parent) {
        installEventFilter(this);
    }
    FlotingButton(const QIcon& icon, const QString &text = QString(), QWidget *parent = nullptr)
        : DFloatingButton(icon, text, parent) {
        installEventFilter(this);
    }

Q_SIGNALS:
    void requestShowMenu();
    void requestShowTips();
    void requestHideTips();

protected:
    bool eventFilter(QObject *watch, QEvent *event) {
        if (watch == this) {
            if (event->type() == QEvent::MouseButtonRelease) {
                QMouseEvent *e = static_cast<QMouseEvent *>(event);
                if (e->button() == Qt::RightButton) {
                    Q_EMIT requestShowMenu();
                }
            } else if (event->type() == QEvent::Enter) {
                Q_EMIT requestShowTips();
            } else if (event->type() == QEvent::Leave) {
                Q_EMIT requestHideTips();
            }
        }

        return false;
    }
};
class ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlWidget(QWidget *parent = nullptr);

signals:
    void requestSwitchUser();
    void requestShutdown();
    void requestSwitchSession();
    void requestSwitchVirtualKB();
    void requestShowModule(const QString &name);

public slots:
    void addModule(dss::module::BaseModuleInterface *module);
    void removeModule(dss::module::BaseModuleInterface *module);
    void setVirtualKBVisible(bool visible);
    void setUserSwitchEnable(const bool visible);
    void setSessionSwitchEnable(const bool visible);
    void chooseToSession(const QString &session);
    void leftKeySwitch();
    void rightKeySwitch();

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnect();
    void showTips();
    void hideTips();
    void updateLayout();

private:
    int m_index = 0;
    QList<DFloatingButton *> m_btnList;

    QHBoxLayout *m_mainLayout = nullptr;
    DFloatingButton *m_virtualKBBtn = nullptr;
    DFloatingButton *m_switchUserBtn = nullptr;
    DFloatingButton *m_powerBtn = nullptr;
    DFloatingButton *m_sessionBtn = nullptr;
    QLabel *m_sessionTip = nullptr;
    QWidget *m_tipWidget = nullptr;
#ifndef SHENWEI_PLATFORM
    QPropertyAnimation *m_tipsAni = nullptr;
#endif
    QMap<QString, QWidget *> m_modules;
    QMenu *m_contextMenu;
    DArrowRectangle *m_tipsWidget;
};

#endif // CONTROLWIDGET_H
