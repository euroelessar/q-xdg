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

#ifndef XDGICONTHEME_H
#define XDGICONTHEME_H

#include <QtCore/QDir>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVector>

#ifdef QT_GUI_LIB
#include "xdgicon.h"
#endif

class XdgIconThemePrivate;

/**
  @brief Icon theme, responsible for retrieving icons by name and size

  Represents an icon theme installed in the system. It can look up icons by name
  and size, returning paths to image files, pixmap or icon objects.
*/
class XdgIconTheme
{
    Q_DECLARE_PRIVATE_D(p, XdgIconTheme)
    Q_DISABLE_COPY(XdgIconTheme)
public:
    XdgIconTheme(const QVector<QDir> &basedirs, const QString &id, const QString &indexFileName = QString());
    virtual ~XdgIconTheme();

    QString id() const;
    QString name() const;
    QString exampleName() const;
    bool hidden() const;
    QStringList parentIds() const;

    void addParent(const XdgIconTheme *parent);
    QString getIconPath(const QString &name, uint size = 22) const;

#ifdef QT_GUI_LIB
    /**
      Returns an icon with the specified name (e.g. "document-new").
    */
    inline QIcon getIcon(const QString &name) const
    { return XdgIcon(name, this); }

    /**
      Convenience function.

      Returns a pixmap with the specified name (e.g. "document-new") and size
      (in pixels).
    */
    inline QPixmap getPixmap(const QString &name, int size) const
    { return getIcon(name).pixmap(size); }

    /**
      Convenience function.

      Returns a pixmap with the specified name (e.g. "document-new") and size
      (in pixels). The pixmap can be smaller than the requested size if it is
      not square, but never larger.
    */
    inline QPixmap getPixmap(const QString &name, QSize size) const
    { return getIcon(name).pixmap(size); }
#endif
private:
#ifdef QT_GUI_LIB
    friend class XdgIcon;
#endif
    XdgIconThemePrivate *p;
    typedef XdgIconThemePrivate * Data;
    const Data &data() const { return p; }
};

#endif // XDGICONTHEME_H
