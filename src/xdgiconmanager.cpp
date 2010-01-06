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

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QSettings>
#include <QtCore/QVector>
#include "xdgenvironment.h"
#include "xdgiconmanager_p.h"

/**
  Creates a new icon manager that searches icons in base directories returned
  by <code>XdgEnvironment::dataDirs()</code>.

  @arg appDirs: Optional. Specifies custom directories to search for icons
    and icon themes in, in addition to system default directories.
*/
XdgIconManager::XdgIconManager(const QList<QDir> &appDirs) : d(new XdgIconManagerPrivate)
{
    d->rules.insert(QRegExp(QLatin1String("gnome"), Qt::CaseInsensitive), &xdgGetGnomeTheme);
    d->rules.insert(QRegExp(QLatin1String("kde"), Qt::CaseInsensitive), &xdgGetKdeTheme);
    d->rules.insert(QRegExp(QLatin1String("xfce"), Qt::CaseInsensitive), &xdgGetXfceTheme);
    d->init(appDirs);
}

/**
  Destroys the icon manager object.
*/
XdgIconManager::~XdgIconManager()
{
}

/**
  Copy constructor. Creates an icon manager object based on this one.
*/
XdgIconManager::XdgIconManager(const XdgIconManager &other) : d(other.d)
{
}

/**
  Assignment operator. Makes this icon manager object a copy of
  <code>other</code>.
*/
XdgIconManager &XdgIconManager::operator =(const XdgIconManager &other)
{
    d = other.d;
    return *this;
}

void XdgIconManagerPrivate::init(const QList<QDir> &appDirs)
{
    // Identify base directories
    QLatin1String hicolorString("hicolor");
    QVector<QDir> basedirs;
    QDir basedir(QDir::home());

    if(basedir.cd(QLatin1String(".icons")) && !basedirs.contains(basedir))
        basedirs.append(basedir);

    QList<QDir> datadirs = XdgEnvironment::dataDirs();
    datadirs << appDirs;

    QString iconsStr(QLatin1String("icons"));

    foreach (QDir dir, datadirs) {
        dir.makeAbsolute();
        if (dir.cd(iconsStr) && !basedirs.contains(dir))
            basedirs.append(dir);
    }

    basedir = QLatin1String("/usr/share/pixmaps");

    if (basedir.exists() && !basedirs.contains(basedir))
        basedirs.append(basedir);

    // Build theme list
    foreach (QDir dir, basedirs) {
        QDirIterator subdirs(dir);

        while (subdirs.hasNext()) {
            QFileInfo subdir(subdirs.next());

            if (!subdir.isDir())
                continue;

            QString index = QDir(subdir.canonicalFilePath()).absoluteFilePath(QLatin1String("index.theme"));

            if (QFileInfo(index).exists()) {
                QSettings settings(index, QSettings::IniFormat);
                QString name = settings.value(QLatin1String("Icon Theme/Name")).toString();

                if (!name.isEmpty()) {
                    QMap<QString, XdgIconTheme *>::const_iterator it = themes.constFind(name);
                    XdgIconTheme *theme;
                    if (it == themes.constEnd()) {
                        theme = new XdgIconTheme(basedirs, subdir.fileName(), index);
                        themes.insert(name, theme);
                    } else
                        theme = it.value();

                    themeIdMap.insert(subdir.fileName(), theme);
                }
            }
        }
    }

    const XdgIconTheme *hicolor = themeIdMap.value(hicolorString);

    if (!hicolor) {
        // create empty theme - hicolor will be guaranteed to always exist
        XdgIconTheme *newTheme = new XdgIconTheme(basedirs, hicolorString);
        themes.insert(hicolorString, newTheme);
        themeIdMap.insert(hicolorString, newTheme);
        hicolor = newTheme;
    }

    // Resolve dependencies
    for(QMap<QString, XdgIconTheme*>::iterator it = themes.begin(); it != themes.end(); ++it) {
        XdgIconTheme &theme = *it.value();

        if (theme.id() == hicolorString)
            continue;

        if (theme.parentIds().isEmpty()) {
            theme.addParent(hicolor);
            continue;
        }

        foreach (QString parent, theme.parentIds()) {
            const XdgIconTheme *parentTheme = themeIdMap.value(parent);
            if(parentTheme)
                theme.addParent(parentTheme);
        }
    }
}

/**
  Clears all rules that the icon manager uses to search for the default theme,
  including default rules for X desktop environments.
*/
void XdgIconManager::clearRules()
{
    d->rules.clear();
}

/**
  Installs a rule (regular expression) for determining the current theme based
  on the <code>DESKTOP_SESSION</code> environment variable. See
  <code>defaultTheme()</code>.
*/
void XdgIconManager::installRule(const QRegExp &regexp, XdgThemeChooser chooser)
{
    d->rules.insert(regexp, chooser);
}

/**
  Returns the system default theme. The theme depends on the current platform
  and desktop environment, and is determined in the following order:

  @arg First, the <code>DESKTOP_SESSION</code> environment variable is matched
    against installed rules (regular expressions). The first rule to match
    determines the chooser function that will be used to get the theme ID.
    By default, rules for GNOME, KDE and Xfce are installed.
  @arg If this fails, specific environment variables for KDE and GNOME are
    checked to be present.
  @arg If nothing else works, the fallback "hicolor" theme is returned.

  This function is guaranteed to always return a non-null theme object.
*/
const XdgIconTheme *XdgIconManager::defaultTheme() const
{
    XdgThemeChooser chooser = 0;
    QByteArray env = qgetenv("DESKTOP_SESSION");
    QString session = QString::fromLocal8Bit(env, env.size());

    QHash<QRegExp, XdgThemeChooser>::const_iterator it;

    for (it = d->rules.begin(); it != d->rules.end(); ++it) {
        // FIXME: Is it really needed to use regular expressions here?
        if(it.key().indexIn(session) != -1) {
            chooser = it.value();
            break;
        }
    }

    if(!chooser) {
        if (qgetenv("KDE_FULL_SESSION") == "true")
            chooser = &xdgGetKdeTheme;
        else if (!qgetenv("GNOME_DESKTOP_SESSION_ID").isEmpty())
            chooser = &xdgGetGnomeTheme;
    }

    return themeById(chooser ? (*chooser)() : QLatin1String("hicolor"));
}

/**
  Returns a theme by its human-readable name (like "GNOME Noble"), or 0 if no
  theme with this name was found.
*/
const XdgIconTheme *XdgIconManager::themeByName(const QString &themeName) const
{
    return d->themes.value(themeName, 0);
}

/**
  Returns a theme by its ID, or directory name (like "gnome-noble"), or 0 if no
  theme with this ID was found. The ID "hicolor" is guaranteed to return a
  non-null fallback theme.
*/
const XdgIconTheme *XdgIconManager::themeById(const QString &themeName) const
{
    return d->themeIdMap.value(themeName, 0);
}

/**
  Returns a list of all human-readable theme names known to the system.

  @arg showHidden: Whether to return themes flagged as hidden. (Default: false)
*/
QStringList XdgIconManager::themeNames(bool showHidden) const
{
    if (showHidden) {
        return QStringList(d->themes.keys());
    }

    QStringList out;

    foreach (QString themeName, d->themes.keys()) {
        const XdgIconTheme *theme = themeByName(themeName);

        if (theme && !theme->hidden()) {
            out.append(themeName);
        }
    }

    return out;
}

/**
  Returns a list of all theme IDs known to the system.

  @arg showHidden: Whether to return themes flagged as hidden. (Default: false)
*/
QStringList XdgIconManager::themeIds(bool showHidden) const
{
    if (showHidden) {
        return QStringList(d->themeIdMap.keys());
    }

    QStringList out;

    foreach (QString themeId, d->themes.keys()) {
        const XdgIconTheme *theme = themeById(themeId);

        if (theme && !theme->hidden()) {
            out.append(themeId);
        }
    }

    return out;
}
