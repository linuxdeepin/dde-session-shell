// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DConfig>
#include <QDBusInterface>
#include <QDBusReply>
#include <QJsonParseError>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QProcess>

#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>
#include <cmath>
#include <iostream>

DCORE_USE_NAMESPACE

const bool IsWayland = qgetenv("XDG_SESSION_TYPE").contains("wayland");

// Load system cursor --end

bool isScaleConfigExists() {
    QDBusInterface configInter("com.deepin.system.Display",
                                                     "/com/deepin/system/Display",
                                                     "com.deepin.system.Display",
                                                    QDBusConnection::systemBus());
    if (!configInter.isValid()) {
        return false;
    }
    QDBusReply<QString> configReply = configInter.call("GetConfig");
    if (configReply.isValid()) {
        QString config = configReply.value();
        QJsonParseError jsonError;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(config.toStdString().data(), &jsonError));
        if (jsonError.error == QJsonParseError::NoError) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonObject Config = rootObj.value("Config").toObject();
            return !Config.value("ScaleFactors").toObject().isEmpty();
        } else {
            return false;
        }
    } else {
        qWarning() << "Call `GetConfig` failed: " << configReply.error().message();
        return false;
    }
}

// 参照后端算法，保持一致
float toListedScaleFactor(float s)  {
	const float min = 1.0, max = 3.0, step = 0.25;
	if (s <= min) {
		return min;
	} else if (s >= max) {
		return max;
	}

	for (float i = min; i <= max; i += step) {
		if (i > s) {
			float ii = i - step;
			float d1 = s - ii;
			float d2 = i - s;
			if (d1 >= d2) {
				return i;
			} else {
				return ii;
			}
		}
	}
	return max;
}

static double getScaleFactor() {
    Display *display = XOpenDisplay(nullptr);
    double scaleFactor = 0.0;

    if (!display) {
        return scaleFactor;
    }

    XRRScreenResources *resources = XRRGetScreenResourcesCurrent(display, DefaultRootWindow(display));

    if (!resources) {
        resources = XRRGetScreenResources(display, DefaultRootWindow(display));
        qWarning() << "Get current XRR screen resources failed, instead of getting XRR screen resources, resources: " << resources;
    }

    if (resources) {
        for (int i = 0; i < resources->noutput; i++) {
            XRROutputInfo* outputInfo = XRRGetOutputInfo(display, resources, resources->outputs[i]);
            if (outputInfo->crtc == 0 || outputInfo->mm_width == 0) continue;

            XRRCrtcInfo *crtInfo = XRRGetCrtcInfo(display, resources, outputInfo->crtc);
            if (crtInfo == nullptr) continue;
            // 参照后端的算法，与后端保持一致
            float lenPx = hypot(static_cast<double>(crtInfo->width), static_cast<double>(crtInfo->height));
            float lenMm = hypot(static_cast<double>(outputInfo->mm_width), static_cast<double>(outputInfo->mm_height));
            float lenPxStd = hypot(1920, 1080);
            float lenMmStd = hypot(477, 268);
            float fix = (lenMm - lenMmStd) * (lenPx / lenPxStd) * 0.00158;
            float scale = (lenPx / lenMm) / (lenPxStd / lenMmStd) + fix;
            scaleFactor = toListedScaleFactor(scale);
        }
    }
    else {
        qWarning() << "Get scale factor failed, please check X11 Extension";
    }

    return scaleFactor;
}

// 读取系统配置文件的缩放比，如果是null，默认返回1
double getScaleFormConfig()
{
    double defaultScaleFactor = 1.0;
    DTK_CORE_NAMESPACE::DConfig * dconfig = DConfig::create("org.deepin.dde.lightdm-deepin-greeter", "org.deepin.dde.lightdm-deepin-greeter", QString(), nullptr);
    //华为机型,从override配置中获取默认缩放比
    if (dconfig) {
        defaultScaleFactor = dconfig->value("defaultScaleFactors", 1.0).toDouble() ;
        qDebug() <<"Default scale factor: " << defaultScaleFactor;
        if(defaultScaleFactor < 1.0){
            defaultScaleFactor = 1.0;
        }

        delete dconfig;
        dconfig = nullptr;
    }

    QDBusInterface configInter("com.deepin.system.Display",
                                                     "/com/deepin/system/Display",
                                                     "com.deepin.system.Display",
                                                    QDBusConnection::systemBus());
    if (!configInter.isValid()) {
        return defaultScaleFactor;
    }
    QDBusReply<QString> configReply = configInter.call("GetConfig");
    if (configReply.isValid()) {
        QString config = configReply.value();
        QJsonParseError jsonError;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(config.toLatin1(), &jsonError));
        if (jsonError.error == QJsonParseError::NoError) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonObject Config = rootObj.value("Config").toObject();
            double scaleFactor = Config.value("ScaleFactors").toObject().value("ALL").toDouble();
            qDebug() << "Scale factor from system display config: " << scaleFactor;
            if(scaleFactor == 0.0) {
                scaleFactor = defaultScaleFactor;
            }
            return scaleFactor;
        } else {
            return defaultScaleFactor;
        }
    } else {
        qWarning() << "DBus call `GetConfig` failed, reply is invaild, error: " << configReply.error().message();
        return defaultScaleFactor;
    }
}

static void setQtScaleFactorEnv() {
    const double scaleFactor = IsWayland ? getScaleFormConfig() : getScaleFactor();
    qDebug() << "Final scale factor: " << scaleFactor;
    if (scaleFactor > 0.0) {
        std::cout << QString("QT_SCALE_FACTOR="+QByteArray::number(scaleFactor)).toStdString().c_str() << std::endl;
    } else {
        std::cout << QString("QT_AUTO_SCREEN_SCALE_FACTOR=1").toStdString().c_str() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    // 设置缩放，文件存在的情况下，由后端去设置，否则前端自行设置
    if (!QFile::exists("/etc/lightdm/deepin/xsettingsd.conf") || !isScaleConfigExists() || IsWayland) {
        setQtScaleFactorEnv();
    }

    return 0;
}
