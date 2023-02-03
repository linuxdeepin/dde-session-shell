// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPDATEWIDGET_H
#define UPDATEWIDGET_H

#include "sessionbasemodel.h"
#include "updatemodel.h"

#include <QFrame>
#include <QWidget>
#include <QPushButton>
#include <QStackedLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPointer>

#include <DSpinner>
#include <DLabel>
#include <DIconButton>
#include <DFloatingButton>
#include <DProgressBar>
#include <DPushButton>
#include <DCommandLinkButton>

class UpdateLogWidget: public QFrame
{
    Q_OBJECT
public:
    explicit UpdateLogWidget(QWidget *parent = nullptr);
    void setErrorLog(const QString &error);

Q_SIGNALS:
    void requestHideLogWidget();

private:
    Dtk::Widget::DCommandLinkButton *m_hideLogWidgetButton;
    Dtk::Widget::DLabel *m_logLabel;
    QWidget* m_logWidget;
};

class UpdatePrepareWidget : public QFrame
{
    Q_OBJECT
public:
    explicit UpdatePrepareWidget(QWidget *parent = nullptr);
    void showPrepare();

private:
    QLabel * m_title;
    QLabel * m_tip;
    Dtk::Widget::DSpinner *m_spinner;
};

class UpdateProgressWidget : public QFrame
{
    Q_OBJECT
public:
    explicit UpdateProgressWidget(QWidget *parent = nullptr);
    void setValue(double value);
    void setTip(const QString &tip);
    void setInstallBeginValue(int value);

private:
    QLabel *m_logo;
    QLabel *m_tip;
    Dtk::Widget::DProgressBar *m_progressBar;
    QLabel *m_progressText;
    int m_installBeginValue;
};

class UpdateCompleteWidget : public QFrame
{
    Q_OBJECT
public:
    explicit UpdateCompleteWidget(QWidget *parent = nullptr);
    void showResult(bool success, UpdateModel::UpdateError error = UpdateModel::UpdateError::NoError);

Q_SIGNALS:
    void requestShowLogWidget();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void showSuccessFrame();
    void showErrorFrame(UpdateModel::UpdateError error);
    void createButtons(const QList<UpdateModel::UpdateAction> &actions);

private:
    QLabel *m_iconLabel;
    QLabel *m_title;
    QLabel *m_tips;
    QVBoxLayout *m_mainLayout;
    QTimer *m_countDownTimer;
    int m_countDown;
    QList<QPushButton *> m_actionButtons;
    QSpacerItem *m_buttonSpacer;
    Dtk::Widget::DCommandLinkButton *m_showLogButton;
    QPointer<QPushButton> m_checkedButton;
};

class UpdateWidget : public QFrame
{
    Q_OBJECT

public:
    static UpdateWidget* instance();
    void setModel(SessionBaseModel * const model);
    void showUpdate();
    bool initialized() { return m_sessionModel != nullptr; }

signals:
    void updateExited();

private slots:
    void onExitUpdating();
    void onUpdateStatusChanged(UpdateModel::UpdateStatus status);
    void showLogWidget();
    void hideLogWidget();

private:
    explicit UpdateWidget(QWidget *parent = nullptr);
    void initUi();
    void initConnections();
    void showChecking();
    void showProgress();
    void setMouseCursorVisible(bool visible);
    void keyPressEvent(QKeyEvent *e);

private:
    UpdatePrepareWidget *m_prepareWidget;
    UpdateProgressWidget *m_progressWidget;
    UpdateCompleteWidget *m_updateCompleteWidget;
    UpdateLogWidget *m_logWidget;
    QStackedWidget *m_stackedWidget;
    SessionBaseModel* m_sessionModel;
};

#endif // UPDATEWIDGET_H
