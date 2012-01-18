/*
    Copyright © 2009 Ruslan Nigmatullin <euroelessar@yandex.ru>

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

#ifndef XDGICONTHEME_P_H
#define XDGICONTHEME_P_H

#include "xdgicontheme.h"
#include <QHash>

class QSettings;

/**
  @private
*/
struct XdgIconDir
{
    enum Type
    {
        Fixed = 0,
        Scalable = 1,
        Threshold = 2
	};
	XdgIconDir() : size(0), type(Threshold), maxsize(0), minsize(0), threshold(0) {}
	void fill(QSettings &settings);

    QString path;
    uint size;
    Type type;
    uint maxsize;
    uint minsize;
    uint threshold;
};

/**
  @private
*/
struct XdgIconEntry
{
    inline XdgIconEntry() : dir(0) {}
    XdgIconEntry(const XdgIconDir *d, const QString &p) : dir(d), path(p) {}
    const XdgIconDir *dir;
    QString path;
};

/**
  @private
*/
class XdgIconData
{
public:
    QList<XdgIconEntry> entries;
    QStringRef name;

    const XdgIconEntry *findEntry(uint size) const;
};

/**
  @private
*/
typedef QHash<QStringRef, XdgIconData> XdgIconDataHash;
typedef QMap<QString, XdgIconDir> XdgIconDirHash;

/**
  @private
*/
class XdgIconThemePrivate
{
public:
	XdgIconManager *manager;
    QString id;
    QString name;
    QString example;
    bool hidden;
    QVector<QDir> basedirs;
    QStringList parentNames;
    XdgIconDirHash subdirs;
    QVector<const XdgIconTheme *> parents;
	mutable QString buffer;
	mutable XdgIconDataHash icons;

    XdgIconData *findIcon(const QString &name) const;
    QString findIcon(const QString &name, uint size) const;
    XdgIconData *lookupIconRecursive(const QString &name, QList<const XdgIconThemePrivate*> &themeSet) const;
    XdgIconData *tryCache(const QString &name) const;
    void saveToCache(const QString &originName, XdgIconData *data) const;
    QString lookupFallbackIcon(const QString &name) const;
    static bool dirMatchesSize(const XdgIconDir &dir, uint size);
    static uint dirSizeDistance(const XdgIconDir &dir, uint size);
	void ensureDirectoryMapsHelper() const;
	inline void ensureDirectoryMaps() const { if(icons.isEmpty()) ensureDirectoryMapsHelper(); }
};

#endif // XDGICONTHEME_P_H
