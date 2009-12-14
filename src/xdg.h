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

#ifndef XDG_H
#define XDG_H

#include "xdgenvironment.h"
#include "xdgicontheme.h"
#include "xdgiconmanager.h"
#include "xdgthemechooser.h"

/**
  \mainpage Q-XDG Library Overview

  \section intro Introduction

  Q-XDG implements support for the freedesktop.org (XDG) Base Directory and
  Icon Theme specifications. You can use it in your Qt4-based software if you
  need support for icon themes compatible with X desktop environments, such as
  GNOME or KDE.

  \section usage_env Usage: Environment

  The <code>XdgEnvironment</code> class can be used independently of others.
  Its static methods return configuration and data paths dictated by the XDG
  Base Directory Specification. See documentation for that class for more
  details.

  \section usage_icon Usage: Icon Themes

  The <code>XdgIconManager</code> class manages the list of available icon
  themes installed in the system. You can use it to retrieve
  <code>XdgIconTheme</code> objects, which represent concrete themes and can
  in turn be used to get icons of different sizes.
*/

#endif // XDG_H
