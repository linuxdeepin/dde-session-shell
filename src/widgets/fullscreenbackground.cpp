// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fullscreenbackground.h"

#include "black_widget.h"
#include "public_func.h"
#include "sessionbasemodel.h"
#include "dconfig_helper.h"

#include <DGuiApplicationHelper>

#include <QDebug>
#include <QImageReader>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QWindow>

Q_LOGGING_CATEGORY(DDE_SS, "dss.active")

DGUI_USE_NAMESPACE

const int PIXMAP_TYPE_BACKGROUND = 0;
const int PIXMAP_TYPE_BLUR_BACKGROUND = 1;

QString FullScreenBackground::backgroundPath;
QString FullScreenBackground::blurBackgroundPath;

QList<QPair<QSize, QPixmap>> FullScreenBackground::backgroundCacheList;
QList<QPair<QSize, QPixmap>> FullScreenBackground::blurBackgroundCacheList;
QList<FullScreenBackground *> FullScreenBackground::frameList;
QPointer<FullScreenBackground> FullScreenBackground::currentFrame = nullptr;
QPointer<QWidget> FullScreenBackground::currentContent = nullptr;

FullScreenBackground::FullScreenBackground(SessionBaseModel *model, QWidget *parent)
    : QWidget(parent)
    , m_fadeOutAni(nullptr)
    , m_imageEffectInter(new ImageEffectInter("com.deepin.daemon.ImageEffect", "/com/deepin/daemon/ImageEffect", QDBusConnection::systemBus(), this))
    , m_model(model)
    , m_useSolidBackground(false)
    , m_fadeOutAniFinished(false)
    , m_enableAnimation(true)
    , m_blackWidget(new BlackWidget(this))
{
#ifndef QT_DEBUG
    if (!m_model->isUseWayland()) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    } else {
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Window);

        setAttribute(Qt::WA_NativeWindow); // 创建窗口 handle
        // onScreenDisplay 低于override，高于tooltip，希望显示在锁屏上方的界面，均需要调整层级为onScreenDisplay或者override
        windowHandle()->setProperty("_d_dwayland_window-type", "onScreenDisplay");
    }
#endif
    frameList.append(this);
    m_useSolidBackground = DConfigHelper::instance()->getConfig("useSolidBackground", false).toBool();
    m_enableAnimation = DGuiApplicationHelper::isSpecialEffectsEnvironment();
    if (m_enableAnimation && !m_useSolidBackground) {
        m_fadeOutAni = new QVariantAnimation(this);
        m_fadeOutAni->setEasingCurve(QEasingCurve::InOutCubic);
        m_fadeOutAni->setDuration(2000);
        m_fadeOutAni->setStartValue(0.0);
        m_fadeOutAni->setEndValue(1.0);

        connect(m_fadeOutAni, &QVariantAnimation::valueChanged, this, [this] {
            update();
            // 动画播放完毕后不在需要清晰的壁纸，释放资源
            if (m_fadeOutAni->currentValue() == 1.0) {
                backgroundCacheList.clear();
                backgroundPath = "";
                m_fadeOutAniFinished = true;
            }
        });
    }

    m_blackWidget->setBlackMode(m_model->isBlackMode());
    connect(m_model, &SessionBaseModel::blackModeChanged, this, [this](bool is_black) {
        m_blackWidget->setBlackMode(is_black);
        if (currentContent) {
            currentContent->show();
            currentContent->raise();
        }
    });
}

FullScreenBackground::~FullScreenBackground()
{
    frameList.removeAll(this);
}

void FullScreenBackground::updateBackground(const QString &path)
{
    if (m_useSolidBackground)
        return;

    if (isPicture(path)) {
        // 动画播放完毕不再需要清晰的背景图片
        if (!m_fadeOutAniFinished && !(backgroundPath == path && contains(PIXMAP_TYPE_BACKGROUND))) {
            backgroundPath = path;

            QPixmap pixmap;
            loadPixmap(backgroundPath, pixmap);
            addPixmap(pixmapHandle(pixmap), PIXMAP_TYPE_BACKGROUND);
        }

        // 需要播放动画的时候才更新模糊壁纸
        if (m_enableAnimation)
            updateBlurBackground(path);
    }
}

void FullScreenBackground::updateBlurBackground(const QString &path)
{
    QDBusPendingCall async = m_imageEffectInter->Get("", path);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call) {
        const QDBusPendingReply<QString> reply = *call;
        if (!reply.isError()) {
            QString blurPath = reply.value();

            if (!isPicture(blurPath)) {
                blurPath = "/usr/share/backgrounds/default_background.jpg";
            }

            if (blurBackgroundPath != blurPath || !contains(PIXMAP_TYPE_BLUR_BACKGROUND)) {
                blurBackgroundPath = blurPath;

                QPixmap pixmap;
                loadPixmap(blurBackgroundPath, pixmap);
                addPixmap(pixmapHandle(pixmap), PIXMAP_TYPE_BLUR_BACKGROUND);
            }

            // 只播放一次动画，后续背景图片变更直接更新模糊壁纸即可
            if (m_fadeOutAni && !m_fadeOutAniFinished)
                m_fadeOutAni->start();
            else
                update();
        } else {
            qWarning() << "get blur background image error: " << reply.error().message();
        }
        call->deleteLater();
    }, Qt::QueuedConnection);
}

bool FullScreenBackground::contentVisible() const
{
    return currentContent && currentContent->isVisible() && currentContent->parent() == this;
}

void FullScreenBackground::setEnterEnable(bool enable)
{
    m_enableEnterEvent = enable;
}

void FullScreenBackground::setScreen(QPointer<QScreen> screen, bool isVisible)
{
    if (screen.isNull()) {
        qWarning() << "Screen is nullptr";
        return;
    }

    qInfo() << "Screen:" << screen
            << ", screen geometry:" << screen->geometry()
            << ", full screen background object:" << this;
    if (isVisible) {
        updateCurrentFrame(this);
    } else {
        // 如果有多个屏幕则根据鼠标所在的屏幕来显示
        QTimer::singleShot(1000, this, [this] {
            setMouseTracking(true);
        });
    }

    updateScreen(screen);
}

void FullScreenBackground::setContent(QWidget *const w)
{
    if (!w) {
        qWarning() << "Content is null";
        return;
    }
    qInfo() << "Incoming  content:" << w << ", current content:" << currentContent;
    // 不重复设置content
    if (currentContent && currentContent->isVisible() && currentContent == w && currentFrame && w->parent() == currentFrame) {
        qDebug() << "Parent is current frame";
        return;
    }

    if (!currentContent.isNull()) {
        currentContent->setParent(nullptr);
        currentContent->hide();
    }
    currentContent = w;
    if (!currentFrame) {
        qWarning() << "Current frame is null";
        return;
    }

    currentContent->setParent(currentFrame);
    currentContent->move(0, 0);
    currentContent->resize(currentFrame->size());
    currentFrame->setFocusProxy(currentContent);
    currentFrame->setFocus();
    currentFrame->activateWindow();
    // 如果是黑屏状态则不置顶
    if (!currentFrame->m_blackWidget->isVisible()) {
        currentContent->raise();
        currentContent->show();
    };
}

void FullScreenBackground::setIsHibernateMode()
{
    updateGeometry();
    updateCurrentFrame(this);
}

bool FullScreenBackground::isPicture(const QString &file)
{
    return QFile::exists(file) && QFile(file).size() && checkPictureCanRead(file);
}

QString FullScreenBackground::getLocalFile(const QString &file)
{
    const QUrl url(file);
    return url.isLocalFile() ? url.toLocalFile() : url.url();
}

void FullScreenBackground::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const QPixmap &background = getPixmap(PIXMAP_TYPE_BACKGROUND);
    const QPixmap &blurBackground = getPixmap(PIXMAP_TYPE_BLUR_BACKGROUND);

    const QRect trueRect(QPoint(0, 0), QSize(size() * devicePixelRatioF()));
    if (m_useSolidBackground) {
        painter.fillRect(trueRect, QColor(DDESESSIONCC::SOLID_BACKGROUND_COLOR));
    } else {
        if (m_fadeOutAni) {
            const double currentAniValue = m_fadeOutAni->currentValue().toDouble();
            if (!background.isNull() && currentAniValue != 1.0) {
                painter.setOpacity(1);
                painter.drawPixmap(trueRect,
                                background,
                                QRect(trueRect.topLeft(), trueRect.size() * background.devicePixelRatioF()));
            }

            if (!blurBackground.isNull()) {
                painter.setOpacity(currentAniValue);
                painter.drawPixmap(trueRect,
                                blurBackground,
                                QRect(trueRect.topLeft(), trueRect.size() * blurBackground.devicePixelRatioF()));
            }
        } else {
            // 不使用的动画的过渡的时候直接绘制清晰的背景
            if (!background.isNull()) {
                painter.drawPixmap(trueRect,
                                background,
                                QRect(trueRect.topLeft(), trueRect.size() * background.devicePixelRatioF()));
            }
        }
    }
}

void FullScreenBackground::tryActiveWindow(int count/* = 9*/)
{
    if (count < 0 || m_model->isUseWayland())
        return;

    qCDebug(DDE_SS) << "try active window..." << count;
    if (isActiveWindow()) {
        qCDebug(DDE_SS) << "...finally activewindow is me ...";
        return;
    }

    activateWindow();

    if (currentContent && !currentContent->isVisible()) {
        qCDebug(DDE_SS) << "hide..." << count;
        return;
    }
    QTimer::singleShot(50 , this, std::bind(&FullScreenBackground::tryActiveWindow, this, count -1));
}

void FullScreenBackground::enterEvent(QEvent *event)
{
    qInfo() << Q_FUNC_INFO << this << ", m_enableEnterEvent: " << m_enableEnterEvent;
    if (m_enableEnterEvent && m_model->visible()) {
        updateCurrentFrame(this);
        // 多屏情况下，此Frame晚于其它Frame显示出来时，可能处于未激活状态（特别是在wayland环境下比较明显）
        activateWindow();
    }

    // 锁屏截图之后 active window 不是锁屏了，此时发现不是 active window 主动尝试激活
    if (!isActiveWindow() && contentVisible())
        tryActiveWindow();

    return QWidget::enterEvent(event);
}

void FullScreenBackground::leaveEvent(QEvent *event)
{
    return QWidget::leaveEvent(event);
}

void FullScreenBackground::resizeEvent(QResizeEvent *event)
{
    if (isPicture(backgroundPath) && !contains(PIXMAP_TYPE_BACKGROUND)) {
        QPixmap pixmap;
        loadPixmap(backgroundPath, pixmap);
        addPixmap(pixmapHandle(pixmap), PIXMAP_TYPE_BACKGROUND);
    }
    if (isPicture(blurBackgroundPath) && !contains(PIXMAP_TYPE_BLUR_BACKGROUND)) {
        QPixmap pixmap;
        loadPixmap(blurBackgroundPath, pixmap);
        addPixmap(pixmapHandle(QPixmap(blurBackgroundPath)), PIXMAP_TYPE_BLUR_BACKGROUND);
    }

    updatePixmap();

    m_blackWidget->resize(size());
    if (currentFrame == this && currentContent) {
        currentContent->resize(size());
    }

    QWidget::resizeEvent(event);
}

/**
 * @brief 鼠标移动触发这个事件，可能需要配合setMouseTracking(true)使用
 *
 * @param event
 */
void FullScreenBackground::mouseMoveEvent(QMouseEvent *event)
{
    if (m_model->visible() && m_enableEnterEvent) {
        updateCurrentFrame(this);
    }

    QWidget::mouseMoveEvent(event);
}

void FullScreenBackground::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);

    switch (e->key()) {
#ifdef QT_DEBUG
    case Qt::Key_Escape:
        qApp->quit();
        break;
#endif
    default:
        break;
    }
}

void FullScreenBackground::showEvent(QShowEvent *event)
{
    qInfo() << "Frame is already displayed:" << this;
    if (m_model->isUseWayland()) {
        // fix bug 155019 此处是针对wayland下屏保退出，因qt没有发送enterEvent事件，进而导致密码框没有显示的规避方案
        // 如果鼠标的位置在当前屏幕内且锁屏状态为显示，将密码框显示出来
        if (m_model->visible() && geometry().contains(QCursor::pos())) {
            currentContent->show();
            // 多屏情况下，此Frame晚于其它Frame显示出来时，可能处于未激活状态（特别是在wayland环境下比较明显）
            activateWindow();
        }

        Q_EMIT requestDisableGlobalShortcutsForWayland(true);
    }

    updateGeometry();
    // 显示的时候需要置顶，截图在上方的话无法显示锁屏 见Bug-140545
    if (contentVisible())
        raise();

    m_blackWidget->setBlackMode(m_model->isBlackMode());

    return QWidget::showEvent(event);
}

void FullScreenBackground::hideEvent(QHideEvent *event)
{
    qInfo() << "Frame is hidden:" << this;
    if (m_model->isUseWayland()) {
        Q_EMIT requestDisableGlobalShortcutsForWayland(false);
    }

    QWidget::hideEvent(event);
}

const QPixmap FullScreenBackground::pixmapHandle(const QPixmap &pixmap)
{
    const QSize trueSize {size() * devicePixelRatioF()};
    QPixmap pix;
    if (!pixmap.isNull())
        pix = pixmap.scaled(trueSize, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);

    pix = pix.copy(QRect((pix.width() - trueSize.width()) / 2,
                         (pix.height() - trueSize.height()) / 2,
                         trueSize.width(),
                         trueSize.height()));

    // draw pix to widget, so pix need set pixel ratio from qwidget devicepixelratioF
    pix.setDevicePixelRatio(devicePixelRatioF());

    return pix;
}

void FullScreenBackground::updateScreen(QPointer<QScreen> screen)
{
    if (screen == m_screen)
        return;

    if (!m_screen.isNull())
        disconnect(m_screen, &QScreen::geometryChanged, this, &FullScreenBackground::updateGeometry);

    if (!screen.isNull()) {
        connect(screen, &QScreen::geometryChanged, this, &FullScreenBackground::updateGeometry, Qt::ConnectionType::QueuedConnection);
    }

    m_screen = screen;

    updateGeometry();
}

void FullScreenBackground::updateGeometry()
{
    if (m_screen.isNull()) {
        return;
    }

    qInfo() << "set background screen:" << m_screen;

    // for bug:184943.系统修改分辨率后，登录界面获取的屏幕分辨率不正确,通过xrandr获取屏幕分辨率
    if (m_model->appType() == AuthCommon::Login && !m_model->isUseWayland()) {
        static QMap<QString, QRect> screensGeometry;
        if (screensGeometry.isEmpty() || !screensGeometry.contains(m_screen->name())) {
            screensGeometry = getScreenGeometryByXrandr();
            qInfo() << "getScreenGeometryByXrandr:" << screensGeometry;
        }

        if (screensGeometry.contains(m_screen->name())) {
            setGeometry(screensGeometry[m_screen->name()]);
            qInfo() << "set geometry by xrandr rect:" << screensGeometry[m_screen->name()];
        } else {
            setGeometry(m_screen->geometry());
        }

        return;
    }

    if (!m_screen.isNull()) {
        setGeometry(m_screen->geometry());

        qInfo() << "Screen:" << m_screen
                << ", screen geometry:" << m_screen->geometry()
                << ", lockFrame:" << this
                << ", frame geometry:" << this->geometry();
    } else {
        qWarning() << "Screen is nullptr";
    }
}

/********************************************************
 * 监听主窗体属性。
 * 用户登录界面，主窗体在某时刻会被设置为WindowDeactivate，
 * 此时登录界面获取不到焦点，需要调用requestActivate激活窗体。
********************************************************/
bool FullScreenBackground::event(QEvent *e)
{
#ifndef QT_DEBUG
    if (e->type() == QEvent::WindowDeactivate) {
        if (contentVisible()) {
            tryActiveWindow();
        }
    }
#endif
    return QWidget::event(e);
}

const QPixmap& FullScreenBackground::getPixmap(int type)
{
    static QPixmap pixmap;

    const QSize &size = trueSize();
    auto findPixmap = [size](QList<QPair<QSize, QPixmap>> &list) -> const QPixmap & {
        auto it = std::find_if(list.begin(), list.end(),
                               [size](QPair<QSize, QPixmap> pair) { return pair.first == size; });
        return it != list.end() ? it.i->t().second : pixmap;
    };

    if (PIXMAP_TYPE_BACKGROUND == type)
        return findPixmap(backgroundCacheList);
    else
        return findPixmap(blurBackgroundCacheList);
}

QSize FullScreenBackground::trueSize() const
{
    return size() * devicePixelRatioF();
}

/**
 * @brief FullScreenBackground::addPixmap
 * 新增pixmap，存在相同size的则替换，没有则新增
 * @param pixmap pixmap对象
 * @param type 清晰壁纸还是模糊壁纸
 */
void FullScreenBackground::addPixmap(const QPixmap &pixmap, const int type)
{
    const QSize &size = trueSize();
    auto addFunc = [this, &pixmap, size](QList<QPair<QSize, QPixmap>> &list){
        bool exist = false;
        for (auto &pair : list) {
            if (pair.first == size) {
                pair.second = pixmap;
                exist = true;
            }
        }
        if (!exist)
            list.append(QPair<QSize, QPixmap>(trueSize(), pixmap));
    };

    if (PIXMAP_TYPE_BACKGROUND == type)
        addFunc(backgroundCacheList);
    else
        addFunc(blurBackgroundCacheList);
}

/**
 * @brief FullScreenBackground::updatePixmap
 * 更新壁纸列表，移除当前所有屏幕都不会使用的壁纸数据
 */
void FullScreenBackground::updatePixmap()
{
    auto isNoUseFunc = [](const QSize &size) -> bool {
        bool b = std::any_of(frameList.begin(), frameList.end(), [size](FullScreenBackground *frame) {
            return frame->trueSize() == size;
        });
        return !b;
    };

    auto updateFunc = [ isNoUseFunc ](QList<QPair<QSize, QPixmap>> &list) {
        for (auto &pair : list) {
            if (isNoUseFunc(pair.first))
                list.removeAll(pair);
        }
    };

    updateFunc(backgroundCacheList);
    updateFunc(blurBackgroundCacheList);
}

bool FullScreenBackground::contains(int type)
{
    auto containsFunc = [this](QList<QPair<QSize, QPixmap>> &list) -> bool {
        bool b = std::any_of(list.begin(), list.end(), [this](QPair<QSize, QPixmap> &pair) {
            return pair.first == trueSize();
        });
        return b;
    };

    if (PIXMAP_TYPE_BACKGROUND == type)
        return containsFunc(backgroundCacheList);
    else
        return containsFunc(blurBackgroundCacheList);
}

void FullScreenBackground::moveEvent(QMoveEvent *event)
{
    // 规避 bug189309，登录时后端屏幕变化通知太晚
    if (currentContent && !currentContent->isVisible()) {
        if (m_model->isUseWayland()) {
            if (m_model->visible() && geometry().contains(QCursor::pos())) {
                currentContent->show();
            }
        }
    }
    qInfo() << "FullscreenBackground::moveEvent: " << ", old pos: " << event->oldPos() << ", pos: " << event->pos();
    QWidget::moveEvent(event);
}

QMap<QString, QRect> FullScreenBackground::getScreenGeometryByXrandr()
{
    QMap<QString, QRect> screensGeometry;

    // 启动 xrandr | grep connected 进程
    QProcess process;
    QStringList arguments;
    arguments << "-c" << "xrandr | grep connected";
    process.start("/bin/sh", arguments);
    process.waitForStarted();
    process.waitForFinished();

    // 读取进程输出并解析屏幕信息
    QString output = QString::fromLocal8Bit(process.readAll());
    QRegularExpression regex("(\\S+) connected (?:primary )?(\\d+)x(\\d+)\\+(\\d+)\\+(\\d+)");
    QRegularExpressionMatchIterator iter = regex.globalMatch(output);
    while (iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        if (match.hasMatch()) {
            QString name = match.captured(1);
            int width = match.captured(2).toInt();
            int height = match.captured(3).toInt();
            int x = match.captured(4).toInt();
            int y = match.captured(5).toInt();
            QRect rect(x, y, width, height);
            screensGeometry.insert(name, rect);
            qInfo() << "Screen Name:" << name << "Screen Rect:" << rect;
        }
    }

    return screensGeometry;
}

void FullScreenBackground::updateCurrentFrame(FullScreenBackground *frame)
{
    if (!frame) {
        qWarning() << "Frame is nullptr";
        return;
    }

    if (frame->m_screen)
        qInfo() << "Frame:" << frame << ", screen:" << frame->m_screen->name();

    currentFrame = frame;
    setContent(currentContent);
}
