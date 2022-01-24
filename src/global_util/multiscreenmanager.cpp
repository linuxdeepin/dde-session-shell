#include "multiscreenmanager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonObject>

MultiScreenManager::MultiScreenManager(QObject *parent)
    : QObject(parent)
    , m_registerFunction(nullptr)
    , m_raiseContentFrameTimer(new QTimer(this))
    , m_systemDisplay(new SystemDisplayInter("com.deepin.system.Display", "/com/deepin/system/Display", QDBusConnection::systemBus(), this))
    , m_isCopyMode(false)
{
    connect(qApp, &QGuiApplication::screenAdded, this, &MultiScreenManager::onScreenAdded, Qt::DirectConnection);
    connect(qApp, &QGuiApplication::screenRemoved, this, &MultiScreenManager::onScreenRemoved, Qt::DirectConnection);

    // 在sw平台存在复制模式显示问题，使用延迟来置顶一个Frame
    m_raiseContentFrameTimer->setInterval(50);
    m_raiseContentFrameTimer->setSingleShot(true);

    connect(m_raiseContentFrameTimer, &QTimer::timeout, this, &MultiScreenManager::raiseContentFrame);
    connect(m_systemDisplay, &SystemDisplayInter::ConfigUpdated, this, &MultiScreenManager::onDisplayModeChanged);
    if (m_systemDisplay->isValid()) {
        m_isCopyMode = (COPY_MODE == getDisplayModeByConfig(m_systemDisplay->GetConfig()));
    }
}

void MultiScreenManager::register_for_mutil_screen(std::function<QWidget *(QScreen *, int)> function)
{
    m_registerFunction = function;

    // update all screen
    if (m_isCopyMode) {
        if (!qApp->screens().isEmpty())
            onScreenAdded(qApp->screens().first());
    } else {
        for (QScreen *screen : qApp->screens()) {
            onScreenAdded(screen);
        }
    }
}

void MultiScreenManager::startRaiseContentFrame()
{
    m_raiseContentFrameTimer->start();
}

void MultiScreenManager::onScreenAdded(QScreen *screen)
{
    if (!m_registerFunction) {
        return;
    }

    if (m_isCopyMode) {
        // 如果m_frames不为空则直接退出
        if (m_frames.isEmpty()) {
            m_frames[screen] = m_registerFunction(screen, m_frames.size());
        }
    } else {
        m_frames[screen] = m_registerFunction(screen, m_frames.size());
    }

    startRaiseContentFrame();
}

void MultiScreenManager::onScreenRemoved(QScreen *screen)
{
    if (!m_registerFunction) {
        return;
    }

    qDebug() << Q_FUNC_INFO << " is copy mode: " << m_isCopyMode;
    if (m_isCopyMode) {
        if (m_frames.contains(screen)) {
            QWidget *frame = m_frames[screen];
            m_frames.remove(screen);
            // 如果此时m_frames为空则其它的屏幕继续使用此frame，不重新创建
            if (!qApp->screens().isEmpty() && m_frames.isEmpty())
                m_frames[qApp->screens().first()] = frame;
            else
                frame->deleteLater();
        }
    } else {
        m_frames[screen]->deleteLater();
        m_frames.remove(screen);
    }

    startRaiseContentFrame();
}

void MultiScreenManager::raiseContentFrame()
{
    for (auto it = m_frames.constBegin(); it != m_frames.constEnd(); ++it) {
        if (it.value()->property("contentVisible").toBool()) {
            it.value()->raise();
            return;
        }
    }
}

void MultiScreenManager::onDisplayModeChanged(const QString &)
{
    m_isCopyMode = (COPY_MODE == getDisplayModeByConfig(m_systemDisplay->GetConfig()));
    if (m_isCopyMode) {
        for (QScreen *screen : qApp->screens()) {
            // 留下一个屏即可
            if (screen != qApp->screens().first())
                onScreenRemoved(screen);
        }
    } else {
        for (QScreen *screen : qApp->screens()) {
            if (!m_frames[screen])
                onScreenAdded(screen);
        }
    }
}

int MultiScreenManager::getDisplayModeByConfig(const QString &config) const
{
    if (config.isEmpty())
        return EXTENDED_MODE;

    const QJsonObject &rootObj = QJsonDocument::fromJson(config.toUtf8()).object();
    if (rootObj.contains("Config")) {
        const QJsonObject &configObj = rootObj.value("Config").toObject();
        if (configObj.contains("DisplayMode")) {
            return configObj.value("DisplayMode").toInt();
        }
    }

    return EXTENDED_MODE;
}