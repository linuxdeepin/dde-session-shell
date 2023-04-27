// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLSCREENBACKGROUND_H
#define FULLSCREENBACKGROUND_H

#include <QWidget>
#include <QSharedPointer>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(DDE_SS)

#include <com_deepin_daemon_imageeffect.h>

using ImageEffectInter = com::deepin::daemon::ImageEffect;

class BlackWidget;
class SessionBaseModel;
class FullScreenBackground : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool contentVisible READ contentVisible)

public:
    explicit FullScreenBackground(SessionBaseModel *model, QWidget *parent = nullptr);
    ~FullScreenBackground() override;

    bool contentVisible() const;
    void setEnterEnable(bool enable);
    static void setContent(QWidget *const w);

public slots:
    void updateBackground(const QString &path);
    void updateBlurBackground(const QString &path);
    void setScreen(QPointer<QScreen> screen, bool isVisible = true);
    void setIsHibernateMode();

signals:
    void requestDisableGlobalShortcutsForWayland(bool enable);
    void requestLockFrameHide();

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool event(QEvent *e) override;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void moveEvent(QMoveEvent *event) Q_DECL_OVERRIDE;

private:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    const QPixmap pixmapHandle(const QPixmap &pixmap);
    void updateScreen(QPointer<QScreen> screen);
    void updateGeometry();
    bool isPicture(const QString &file);
    QString getLocalFile(const QString &file);
    const QPixmap& getPixmap(int type);
    QSize trueSize() const;
    void addPixmap(const QPixmap &pixmap, const int type);
    static void updatePixmap();
    bool contains(int type);
    void tryActiveWindow(int count = 9);
    static void updateCurrentFrame(FullScreenBackground *frame);

protected:
    static QPointer<QWidget> currentContent;
    static QList<FullScreenBackground *> frameList;
    static QPointer<FullScreenBackground> currentFrame;

private:
    static QString backgroundPath;                             // 高清背景图片路径
    static QString blurBackgroundPath;                         // 模糊背景图片路径
    static QList<QPair<QSize, QPixmap>> backgroundCacheList;
    static QList<QPair<QSize, QPixmap>> blurBackgroundCacheList;

    QVariantAnimation *m_fadeOutAni;      // 背景动画
    ImageEffectInter *m_imageEffectInter; // 获取模糊背景服务
    QPointer<QScreen> m_screen;
    SessionBaseModel *m_model = nullptr;
    bool m_enableEnterEvent = true;
    bool m_useSolidBackground;
    bool m_fadeOutAniFinished;
    bool m_enableAnimation;

    BlackWidget *m_blackWidget;
};

#endif // FULLSCREENBACKGROUND_H
