/*
    Copyright (C) 2009 Nigmatullin Ruslan <euroelessar@ya.ru>

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
    QAtomicInt ref;
    QList<XdgIconEntry> entries;
    const XdgIconThemePrivate *theme;
    QString name;

    const XdgIconEntry *findEntry(uint size) const;
    bool destroy();
};

/**
  @private
*/
typedef QHash<QString, XdgIconData *> XdgIconDataHash;

/**
  @private
*/
class XdgIconThemePrivate
{
public:
    QString id;
    QString name;
    QVector<QDir> basedirs;
    QStringList parentNames;
    QVector<XdgIconDir> subdirs;
    QVector<const XdgIconTheme *> parents;
    mutable XdgIconDataHash cache;

    XdgIconData *findIcon(const QString &name) const;
    QString findIcon(const QString &name, uint size) const;
    XdgIconData *lookupIconRecursive(const QString &name) const;
    QString lookupFallbackIcon(const QString &name) const;
    static bool dirMatchesSize(const XdgIconDir &dir, uint size);
    static uint dirSizeDistance(const XdgIconDir &dir, uint size);
};

#endif // XDGICONTHEME_P_H
