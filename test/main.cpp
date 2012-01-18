/*
    Copyright © 2009 Maia Kozheva <sikon@ubuntu.com>

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

#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include "../src/xdg.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    XdgIconManager manager;

    qDebug() << manager.themeNames() << manager.themeIds() << xdgGetGnomeTheme() << xdgGetKdeTheme();

    const XdgIconTheme *current = manager.defaultTheme();
#ifdef Q_OS_WIN
	if (!current)
		current = manager.themeById("oxygen");
#endif

    if (current) {
        qDebug() << current->getIconPath("document-new", 22);
        qDebug() << current->getIconPath("document-open-subtype-test", 128);
        QIcon icon = current->getIcon("document-save-test");
        QPixmap pixmap = icon.pixmap(64, QIcon::Normal);
        QLabel label;
        label.setPixmap(pixmap);
        label.show();
        return QApplication::exec();
    } else {
        qDebug() << "Current theme not found!";
    }
}
