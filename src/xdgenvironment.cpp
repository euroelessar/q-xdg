/*
    Copyright (C) 2009 Maia Kozheva <sikon@ubuntu.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "xdgenvironment.h"
#include <QtCore/QCoreApplication>

namespace
{
    inline QList<QDir> splitDirList(const QString &str)
    {
        QList<QDir> list;

        foreach(QString str, str.split(QLatin1Char(':')))
            list.append(QDir(str));

        return list;
    }

    inline QString getValue(const char *varName, const QString &defValue)
    {
        QByteArray env = qgetenv(varName);
        return env.isEmpty() ? defValue : QString::fromLocal8Bit(env.constData(), env.size());
    }
}

XdgEnvironment::XdgEnvironment()
{
}

XdgEnvironment::~XdgEnvironment()
{
}

QDir XdgEnvironment::dataHome()
{
#ifdef Q_WS_WIN
    return QDir(getValue("APPDATA", QDir::home()));
#elif defined(Q_WS_MAC)
    return QDir(getValue("XDG_DATA_HOME",
                         QDir::home().absoluteFilePath(QLatin1String("Library/Preferences"))));
#else
    return QDir(getValue("XDG_DATA_HOME",
                         QDir::home().absoluteFilePath(QLatin1String(".local/share"))));
#endif
}

QDir XdgEnvironment::configHome()
{
#ifdef Q_WS_WIN
    return QDir(getValue("APPDATA", QDir::home()));
#elif defined(Q_WS_MAC)
    return QDir(getValue("XDG_CONFIG_HOME",
                         QDir::home().absoluteFilePath(QLatin1String("Library/Preferences"))));
#else
    return QDir(getValue("XDG_CONFIG_HOME",
                         QDir::home().absoluteFilePath(QLatin1String(".config"))));
#endif
}

QList<QDir> XdgEnvironment::dataDirs()
{
#if defined(Q_WS_WIN) || defined (Q_WS_MAC)
    QList<QDir> list;
    list.append(QDir(QCoreApplication::applicationDirPath()));
    return list;
#else
    return splitDirList(getValue("XDG_DATA_DIRS",
                                 QLatin1String("/usr/local/share:/usr/share")));
#endif
}

QList<QDir> XdgEnvironment::configDirs()
{
#ifdef Q_WS_WIN
    QList<QDir> list;
    list.append(QDir(getValue("COMMON_APPDATA", QCoreApplication::applicationDirPath())));
    return list;
#elif defined(Q_WS_MAC)
    QList<QDir> list;
    list.append(QDir("/Library/Preferences"));
    return list;
#else
    return splitDirList(getValue("XDG_CONFIG_DIRS",
                                 QDir::home().absoluteFilePath(QLatin1String("/etc/xdg"))));
#endif
}
