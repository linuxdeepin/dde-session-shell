// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "public_func.h"

#include "constants.h"

#include <QDBusConnection>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QGSettings>
#include <QTranslator>
#include <QJsonDocument>
#include <QApplication>
#include <QScreen>

#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>

using namespace std;

DCORE_USE_NAMESPACE

static int appType = APP_TYPE_LOCK;

//Load the System cursor --begin
static XcursorImages*
xcLoadImages(const char *image, int size)
{
    QSettings settings(DDESESSIONCC::DEFAULT_CURSOR_THEME, QSettings::IniFormat);
    //The default cursor theme path
    qDebug() << "Theme Path:" << DDESESSIONCC::DEFAULT_CURSOR_THEME;
    QString item = "Icon Theme";
    const char* defaultTheme = "";
    QVariant tmpCursorTheme = settings.value(item + "/Inherits");
    if (tmpCursorTheme.isNull()) {
        qDebug() << "Set a default one instead, if get the cursor theme failed!";
        defaultTheme = "Deepin";
    } else {
        defaultTheme = tmpCursorTheme.toString().toLocal8Bit().data();
    }

    qDebug() << "Get defaultTheme:" << tmpCursorTheme.isNull()
             << defaultTheme;
    return XcursorLibraryLoadImages(image, defaultTheme, size);
}

static unsigned long loadCursorHandle(Display *dpy, const char *name, int size)
{
    if (size == -1) {
        size = XcursorGetDefaultSize(dpy);
    }

    // Load the cursor images
    XcursorImages *images = nullptr;
    images = xcLoadImages(name, size);

    if (!images) {
        images = xcLoadImages(name, size);
        if (!images) {
            return 0;
        }
    }

    unsigned long handle = static_cast<unsigned long>(XcursorImagesLoadCursor(dpy,images));
    XcursorImagesDestroy(images);

    return handle;
}

int setRootWindowCursor() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        qDebug() << "Open display failed";
        return -1;
    }

    const char *cursorPath = qApp->devicePixelRatio() > 1.7
            ? "/usr/share/icons/bloom/cursors/loginspinner@2x"
            : "/usr/share/icons/bloom/cursors/loginspinner";

    Cursor cursor = static_cast<Cursor>(XcursorFilenameLoadCursor(display, cursorPath));
    if (cursor == 0) {
        cursor = static_cast<Cursor>(loadCursorHandle(display, "watch", 24));
    }
    XDefineCursor(display, XDefaultRootWindow(display),cursor);

    // XFixesChangeCursorByName is the key to change the cursor
    // and the XFreeCursor and XCloseDisplay is also essential.

    XFixesChangeCursorByName(display, cursor, "watch");

    XFreeCursor(display, cursor);
    XCloseDisplay(display);

    return 0;
}

// 初次启动时，设置一下鼠标的默认位置
void setPointer()
{
    auto set_position = [ = ] (QPoint p) {
        Display *dpy;
        dpy = XOpenDisplay(nullptr);

        if (!dpy) return;

        Window root_window;
        root_window = XRootWindow(dpy, 0);
        XSelectInput(dpy, root_window, KeyReleaseMask);
        XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, p.x(), p.y());
        XFlush(dpy);
    };

    if (!qApp->primaryScreen()) {
        QObject::connect(qApp, &QGuiApplication::primaryScreenChanged, [ = ] {
            static bool first = true;
            if (first) {
                set_position(qApp->primaryScreen()->geometry().center());
                first = false;
            }
        });
    } else {
        set_position(qApp->primaryScreen()->geometry().center());
    }
}

QPixmap loadPixmap(const QString &file, const QSize& size)
{
    qreal ratio = 1.0;
    qreal devicePixel = qApp->devicePixelRatio();

    QPixmap pixmap;

    if (!qFuzzyCompare(ratio, devicePixel) || size.isValid()) {
        QImageReader reader;
        reader.setDecideFormatFromContent(true);
        reader.setFileName(qt_findAtNxFile(file, devicePixel, &ratio));
        if (reader.canRead()) {
            reader.setScaledSize((size.isNull() ? reader.size() : reader.size().scaled(size, Qt::KeepAspectRatio)) * (devicePixel / ratio));
            pixmap = QPixmap::fromImage(reader.read());
            pixmap.setDevicePixelRatio(devicePixel);
        }
    } else {
        pixmap.load(file);
    }

    return pixmap;
}

void loadPixmap(const QString &fileName, QPixmap &pixmap)
{
    if (!pixmap.load(fileName)) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            pixmap.loadFromData(file.readAll());
        }
    }
}

bool checkPictureCanRead(const QString &fileName)
{
    QImageReader reader;
    reader.setDecideFormatFromContent(true);
    reader.setFileName(fileName);
    return reader.canRead();
}

/**
 * @brief 是否使用域管认证。
 *
 * @return true 使用域管认证
 * @return false 使用系统认证
 */
bool isDeepinAuth()
{
    const char* controlId = "com.deepin.dde.auth.control";
    if (QGSettings::isSchemaInstalled(controlId)) {
        const char *controlPath = "/com/deepin/dde/auth/control/";
        QGSettings controlObj(controlId, controlPath);
        const QString &key = "useDeepinAuth";
        bool useDeepinAuth = controlObj.keys().contains(key) && controlObj.get(key).toBool();
    #ifdef QT_DEBUG
        qDebug() << "use deepin auth: " << useDeepinAuth;
    #endif
        return useDeepinAuth;
    }
    return true;
}

uint timeFromString(QString time)
{
    if (time.isEmpty()) {
        return QDateTime::currentDateTime().toTime_t();
    }
    return QDateTime::fromString(time, Qt::ISODateWithMs).toLocalTime().toTime_t();
}

void setAppType(int type)
{
    appType = type;
}

QString getDefaultConfigFileName()
{
    return APP_TYPE_LOCK == appType ? DDESESSIONCC::LOCK_DCONFIG_SOURCE : DDESESSIONCC::LOGIN_DCONFIG_SOURCE;
}

/**
 * @brief 加载翻译文件
 *
 * @param locale
 */
void loadTranslation(const QString &locale)
{
    static QTranslator translator;
    static QString localTmp;
    if (localTmp == locale) {
        return;
    }
    localTmp = locale;
    qApp->removeTranslator(&translator);
    translator.load("/usr/share/dde-session-shell/translations/dde-session-shell_" + locale.split(".").first());
    qApp->installTranslator(&translator);
}

/**
 * @brief findSymLinTarget 查找软连接最终链接到的文件，如果不是软连接，直接返回原始路径
 * @param symLink 文件路径
 * @return 最终链接到的文件路径
 */
QString findSymLinTarget(const QString &symLink)
{
    QString file = symLink;
    QString tmpFile = symLink;
    while(!tmpFile.isEmpty()) {
        file = tmpFile;
        tmpFile = QFile::symLinkTarget(tmpFile);
    }

    return file;
}

/**
 * @brief json对象转为json字符串
 *
 * @param QJsonObject对象
 */
QString toJson(const QJsonObject &jsonObj)
{
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}

bool checkVersion(const QString &target, const QString &base)
{
    if (target == base) {
        return true;
    }
    const QStringList baseVersion = base.split(".");
    const QStringList targetVersion = target.split(".");

    const int baseVersionSize = baseVersion.size();
    const int targetVersionSize = targetVersion.size();
    const int size = baseVersionSize < targetVersionSize ? baseVersionSize : targetVersionSize;

    for (int i = 0; i < size; i++) {
        if (targetVersion[i] == baseVersion[i]) continue;
        return targetVersion[i].toInt() > baseVersion[i].toInt() ? true : false;
    }
    return true;
}
