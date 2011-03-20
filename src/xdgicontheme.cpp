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

XdgIconData *XdgIconThemePrivate::findIcon(const QString &name) const
{
	QList<const XdgIconThemePrivate*> themeSet;
	return lookupIconRecursive(name, themeSet);
}

XdgIconData *XdgIconThemePrivate::lookupIconRecursive(const QString &originName,
                                                      QList<const XdgIconThemePrivate*> &themeSet) const
{
    if (themeSet.contains(this))
        return 0;
    themeSet.append(this);
    ensureDirectoryMaps();
	QStringRef iconName(&originName);
	while (!iconName.isEmpty()) {
		XdgIconDataHash::Iterator it = icons.find(iconName);
		if (it != icons.end())
			return &it.value();
		int index = originName.lastIndexOf('-', iconName.size() - 1);
		if (index <= 0)
			iconName = QStringRef();
		else
			iconName = QStringRef(&originName, 0, index);
	}
	foreach (const XdgIconTheme *parent, parents) {
		XdgIconData *data = parent->d_func()->lookupIconRecursive(originName, themeSet);
		if (data)
			return data;
	}
    return 0;
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

void XdgIconThemePrivate::ensureDirectoryMapsHelper() const
{
    foreach (const QDir &basedir, basedirs) {
        QDir dir = basedir;
        if (!dir.cd(id))
            continue;
        QString dirPath = dir.absolutePath();
        QDirIterator it(dirPath, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
			QFileInfo info = it.fileInfo();
            if (info.isFile()) {
				const XdgIconDir *dir = 0;
				QString dirPath = info.path();
				for (int i = 0; i < subdirs.size(); i++) {
					if (dirPath.endsWith(subdirs.at(i).path)) {
						dir = &subdirs.at(i);
						break;
					}
				}
				if (!dir) {
					qWarning("QXdg: \"%s\" is unknown dir", qPrintable(info.absolutePath()));
					continue;
				}
				QString name = info.baseName();
				XdgIconDataHash::Iterator it = icons.find(QStringRef(&name));
				QString path = info.absoluteFilePath();
				if (it == icons.end()) {
					XdgIconData data;
					data.name = name;
					QStringRef iconName(&buffer, buffer.size(), name.size());
					buffer.append(name);
					it = icons.insert(iconName, data);
				}
				it.value().entries << XdgIconEntry(dir, path);
			}
        }
    }
	buffer.squeeze();
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
XdgIconTheme::XdgIconTheme(const QVector<QDir> &basedirs, const QString &id, XdgIconManager *manager, const QString &indexFileName)
        : p(new XdgIconThemePrivate)
{
    Q_D(XdgIconTheme);

	d->manager = manager;
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
    d->hidden = settings.value(QLatin1String("Hidden"), false).toBool();
    d->parentNames = settings.value(QLatin1String("Inherits")).toStringList();
    QStringList subdirList = settings.value(QLatin1String("Directories")).toStringList();
    settings.endGroup();

	QSet<QString> set;
    for (int i = 0; i < subdirList.size(); i++) {
        const QString &subdir = subdirList.at(i);
        // The defaults are dictated by the FDO specification
        d->subdirs.append(XdgIconDir());
        XdgIconDir &dirdata = d->subdirs.last();

        dirdata.path = subdir;
		set << dirdata.path;
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
	foreach (QDir basedir, basedirs) {
		if (!basedir.cd(id))
			continue;
		foreach (const QString &size, basedir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
			XdgIconDir dir;
			if (size == QLatin1String("scalable")) {
				dir.size = 128;
				dir.minsize = 1;
				dir.maxsize = 256;
				dir.type = XdgIconDir::Scalable;
			} else if (size.contains('x')) {
				dir.size = size.section(QLatin1Char('x'), 0, 0).toInt();
				dir.minsize = dir.maxsize = dir.size;
				dir.type = XdgIconDir::Threshold;
			} else {
				continue;
			}
			foreach (const QString &type, QDir(basedir.filePath(size)).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
				QString path = size;
				path += QLatin1String("/");
				path += type;
				if (set.contains(path))
					continue;
				dir.path = path;
				d->subdirs << dir;
				set << path;
			}
		}
	}
}

/**
  Destroys the theme object, invalidating icons created with it.
*/
XdgIconTheme::~XdgIconTheme()
{
    delete p;
}

XdgIconManager *XdgIconTheme::manager() const
{
	return d_func()->manager;
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
