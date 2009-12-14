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

/**
  Returns the directory for per-user application-specific data.

  \arg Windows: Returns <code>%APPDATA%</code> (usually
    <code>C:\\Documents and Settings\\Application Data</code>).
  \arg Mac: Returns <code>$XDG_DATA_HOME</code> if the variable exists,
    otherwise <code>$HOME/Library/Preferences</code>.
  \arg Unix: Returns <code>$XDG_DATA_HOME</code> if the variable exists,
    otherwise <code>$HOME/.local/share</code>.
*/
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

/**
  Returns the directory for per-user configuration files. This is the directory
  used by <code>QSettings</code> to store its ini-format configuration files.

  \arg Windows: Returns <code>%APPDATA%</code> (usually
    <code>C:\\Documents and Settings\\Application Data</code>).
  \arg Mac: Returns <code>$XDG_CONFIG_HOME</code> if the variable exists,
    otherwise <code>$HOME/Library/Preferences</code>.
  \arg Unix: Returns <code>$XDG_CONFIG_HOME</code> if the variable exists,
    otherwise <code>$HOME/.config</code>.
*/
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

/**
  Returns the list of directories for system application-specific data.

  \arg Windows and Mac: Returns the directory where the application executable
    resides.
  \arg Unix: Returns <code>$XDG_DATA_DIRS</code> if the variable exists,
    otherwise a list consisting of <code>/usr/local/share</code> and
    <code>/usr/share</code>.
*/
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

/**
  Returns the list of directories for system application-specific configuration.

  \arg Windows: Returns the directory where the application executable
    resides.
  \arg Mac: Returns the <code>/Library/Preferences</code> directory.
  \arg Unix: Returns <code>$XDG_CONFIG_DIRS</code> if the variable exists,
    otherwise <code>/etc/xdg</code>.
*/
QList<QDir> XdgEnvironment::configDirs()
{
#ifdef Q_WS_WIN
    QList<QDir> list;
    list.append(QDir(getValue("COMMON_APPDATA", QCoreApplication::applicationDirPath())));
    return list;
#elif defined(Q_WS_MAC)
    QList<QDir> list;
    list.append(QDir(QLatin1String("/Library/Preferences")));
    return list;
#else
    return splitDirList(getValue("XDG_CONFIG_DIRS",
                                 QDir::home().absoluteFilePath(QLatin1String("/etc/xdg"))));
#endif
}
