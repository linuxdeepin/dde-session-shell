/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dde-session-shell/xml/org.freedesktop.DBus.xml -a ./dde-session-shell/toolGenerate/qdbusxml2cpp/org.freedesktop.DBusAdaptor -i ./dde-session-shell/toolGenerate/qdbusxml2cpp/org.freedesktop.DBus.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef ORG_FREEDESKTOP_DBUSADAPTOR_H
#define ORG_FREEDESKTOP_DBUSADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "./dde-session-shell/toolGenerate/qdbusxml2cpp/org.freedesktop.DBus.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.freedesktop.DBus
 */
class DBusAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.freedesktop.DBus\">\n"
"    <signal name=\"NameOwnerChanged\">\n"
"      <arg type=\"s\" name=\"name\"/>\n"
"      <arg type=\"s\" name=\"oldOwner\"/>\n"
"      <arg type=\"s\" name=\"newOwner\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    DBusAdaptor(QObject *parent);
    virtual ~DBusAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    void NameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
};

#endif