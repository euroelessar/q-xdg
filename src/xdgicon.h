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

#ifndef XDGICON_H
#define XDGICON_H

#include <QtGui/QIcon>
#include "xdgexport.h"

class XdgIconTheme;

/**
  @brief <code>QIcon</code> implementation backed by a theme

  An implementation of <code>QIcon</code> used by Q-XDG to retrieve icons
  based on a certain theme. Creating an instance of this class is equivalent
  to calling <code>XdgIconTheme::getIcon()</code>.
*/
class XDG_API XdgIcon : public QIcon
{
public:
    XdgIcon(const QString &name, const XdgIconTheme *theme);
    XdgIcon(const QIcon &other);
    XdgIcon();
    ~XdgIcon();

    XdgIcon &operator =(const XdgIcon &other);
};

#endif // XDGICON_H
