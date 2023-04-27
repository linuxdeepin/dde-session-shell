// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "updatewidget.h"
#include "updateworker.h"
#include "lockcontent.h"
#include "fullscreenbackground.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScrollArea>

#include <DFontSizeManager>
#include <DHiDPIHelper>

// TODO 把类拆分成单独的文件

using namespace DDESESSIONCC;
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

const int BACKUP_BEGIN_PROGRESS = 20;
const int BACKUP_END_PROGRESS = 50;

UpdateLogWidget::UpdateLogWidget(QWidget *parent)
    : QFrame(parent)
    , m_hideLogWidgetButton(new DCommandLinkButton(tr("Hide Logs"), this))
    , m_logLabel(new DLabel(this))
    , m_logWidget(new QWidget(this))
{
    m_logLabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(m_logLabel, DFontSizeManager::T6);
    auto font = m_logLabel->font();
    font.setWeight(QFont::ExtraLight);
    m_logLabel->setFont(font);
    m_logLabel->setAlignment(Qt::AlignLeft);
    m_logLabel->setAttribute(Qt::WA_TranslucentBackground);
    m_logLabel->setTextFormat(Qt::TextFormat::PlainText);
    m_logLabel->adjustSize();
    QPalette palette = m_logLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_logLabel->setPalette(palette);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setFixedWidth(900);
    scrollArea->setFixedHeight(527);
    scrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setWidget(m_logLabel);
    scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    QPalette pa = scrollArea->palette();
    pa.setColor(QPalette::Background, Qt::transparent);
    scrollArea->setPalette(pa);

    auto logLayout = new QVBoxLayout(m_logWidget);
    logLayout->addWidget(scrollArea, 0, Qt::AlignCenter);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addWidget(m_hideLogWidgetButton, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_logWidget, 0, Qt::AlignHCenter);
    mainLayout->addStretch();

    connect(m_hideLogWidgetButton, &DCommandLinkButton::clicked, this, &UpdateLogWidget::requestHideLogWidget);
}

void UpdateLogWidget::setErrorLog(const QString &error)
{
    m_logLabel->setText(error);
}

UpdatePrepareWidget::UpdatePrepareWidget(QWidget *parent) : QFrame(parent)
{
    m_tip = new QLabel(this);
    m_tip->setText(tr("Preparing for updates…"));
    DFontSizeManager::instance()->bind(m_tip, DFontSizeManager::T6);

    m_spinner = new Dtk::Widget::DSpinner(this);
    m_spinner->setFixedSize(72, 72);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->setMargin(0);
    pLayout->addStretch();
    pLayout->addWidget(m_spinner, 0, Qt::AlignCenter);
    pLayout->addSpacing(30);
    pLayout->addWidget(m_tip, 0, Qt::AlignCenter);
    pLayout->addStretch();
}

void UpdatePrepareWidget::showPrepare()
{
    m_spinner->setVisible(true);
    m_spinner->start();
    m_tip->setVisible(true);
}

UpdateProgressWidget::UpdateProgressWidget(QWidget *parent)
    : QFrame(parent)
    , m_logo(new QLabel(this))
    , m_tip(new QLabel(this))
    , m_waitingView(new DPictureSequenceView(this))
    , m_progressBar(new DProgressBar(this))
    , m_progressText(new QLabel(this))
    , m_installBeginValue(0)
{
    m_logo->setFixedSize(286, 57);
    m_logo->setPixmap(DHiDPIHelper::loadNxPixmap(":img/update/logo.svg"));

    auto palette = m_tip->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_tip->setPalette(palette);
    m_tip->setText(tr("Do not force a shutdown or power off when installing updates. Otherwise, your system may be damaged."));
    DFontSizeManager::instance()->bind(m_tip, DFontSizeManager::T6);

    m_waitingView->setAccessibleName("WaitingUpdateSequenceView");
    m_waitingView->setFixedSize(20 * qApp->devicePixelRatio(), 20 * qApp->devicePixelRatio());
    m_waitingView->setSingleShot(false);
    QStringList pics;
    for (int i = 0; i < 40; ++i)
        pics << QString(":img/update/waiting_update/waiting_update_%1.png").arg(QString::number(i));
    m_waitingView->setPictureSequence(pics, true);

    QHBoxLayout *tipsLayout = new QHBoxLayout;
    tipsLayout->setSpacing(0);
    tipsLayout->addStretch();
    tipsLayout->addWidget(m_tip);
    tipsLayout->addSpacing(5);
    tipsLayout->addWidget(m_waitingView);
    tipsLayout->addStretch();

    m_progressBar->setFixedWidth(500);
    m_progressBar->setFixedHeight(8);
    m_progressBar->setRange(0, 100);
    m_progressBar->setAlignment(Qt::AlignRight);
    m_progressBar->setAccessibleName("ProgressBar");
    m_progressBar->setValue(1);

    m_progressText->setText("1%");
    DFontSizeManager::instance()->bind(m_progressText, DFontSizeManager::T6);

    QHBoxLayout *pProgressLayout = new QHBoxLayout;
    pProgressLayout->addStretch();
    pProgressLayout->addWidget(m_progressBar, 0, Qt::AlignCenter);
    pProgressLayout->addSpacing(10);
    pProgressLayout->addWidget(m_progressText, 0, Qt::AlignCenter);
    pProgressLayout->addStretch();

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addStretch();
    pLayout->addWidget(m_logo, 0, Qt::AlignCenter);
    pLayout->addSpacing(100);
    pLayout->addLayout(pProgressLayout, 0);
    pLayout->addSpacing(10);
    pLayout->addLayout(tipsLayout, 0);
    pLayout->addStretch();
}

bool UpdateProgressWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Show)
        m_waitingView->play();
    else if (e->type() == QEvent::Hide)
        m_waitingView->stop();

    return false;
}

void UpdateProgressWidget::setValue(double value)
{
    qInfo() << "UpdateProgressWidget::setValue: " << value
            << " begin progress: " << m_installBeginValue;

    double tmpValue = value * (100 - m_installBeginValue);
    // 在备份完成后,如果m_installBeginValue=50,那么value需要大于等于2进度条才会增加,等待时间过长,体验不好.
    if (m_installBeginValue > 0 && tmpValue < 1 && tmpValue > 0)
        tmpValue = 1.0;

    int iProgress = m_installBeginValue + static_cast<int>(tmpValue);
    // 进度条不能大于100，不能小于0，不能回退
    if (iProgress > 100 || iProgress < 0 || iProgress <= m_progressBar->value())
        return;

    qInfo() << Q_FUNC_INFO << iProgress;
    m_progressBar->setValue(iProgress);
    m_progressText->setText(QString::number(iProgress) + "%");
}

void UpdateProgressWidget::setInstallBeginValue(int value)
{
    m_installBeginValue = value;

    m_progressBar->setValue(value);
    m_progressText->setText(QString::number(value) + "%");
}

UpdateCompleteWidget::UpdateCompleteWidget(QWidget *parent)
    : QFrame(parent)
    , m_iconLabel(new QLabel(this))
    , m_title(new QLabel(this))
    , m_tips(new QLabel(this))
    , m_mainLayout(nullptr)
    , m_countDownTimer(nullptr)
    , m_countDown(3)
    , m_buttonSpacer(new QSpacerItem(0, 80))
    , m_showLogButton(new Dtk::Widget::DCommandLinkButton(tr("View Logs"), this))
    , m_checkedButton(nullptr)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    setPalette(palette);

    m_iconLabel->setFixedSize(128, 128);

    m_title->setAlignment(Qt::AlignVCenter);
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T4);

    m_tips->setAlignment(Qt::AlignVCenter);
    DFontSizeManager::instance()->bind(m_tips, DFontSizeManager::T6);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_title, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_tips,0 , Qt::AlignCenter);
    m_mainLayout->addWidget(m_showLogButton,0 , Qt::AlignCenter);
    m_mainLayout->addItem(m_buttonSpacer);

    connect(m_showLogButton, &DCommandLinkButton::clicked, this, &UpdateCompleteWidget::requestShowLogWidget);
}

void UpdateCompleteWidget::showResult(bool success, UpdateModel::UpdateError error)
{
    qInfo() << "UpdateCompleteWidget::showResult: " << success
            << ", error: " << error;

    if (success)
        showSuccessFrame();
    else
        showErrorFrame(error);
}

void UpdateCompleteWidget::showSuccessFrame()
{
    qDeleteAll(m_actionButtons);
    m_actionButtons.clear();
    m_buttonSpacer->changeSize(0, 0);
    m_mainLayout->invalidate();
    m_showLogButton->setVisible(false);

    m_iconLabel->setPixmap(DHiDPIHelper::loadNxPixmap(":img/update/success.svg"));
    m_title->setText(tr("Updates successful"));
    m_countDown = 3;

    auto setTipsText = [this] {
        const QString &tips = UpdateModel::instance()->isReboot() ?
                tr("Your computer will reboot soon %1") : tr("Your computer will be turned off soon %1");
        m_tips->setText(tips.arg(QString::number(m_countDown)));
        m_tips->setVisible(true);
        m_countDown--;
    };

    if (m_countDownTimer == nullptr) {
        m_countDownTimer = new QTimer(this);
        m_countDownTimer->setInterval(1000);
        m_countDownTimer->setSingleShot(false);
        connect(m_countDownTimer, &QTimer::timeout, this, [this, setTipsText] {
            if (m_countDown <= 0) {
                UpdateWorker::instance()->doPowerAction(UpdateModel::instance()->isReboot());
                m_countDownTimer->stop();
                return;
            }
            setTipsText();
        });
    }

    setTipsText();
    m_countDownTimer->start();
}

void UpdateCompleteWidget::showErrorFrame(UpdateModel::UpdateError error)
{
    qInfo() << "UpdateCompleteWidget::showErrorFrame: " << error;

    m_buttonSpacer->changeSize(0, 0);
    static const QMap<UpdateModel::UpdateError, QList<UpdateModel::UpdateAction>> ErrorActions = {
        {UpdateModel::CanNotBackup, {UpdateModel::ContinueUpdating, UpdateModel::ExitUpdating}},
        {UpdateModel::BackupInterfaceError, {UpdateModel::ExitUpdating, UpdateModel::ContinueUpdating}},
        {UpdateModel::BackupFailedUnknownReason, {UpdateModel::DoBackupAgain, UpdateModel::ExitUpdating, UpdateModel::ContinueUpdating}},
        {UpdateModel::BackupNoSpace, {UpdateModel::ContinueUpdating, UpdateModel::ExitUpdating}},
        {UpdateModel::UpdateInterfaceError, {UpdateModel::Reboot, UpdateModel::ShutDown}},
        {UpdateModel::InstallNoSpace, {UpdateModel::Reboot, UpdateModel::ShutDown}},
        {UpdateModel::DependenciesBrokenError, {UpdateModel::Reboot, UpdateModel::ShutDown}},
        {UpdateModel::DpkgInterrupted, {UpdateModel::Reboot, UpdateModel::ShutDown}},
        {UpdateModel::UnKnown, {UpdateModel::Reboot, UpdateModel::ShutDown}}
    };

    m_iconLabel->setPixmap(DHiDPIHelper::loadNxPixmap(":img/update/failed.svg"));
    const auto actions = ErrorActions.value(error);
    auto pair = UpdateModel::updateErrorMessage(error);
    m_title->setText(pair.first);
    m_tips->setVisible(!pair.second.isEmpty());
    m_tips->setText(pair.second);
    m_showLogButton->setVisible(error >= UpdateModel::UpdateInterfaceError);
    createButtons(actions);
    if (!m_actionButtons.isEmpty())
        m_buttonSpacer->changeSize(0, 80);

    m_mainLayout->invalidate();

    // 安装失败后10分钟后自动关机
    if (error >= UpdateModel::UpdateInterfaceError) {
        QTimer::singleShot(1000*60*10, this, [] {
            qInfo() << "User has not operated for a long time， do shut down action now";
            UpdateWorker::instance()->doPowerAction(false);
        });
    }
}

void UpdateCompleteWidget::createButtons(const QList<UpdateModel::UpdateAction> &actions)
{
    qDeleteAll(m_actionButtons);
    m_actionButtons.clear();

    m_checkedButton = nullptr;
    for (auto action : actions) {
        auto button = new QPushButton(UpdateModel::updateActionText(action), this);
        button->setFixedSize(240, 48);
        m_mainLayout->addWidget(button, 0, Qt::AlignHCenter);
        m_actionButtons.append(button);
        button->setFocusPolicy(Qt::NoFocus);
        button->setCheckable(true);
        // 按钮的选中状态跟随用户最初选择的是关机还是重启
        if ((action == UpdateModel::Reboot && UpdateModel::instance()->isReboot())
            || (action == UpdateModel::ShutDown && !UpdateModel::instance()->isReboot())) {
            button->setChecked(true);
            m_checkedButton = button;
        }

        connect(button, &QPushButton::clicked, this, [action] {
            UpdateWorker::instance()->doAction(action);
        });
    }

    // 非重启/关机按钮的情况，默认选中第一个按钮
    if (!m_checkedButton && !m_actionButtons.isEmpty()) {
        m_checkedButton = m_actionButtons.first();
        m_checkedButton->setChecked(true);
    }
}

void UpdateCompleteWidget::keyPressEvent(QKeyEvent *event)
{
    qInfo() << Q_FUNC_INFO;
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Tab: {
        if (m_actionButtons.size() > 1) {
            int index = 0;
            if (m_checkedButton) {
                index = m_actionButtons.indexOf(m_checkedButton.data());
                if (index == m_actionButtons.length() - 1)
                    index = 0;
                else
                    index++;

                m_checkedButton->setChecked(false);
            }
            m_checkedButton = m_actionButtons.at(index);
            m_checkedButton->setChecked(true);
        }

        break;
    }
    case Qt::Key_Return:
        if(m_checkedButton)
            m_checkedButton->clicked();
        break;
    case Qt::Key_Enter:
        if (m_checkedButton)
            m_checkedButton->clicked();
        break;
    }
    QWidget::keyPressEvent(event);
}

UpdateWidget::UpdateWidget(QWidget *parent)
    : QFrame(parent)
    , m_sessionModel(nullptr)
{

}

UpdateWidget* UpdateWidget::instance()
{
    static UpdateWidget* updateWidget = nullptr;
    if (!updateWidget) {
        updateWidget = new UpdateWidget;
    }
    return updateWidget;
}

void UpdateWidget::setModel(SessionBaseModel * const model)
{
    m_sessionModel = model;

    connect(m_sessionModel, &SessionBaseModel::showUpdate, this, [this](bool doReboot) {
        qInfo() << "Show update" << doReboot;
        if (UpdateModel::instance()->isUpdating()) {
            qWarning() << "Is updating, can not do it again";
            return;
        }

        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            initUi();
            initConnections();
        }

        FullScreenBackground::setContent(UpdateWidget::instance());
        m_sessionModel->setCurrentContentType(SessionBaseModel::UpdateContent);
        UpdateModel::instance()->setIsReboot(doReboot);
        UpdateWidget::instance()->showUpdate();
    });
}

void UpdateWidget::showUpdate()
{
    UpdateModel::instance()->setIsUpdating(true);
    UpdateModel::instance()->setUpdateStatus(UpdateModel::Ready);
}

void UpdateWidget::onUpdateStatusChanged(UpdateModel::UpdateStatus status)
{
    qInfo() << Q_FUNC_INFO << status;
    switch (status) {
        case UpdateModel::UpdateStatus::Ready:
            showChecking();
            UpdateWorker::instance()->startUpdateProgress();
            break;
        case UpdateModel::UpdateStatus::BackingUp:
            m_stackedWidget->setCurrentWidget(m_progressWidget);
            m_progressWidget->setInstallBeginValue(BACKUP_BEGIN_PROGRESS);
            break;
        case UpdateModel::UpdateStatus::BackupSuccess:
            m_progressWidget->setInstallBeginValue(BACKUP_END_PROGRESS);
            break;
        case UpdateModel::UpdateStatus::Installing:
            setMouseCursorVisible(false);
            m_stackedWidget->setCurrentWidget(m_progressWidget);
            break;
        case UpdateModel::UpdateStatus::InstallSuccess:
            // 升级成功
            setMouseCursorVisible(false);
            m_stackedWidget->setCurrentWidget(m_updateCompleteWidget);
            m_updateCompleteWidget->showResult(true);
            break;
        case UpdateModel::UpdateStatus::InstallFailed:
        case UpdateModel::UpdateStatus::BackupFailed:
        case UpdateModel::UpdateStatus::PrepareFailed:
            setMouseCursorVisible(true);
            m_stackedWidget->setCurrentWidget(m_updateCompleteWidget);
            m_updateCompleteWidget->showResult(false, UpdateModel::instance()->updateError());

        default:
            break;
    }
}

void UpdateWidget::onExitUpdating()
{
    qInfo() << Q_FUNC_INFO;
    setMouseCursorVisible(true);
    UpdateModel::instance()->setIsUpdating(false);
    if (m_sessionModel) {
        m_sessionModel->setVisible(false);
        m_sessionModel->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    }

    FullScreenBackground::setContent(LockContent::instance());
    m_sessionModel->setCurrentContentType(SessionBaseModel::LockContent);
    Q_EMIT updateExited();
}

void UpdateWidget::initUi()
{
    m_prepareWidget = new UpdatePrepareWidget(this);
    m_progressWidget = new UpdateProgressWidget(this);
    m_updateCompleteWidget = new UpdateCompleteWidget(this);
    m_logWidget = new UpdateLogWidget(this);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->addWidget(m_prepareWidget);
    m_stackedWidget->addWidget(m_progressWidget);
    m_stackedWidget->addWidget(m_updateCompleteWidget);
    m_stackedWidget->addWidget(m_logWidget);

    QVBoxLayout * pLayout = new QVBoxLayout(this);
    pLayout->setMargin(0);
    pLayout->setSpacing(0);
    pLayout->addWidget(m_stackedWidget, 0, Qt::AlignCenter);

    setFocusProxy(m_stackedWidget);
}

void UpdateWidget::initConnections()
{
    connect(UpdateWorker::instance(), &UpdateWorker::requestExitUpdating,this, &UpdateWidget::onExitUpdating);
    connect(UpdateModel::instance(), &UpdateModel::distUpgradeProgressChanged, m_progressWidget, &UpdateProgressWidget::setValue);
    connect(UpdateModel::instance(), &UpdateModel::updateStatusChanged, this, &UpdateWidget::onUpdateStatusChanged);
    connect(m_updateCompleteWidget, &UpdateCompleteWidget::requestShowLogWidget, this, &UpdateWidget::showLogWidget);
    connect(m_logWidget, &UpdateLogWidget::requestHideLogWidget, this, &UpdateWidget::hideLogWidget);
    connect(m_stackedWidget, &QStackedWidget::currentChanged, this, [this] {
        auto w = m_stackedWidget->currentWidget();
        if (w) {
            m_stackedWidget->setFocusProxy(w);
            m_stackedWidget->setFocus();
        }
    });
}

void UpdateWidget::showLogWidget()
{
    setMouseCursorVisible(true);
    m_logWidget->setErrorLog(UpdateModel::instance()->lastErrorLog());
    m_stackedWidget->setCurrentWidget(m_logWidget);
}

void UpdateWidget::hideLogWidget()
{
    m_stackedWidget->setCurrentWidget(m_updateCompleteWidget);
}

void UpdateWidget::showChecking()
{
    setMouseCursorVisible(false);
    m_stackedWidget->setCurrentWidget(m_prepareWidget);
    m_prepareWidget->showPrepare();
}

void UpdateWidget::showProgress()
{
    setMouseCursorVisible(false);
    m_stackedWidget->setCurrentWidget(m_progressWidget);
}

void UpdateWidget::setMouseCursorVisible( bool visible)
{
    qInfo() << Q_FUNC_INFO << visible;
    static bool mouseVisible=true;
    if(mouseVisible == visible)
        return;

    mouseVisible= visible;
    QApplication::setOverrideCursor(mouseVisible ? Qt::ArrowCursor : Qt::BlankCursor);
}

void UpdateWidget::keyPressEvent(QKeyEvent *e)
{
    qInfo() << Q_FUNC_INFO;

    // 屏蔽esc键，设置event的accept无效，暂时不处理
}

bool UpdateWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Show) {
        UpdateWorker::instance()->enableShortcuts(false);
        UpdateWorker::instance()->setLocked(true);  // 截图录屏在Locked状态时会屏蔽一些锁屏时无法使用的功能（更新时同样无法使用）
    } else if (e->type() == QEvent::Hide) {
        UpdateWorker::instance()->enableShortcuts(true);
        UpdateWorker::instance()->setLocked(false);
    }

    return false;
}
