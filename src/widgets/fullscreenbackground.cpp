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

#include "fullscreenbackground.h"
#include "sessionbasemodel.h"
#include "public_func.h"

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

QString FullscreenBackground::backgroundPath;
QString FullscreenBackground::blurBackgroundPath;

QList<QPair<QSize, QPixmap>> FullscreenBackground::backgroundCacheList;
QList<QPair<QSize, QPixmap>> FullscreenBackground::blurBackgroundCacheList;
QList<FullscreenBackground *> FullscreenBackground::frameList;

FullscreenBackground::FullscreenBackground(SessionBaseModel *model, QWidget *parent)
    : QWidget(parent)
    , m_fadeOutAni(nullptr)
    , m_imageEffectInter(new ImageEffectInter("com.deepin.daemon.ImageEffect", "/com/deepin/daemon/ImageEffect", QDBusConnection::systemBus(), this))
    , m_model(model)
    , m_originalCursor(cursor())
    , m_useSolidBackground(false)
    , m_fadeOutAniFinished(false)
    , m_enableAnimation(true)
{
#ifndef QT_DEBUG
    if (!qEnvironmentVariable("XDG_SESSION_TYPE").contains("wayland")) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    } else {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Window);

        setAttribute(Qt::WA_NativeWindow); // 创建窗口 handle
        windowHandle()->setProperty("_d_dwayland_window-type", "standAlone"); // 禁止移动
    }
#endif
    frameList.append(this);
    m_useSolidBackground = getDConfigValue(getDefaultConfigFileName(), "use-solid-background", false).toBool();
    m_enableAnimation = DGuiApplicationHelper::isSpecialEffectsEnvironment();
    if (m_enableAnimation && !m_useSolidBackground) {
        m_fadeOutAni = new QVariantAnimation(this);
        m_fadeOutAni->setEasingCurve(QEasingCurve::InOutCubic);
        m_fadeOutAni->setDuration(2000);
        m_fadeOutAni->setStartValue(0.0);
        m_fadeOutAni->setEndValue(1.0);

        connect(m_fadeOutAni, &QVariantAnimation::valueChanged, this, [ this ] {
            update();
            // 动画播放完毕后不在需要清晰的壁纸，释放资源
            if (m_fadeOutAni->currentValue() == 1.0) {
                backgroundCacheList.clear();
                backgroundPath = "";
                m_fadeOutAniFinished = true;
            }
        });
    }

    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [this](QScreen *screen) {
        qDebug() << "QGuiApplication::primaryScreenChanged:" << m_screen << screen << m_content;
        if (m_screen && m_screen == screen && m_content) {
            m_primaryShowFinished = true;
        }
    });
}

FullscreenBackground::~FullscreenBackground()
{
    frameList.removeAll(this);
}

void FullscreenBackground::updateBackground(const QString &path)
{
    if (m_useSolidBackground)
        return;

    if (isPicture(path)) {
        // 动画播放完毕不再需要清晰的背景图片
        if (!m_fadeOutAniFinished) {
            backgroundPath = path;
            addPixmap(pixmapHandle(QPixmap(path)), PIXMAP_TYPE_BACKGROUND);
        }

        // 需要播放动画的时候才更新模糊壁纸
        if (m_enableAnimation)
            updateBlurBackground(path);
    }
}

void FullscreenBackground::updateBlurBackground(const QString &path)
{
    QDBusPendingCall async = m_imageEffectInter->Get("", path);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *call) {
        const QDBusPendingReply<QString> reply = *call;
        if (!reply.isError()) {
            QString blurPath = reply.value();
            if (!isPicture(blurPath)) {
                blurPath = "/usr/share/backgrounds/default_background.jpg";
            }
            blurBackgroundPath = blurPath;
            addPixmap(pixmapHandle(QPixmap(blurPath)), PIXMAP_TYPE_BLUR_BACKGROUND);
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

bool FullscreenBackground::contentVisible() const
{
    return m_content && m_content->isVisible();
}

void FullscreenBackground::setEnterEnable(bool enable)
{
    m_enableEnterEvent = enable;
}

void FullscreenBackground::setScreen(QScreen *screen, bool isVisible)
{
    QScreen *primary_screen = QGuiApplication::primaryScreen();
    if (primary_screen == screen && !m_isBlackMode && isVisible) {
        m_content->show();
        m_primaryShowFinished = true;
        emit contentVisibleChanged(true);
    } else {
        QTimer::singleShot(1000, this, [=] {
            m_primaryShowFinished = true;
            setMouseTracking(true);
        });
    }

    updateScreen(screen);
}

void FullscreenBackground::setContentVisible(bool visible)
{
    if (this->contentVisible() == visible)
        return;

    if (!m_content)
        return;

    if (!isVisible() && !visible)
        return;

    m_content->setVisible(visible && !m_isBlackMode);

    emit contentVisibleChanged(visible && !m_isBlackMode);
}

void FullscreenBackground::setContent(QWidget *const w)
{
    m_content = w;
    m_content->setParent(this);
    m_content->raise();
    m_content->move(0, 0);
    m_content->setFocus();
    setFocusProxy(m_content);
}

void FullscreenBackground::setIsBlackMode(bool isBlack)
{
    if (m_isBlackMode == isBlack)
        return;

    //黑屏的同时隐藏鼠标,唤醒后显示鼠标
    setCursor(isBlack ? Qt::BlankCursor : m_originalCursor);

    //黑屏鼠标在多屏间移动时不显示界面
    m_enableEnterEvent = !isBlack;
    m_isBlackMode = isBlack;

    m_content->setVisible(!m_isBlackMode);
    emit contentVisibleChanged(!m_isBlackMode);

    update();
    raise();
}

void FullscreenBackground::setIsHibernateMode()
{
    updateGeometry();
    m_content->show();
    emit contentVisibleChanged(true);
}

bool FullscreenBackground::isPicture(const QString &file)
{
    return QFile::exists(file) && QFile(file).size() && QImageReader(file).canRead();
}

QString FullscreenBackground::getLocalFile(const QString &file)
{
    const QUrl url(file);
    return url.isLocalFile() ? url.toLocalFile() : url.url();
}

void FullscreenBackground::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const QPixmap &background = getPixmap(PIXMAP_TYPE_BACKGROUND);
    const QPixmap &blurBackground = getPixmap(PIXMAP_TYPE_BLUR_BACKGROUND);

    const QRect trueRect(QPoint(0, 0), QSize(size() * devicePixelRatioF()));
    if (m_isBlackMode) {
        painter.fillRect(trueRect, Qt::black);
    } else if (m_useSolidBackground) {
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

void FullscreenBackground::tryActiveWindow(int count/* = 9*/)
{
    if (count < 0 || qgetenv("XDG_SESSION_TYPE").contains("wayland"))
        return;

    qCDebug(DDE_SS) << "try active window..." << count;
    if (isActiveWindow()) {
        qCDebug(DDE_SS) << "...finally activewindow is me ...";
        return;
    }

    activateWindow();

    if (m_content && !m_content->isVisible()) {
        qCDebug(DDE_SS) << "hide..." << count;
        return;
    }
    QTimer::singleShot(50 , this, std::bind(&FullscreenBackground::tryActiveWindow, this, count -1));
}

void FullscreenBackground::enterEvent(QEvent *event)
{
    if (m_primaryShowFinished && m_enableEnterEvent && m_model->visible()) {
        m_content->show();
        emit contentVisibleChanged(true);
    }

    // 锁屏截图之后 activewindow 不是锁屏了，此时发现不是 activewindow 主动尝试激活
    if (!isActiveWindow() && m_content && m_content->isVisible())
         tryActiveWindow();

    return QWidget::enterEvent(event);
}

void FullscreenBackground::leaveEvent(QEvent *event)
{
    return QWidget::leaveEvent(event);
}

void FullscreenBackground::resizeEvent(QResizeEvent *event)
{
    m_content->resize(size());
    if (isPicture(backgroundPath) && !contains(PIXMAP_TYPE_BACKGROUND))
        addPixmap(pixmapHandle(QPixmap(backgroundPath)), PIXMAP_TYPE_BACKGROUND);
    if (isPicture(blurBackgroundPath) && !contains(PIXMAP_TYPE_BLUR_BACKGROUND))
        addPixmap(pixmapHandle(QPixmap(blurBackgroundPath)), PIXMAP_TYPE_BLUR_BACKGROUND);

    updatePixmap();
    QWidget::resizeEvent(event);
}

/**
 * @brief 鼠标移动触发这个事件，可能需要配合setMouseTracking(true)使用
 *
 * @param event
 */
void FullscreenBackground::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isBlackMode && m_model->visible()) {
        m_content->show();
        emit contentVisibleChanged(true);
    }

    return QWidget::mouseMoveEvent(event);
}

void FullscreenBackground::keyPressEvent(QKeyEvent *e)
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

void FullscreenBackground::showEvent(QShowEvent *event)
{
    bool isWayLand = qgetenv("XDG_SESSION_TYPE").contains("wayland");
    if (QWindow *w = windowHandle()) {
        if (m_screen) {
            if (w->screen() != m_screen) {
                w->setScreen(m_screen);
            }

            // 更新窗口位置和大小
            updateGeometry();
        } else {
            updateScreen(w->screen());
        }

        if (!isWayLand) {
            connect(w, &QWindow::screenChanged, this, &FullscreenBackground::updateScreen, Qt::DirectConnection);
        }
    }

    return QWidget::showEvent(event);
}

const QPixmap FullscreenBackground::pixmapHandle(const QPixmap &pixmap)
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

void FullscreenBackground::updateScreen(QScreen *screen)
{
    if (screen == m_screen)
        return;

    if (m_screen)
        disconnect(m_screen, &QScreen::geometryChanged, this, &FullscreenBackground::updateGeometry);

    if (screen)
        connect(screen, &QScreen::geometryChanged, this, &FullscreenBackground::updateGeometry);

    m_screen = screen;
    if (m_screen)
        updateGeometry();
}

void FullscreenBackground::updateGeometry()
{
    setGeometry(m_screen->geometry());
}

/********************************************************
 * 监听主窗体属性。
 * 用户登录界面，主窗体在某时刻会被设置为WindowDeactivate，
 * 此时登录界面获取不到焦点，需要调用requestActivate激活窗体。
********************************************************/
bool FullscreenBackground::event(QEvent *e)
{
#ifndef QT_DEBUG
    if (e->type() == QEvent::WindowDeactivate) {
        if (m_content->isVisible()) {
            tryActiveWindow();
        }
    }
#endif
    return QWidget::event(e);
}

const QPixmap& FullscreenBackground::getPixmap(int type)
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

QSize FullscreenBackground::trueSize() const
{
    return size() * devicePixelRatioF();
}

/**
 * @brief FullscreenBackground::addPixmap
 * 新增pixmap，存在相同size的则替换，没有则新增
 * @param pixmap pixmap对象
 * @param type 清晰壁纸还是模糊壁纸
 */
void FullscreenBackground::addPixmap(const QPixmap &pixmap, const int type)
{
    const QSize &size = trueSize();
    auto addFunc = [ this, &pixmap, size ] (QList<QPair<QSize, QPixmap>> &list){
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
 * @brief FullscreenBackground::updatePixmap
 * 更新壁纸列表，移除当前所有屏幕都不会使用的壁纸数据
 */
void FullscreenBackground::updatePixmap()
{
    auto isNoUsefunc = [](const QSize &size) -> bool {
        bool b = std::any_of(frameList.begin(), frameList.end(), [size](FullscreenBackground *frame) {
            return frame->trueSize() == size;
        });
        return !b;
    };

    auto updateFunc = [ isNoUsefunc ] (QList<QPair<QSize, QPixmap>> &list) {
        for (auto &pair : list) {
            if (isNoUsefunc(pair.first))
                list.removeAll(pair);
        }
    };

    updateFunc(backgroundCacheList);
    updateFunc(blurBackgroundCacheList);
}

bool FullscreenBackground::contains(int type)
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
