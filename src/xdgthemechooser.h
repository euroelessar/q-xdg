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

#ifndef XDGTHEMECHOOSER_H
#define XDGTHEMECHOOSER_H

#include "xdgexport.h"

/**
  @file xdgthemechooser.h

  Global functions used to retrieve default themes for various desktop
  environments, as well as their associated function type.
*/

/**
  Function type for theme chooser functions. They are used in
  <code>XdgIconManager</code> rules to match the <code>DESKTOP_SESSION</code>
  environment variable to the chooser logic to be used for the running desktop
  environment.
*/
typedef QString (*XdgThemeChooser)();

XDG_API QString xdgGetKdeTheme();
XDG_API QString xdgGetGnomeTheme();
XDG_API QString xdgGetXfceTheme();

#endif // XDGTHEMECHOOSER_H
