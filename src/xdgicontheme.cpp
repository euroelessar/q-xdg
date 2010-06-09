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

#include <limits>
#include <QtCore/QSettings>
#include <QtCore/QSet>
#include <QtCore/QDirIterator>
#include "xdgicontheme_p.h"
#include "xdgicon.h"

namespace
{
    const char *exts[] = { ".png", ".svg", ".svgz", ".svg.gz", ".xpm" };
    const int extCount = sizeof(exts) / sizeof(char *);
}

const XdgIconEntry *XdgIconData::findEntry(uint size) const
{
    if (entries.isEmpty())
        return 0;

    // Look for an exact size match first, per specification
    for (int i = 0; i < entries.size(); i++) {
        if (XdgIconThemePrivate::dirMatchesSize(*entries[i].dir, size)) {
            return &entries[i];
        }
    }

    // Then find the closest size
    uint mindist = 0;
    const XdgIconEntry *entry = 0;
    for (int i = 0; i < entries.size(); i++) {
        uint distance = XdgIconThemePrivate::dirSizeDistance(*entries[i].dir, size);

        if (!entry || distance < mindist) {
            mindist = distance;
            entry = &entries[i];
        }
    }

    return entry;
}

bool XdgIconData::destroy()
{
    entries.clear();
    name.clear();
    theme = 0;
    return !ref;
}

XdgIconData *XdgIconThemePrivate::findIcon(const QString &name) const
{
    QString key = id;
    key += QLatin1Char('\0');
    key += name;

    XdgIconData *data = 0;

    XdgIconDataHash::const_iterator it = cache.constFind(key);
    if (it != cache.constEnd()) {
        data = it.value();
    } else {
        QSet<const XdgIconThemePrivate*> themeSet;
        data = lookupIconRecursive(name, themeSet);
        cache.insert(key, data);
    }

    return data;
}

XdgIconData *XdgIconThemePrivate::lookupIconRecursive(const QString &originName,
                                                      QSet<const XdgIconThemePrivate*> &themeSet) const
{
    XdgIconData *data = 0;

    if (themeSet.contains(this))
        return data;
    themeSet.insert(this);

    QString baseName = originName.section(QLatin1Char('-'), 0, 0);
    QString nameWithoutBase = originName.mid(baseName.count(), -1) + ".";
    for (int i = 0; i < directoryMaps.size(); i++) {
        const QStringList &map = directoryMaps.at(i);
        for (int j = 0; j < subdirs.size(); j++) {
            const XdgIconDir &dirdata = subdirs.at(j);
            QString baseIconPath = dirdata.path + QLatin1Char('/') + baseName;
            QStringList::const_iterator it;
            QStringList::const_iterator end = map.constEnd();
            it = qLowerBound(map, baseIconPath);
            if (it == end)
                continue;
            for (; it != end; ++it) {
                if (!it->startsWith(baseIconPath))
                    break;
                QString name = it->section('/', -1, -1);
                int index = name.lastIndexOf(QLatin1Char('.'));
                if (index < 0)
                    continue;
                name.truncate(index);
                if (name == originName
                    || (name.size() < originName.size() 
                        && originName.startsWith(name)
                        && originName.at(name.size()) == '-')) {
                    if (!data) {
                        data = new XdgIconData;
                        data->theme = this;
                        data->name = name;
                    }
                    if (data->name.size() > name.size()) {
                        continue;
                    } else if (data->name.size() < name.size()) {
                        data->name = name;
                        data->entries.clear();
                    }
                    QString filePath = basedirs.at(i).absolutePath();
                    filePath += QLatin1Char('/');
                    filePath += id;
                    filePath += QLatin1Char('/');
                    filePath += *it;
                    data->entries.append(XdgIconEntry(&dirdata, QDir::cleanPath(filePath)));
                    break;
                }
                if (it->mid(baseIconPath.length(), -1).startsWith(nameWithoutBase))
                    break;
            }
        }
    }

    if (!data) {
        foreach (const XdgIconTheme *parent, parents) {
            data = parent->d_func()->lookupIconRecursive(originName, themeSet);
            if (data)
                break;
        }
    }

    if (!data) {
        saveToCache(originName, 0);
    } else {
        QString name = originName;
        while (name.size() >= data->name.size()) {
            saveToCache(name, data);
            name.truncate(name.lastIndexOf('-'));
        }
    }

    return data;
}

XdgIconData *XdgIconThemePrivate::tryCache(const QString &name) const
{
    QString key;
    key.reserve(id.size() + name.size() + 1);
    key += id;
    key += QLatin1Char('\0');
    key += name;

    return cache.value(key, 0);
}

void XdgIconThemePrivate::saveToCache(const QString &originName, XdgIconData *data) const
{
    QString name = originName;
    QString key = id;
    key += QLatin1Char('\0');
    while (!name.isEmpty() && (name.size() >= (data ? data->name.size() : 0))) {
        key += name;
        cache.insert(key, data);

        int pos = name.lastIndexOf(QLatin1Char('-'));
        if (pos == -1)
            return;

        key.truncate(id.size() + 1);
        name.truncate(pos);
    }
}

QString XdgIconThemePrivate::lookupFallbackIcon(const QString &name) const
{
    for (int i = 0; i < basedirs.size(); i++) {
        QDir dir(basedirs.at(i));

        for (int j = 0; j < extCount; j++) {
            QString fullname = dir.absoluteFilePath(name + exts[j]);

            if (QFile::exists(fullname))
                return fullname;
        }
    }

    return QString();
}

bool XdgIconThemePrivate::dirMatchesSize(const XdgIconDir &dir, uint size)
{
    switch (dir.type) {
    case XdgIconDir::Fixed:
        return size == dir.size;
    case XdgIconDir::Scalable:
        return (size >= dir.minsize) && (size <= dir.maxsize);
    case XdgIconDir::Threshold:
        return (size >= dir.size - dir.threshold) && (size <= dir.size + dir.threshold);
    }
    Q_ASSERT(!"New directory type?..");
    return false;
}

uint XdgIconThemePrivate::dirSizeDistance(const XdgIconDir &dir, uint size)
{
    switch (dir.type) {
    case XdgIconDir::Fixed:
        return qAbs(int(dir.size) - int(size));
    case XdgIconDir::Scalable:
        if(size < dir.minsize)
            return dir.minsize - size;
        if(size > dir.maxsize)
            return size - dir.maxsize;
        return 0;
    case XdgIconDir::Threshold:
        if(size < dir.size - dir.threshold)
            return dir.size - dir.threshold - size;
        if(size > dir.size + dir.threshold)
            return size - dir.size - dir.threshold;
        return 0;
    }

    Q_ASSERT(!"New directory type?..");
    return 0;
}

/**
  Creates a new icon theme with the specified list of base directories, ID, and
  <code>index.theme</code> file. If the third parameter is empty or missing,
  creates an empty theme with no subdirectories, looking up only fallback icons.

  This constructor does not recursively create parent themes. To add parent
  themes, use <code>addParent()</code> after creating the object.

  You will most likely not need to create objects of this class directly.
  Instead, use <code>XdgIconManager</code> to get references to themes.
*/
XdgIconTheme::XdgIconTheme(const QVector<QDir> &basedirs, const QString &id, const QString &indexFileName)
        : p(new XdgIconThemePrivate)
{
    Q_D(XdgIconTheme);

    d->id = id;
    d->basedirs = basedirs;
    d->hidden = false;
    d->example = QString();

    if (indexFileName.isEmpty()) {
        // create an empty theme with defaults
        d->name = id;
        return;
    }

    QSettings settings(indexFileName, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("Icon Theme"));
    d->name = settings.value(QLatin1String("Name")).toString();
    d->example = settings.value(QLatin1String("Example")).toString();
    d->hidden = settings.value(QLatin1String("Hidden")).toBool();
    d->parentNames = settings.value(QLatin1String("Inherits")).toStringList();
    QStringList subdirList = settings.value(QLatin1String("Directories")).toStringList();
    settings.endGroup();

    for (int i = 0; i < subdirList.size(); i++) {
        const QString &subdir = subdirList.at(i);
        // The defaults are dictated by the FDO specification
        d->subdirs.append(XdgIconDir());
        XdgIconDir &dirdata = d->subdirs.last();

        dirdata.path = subdir;
        settings.beginGroup(dirdata.path);
        dirdata.size = settings.value(QLatin1String("Size")).toUInt();
        dirdata.maxsize = settings.value(QLatin1String("MaxSize"), dirdata.size).toUInt();
        dirdata.minsize = settings.value(QLatin1String("MinSize"), dirdata.size).toUInt();
        dirdata.threshold = settings.value(QLatin1String("Threshold"), 2).toUInt();
        QString type = settings.value(QLatin1String("Type"), QLatin1String("Threshold")).toString();
        settings.endGroup();

        if (type == QLatin1String("Fixed"))
            dirdata.type = XdgIconDir::Fixed;
        else if (type == QLatin1String("Scalable"))
            dirdata.type = XdgIconDir::Scalable;
        else
            dirdata.type = XdgIconDir::Threshold;
    }

    foreach (const QDir &basedir, basedirs) {
        QStringList map;
        QDir dir = basedir;
        if (!dir.cd(id)) {
            d->directoryMaps << map;
            continue;
        }
        QString dirPath = dir.absolutePath();
        QDirIterator it(dirPath, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString name = it.next();
            if (it.fileInfo().isFile())
                map << name.mid(dirPath.size() + 1);
        }
        map.sort();
        d->directoryMaps << map;
    }
}

/**
  Destroys the theme object, invalidating icons created with it.
*/
XdgIconTheme::~XdgIconTheme()
{
    delete p;
}

/**
  Returns the theme ID (e.g. "gnome-noble").
*/
QString XdgIconTheme::id() const
{
    return d_func()->id;
}

/**
  Returns the human-readable theme name (e.g. "GNOME Noble").
*/
QString XdgIconTheme::name() const
{
    return d_func()->name;
}

/**
  Returns the XDG name (e.g. "document-new") of the icon that is supposed to
  be an example of how the theme looks, or an empty string if the theme author
  had none set. This setting is up to the application to honor.
*/
QString XdgIconTheme::exampleName() const
{
    return d_func()->example;
}

/**
  Indicates whether the theme has the hidden flag, i.e. should not be visible
  to the user in theme selection lists. This flag is up to the application to
  honor. (Default: false)
*/
bool XdgIconTheme::hidden() const
{
    return d_func()->hidden;
}

/**
  Returns the list of parent theme IDs found in <code>index.theme</code>,
  whether or not these parent themes were actually created.
*/
QStringList XdgIconTheme::parentIds() const
{
    return d_func()->parentNames;
}

/**
  Adds a parent theme to this theme.
*/
void XdgIconTheme::addParent(const XdgIconTheme *parent)
{
    Q_D(XdgIconTheme);
    Q_ASSERT_X(parent, "XdgIconTheme::addParent", "Parent must be not null");
    if (!d->parents.contains(parent))
        d->parents.append(parent);
}

/**
  Looks up an icon file with the specified name (e.g. "document-new") and size,
  and returns its full file path. The lookup algorithm involves scanning parent
  themes and fallback icons if no match is found in the current theme, and is
  described in detail in the XDG Icon Theme Specification on freedesktop.org.
*/
QString XdgIconTheme::getIconPath(const QString &name, uint size) const
{
    Q_D(const XdgIconTheme);

    XdgIconData *data = d->findIcon(name);
    const XdgIconEntry *entry = data ? data->findEntry(size) : 0;
    return entry ? entry->path : QString();
}
