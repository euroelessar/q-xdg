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

#include "xdgiconengine_p.h"
#include <QtGui/QPixmapCache>
#include <QtGui/QPainter>
#include <QtGui/QImageReader>
#include <QtGui/QApplication>
#include <QtGui/QPalette>
#include <QtGui/QStyleOption>
#include <QtGui/QStyle>

XdgIconEngine::XdgIconEngine(XdgIconData *data)
{
    d = data;
    if (d)
        d->ref.ref();
}

XdgIconEngine::~XdgIconEngine()
{
    if (d && !d->ref.deref() && !d->theme) {
        delete d;
        d = 0;
    }
}

void XdgIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    painter->drawPixmap(rect, pixmap(rect.size(), mode, state));
}

QSize XdgIconEngine::actualSize(const QSize &size, QIcon::Mode, QIcon::State)
{
    int min = qMin(size.width(), size.height());
    const XdgIconEntry *entry = d ? d->findEntry(min) : 0;
    if (entry) {
        if (entry->dir->type == XdgIconDir::Scalable)
            return QSize(min, min);
        else
            return QSize(entry->dir->size, entry->dir->size);
    }
    return QSize();
}

QPixmap XdgIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);

    QPixmap pixmap;
    if (!size.isValid() || !d)
        return pixmap;

    int min = qMin(size.width(), size.height());
    const XdgIconEntry *entry = d->findEntry(min);

    if (entry) {
        QString key = QLatin1String("$xdg_icon_");
        // TODO: Think about how to use QIcon::State,
		// Qt's default implementation doesn't hold it
//        key += QString::number(state);
        key += QString::number(min);
        key += QString::number(QApplication::palette().cacheKey());
        key += QLatin1Char('_');
        key += d->name;
        key += QString::number(mode);

        if (QPixmapCache::find(key, pixmap))
            return pixmap;

        bool hasNormalIcon = false;

        if (mode != QIcon::Normal) {
            key.chop(1);
            key += QString::number(QIcon::Normal);

            hasNormalIcon = QPixmapCache::find(key, pixmap);
        }

        if (!hasNormalIcon) {
            QImage image;
            QImageReader reader;
            reader.setFileName(entry->path);
			QSize minSize(min, min);
            reader.setScaledSize(minSize);
            reader.read(&image);
            pixmap = QPixmap::fromImage(image);
			if (pixmap.size() != minSize) {
				pixmap = pixmap.scaled(minSize, Qt::IgnoreAspectRatio,
				                       Qt::SmoothTransformation);
			}

            QPixmapCache::insert(key, pixmap);
        }

        if (mode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pixmap, &opt);

            if (!generated.isNull())
                pixmap = generated;

            key.chop(1);
            key += QString::number(mode);
            QPixmapCache::insert(key, pixmap);
        }
    }
    return pixmap;
}

void XdgIconEngine::addPixmap(const QPixmap &, QIcon::Mode, QIcon::State)
{
}

void XdgIconEngine::addFile(const QString &, const QSize &, QIcon::Mode, QIcon::State)
{
}

QString XdgIconEngine::key() const
{
    return QLatin1String("XdgIconEngine");
}

QIconEngineV2 *XdgIconEngine::clone() const
{
    return new XdgIconEngine(d);
}

// TODO: There may be different IconManager's, which we should use?..

bool XdgIconEngine::read(QDataStream &in)
{
    Q_UNUSED(in);
    return false;
}

bool XdgIconEngine::write(QDataStream &out) const
{
    Q_UNUSED(out);
    return false;
}

#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
# define IconNameHook 2
#endif

void XdgIconEngine::virtual_hook(int id, void *data)
{
	switch (id) {
	case IconNameHook:
		*reinterpret_cast<QString*>(data) = d->name;
		break;
	default:
		QIconEngineV2::virtual_hook(id, data);
		break;
	}
}
