/***************************************************************************
 *   Copyright (C) 2007, 2010 by David Bitseff                             *
 *   dbitsef@zipcon.net                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "flamfileiconprovider.h"

#include <QIcon>
#include <QDir>
#include <QDateTime>

#include "logger.h"

FlamFileIconProvider::FlamFileIconProvider(): QFileIconProvider()
{
}


FlamFileIconProvider::~FlamFileIconProvider()
{
}


QIcon FlamFileIconProvider::icon(const QFileInfo& info) const
{
	QString file_name(info.fileName());
	QRegExp rex("flam(3|e)$");
	if (file_name.contains(rex))
	{
		QString img_file(file_name);
		img_file.replace(rex, "png");
		QFileInfo img(info.dir(), img_file);

		if (!img.exists())
			return QFileIconProvider::icon(info);

		QDir cache_dir(info.dir());
		if (!cache_dir.exists(".icons"))
			cache_dir.mkdir(".icons");
		cache_dir.cd(".icons");
		QFileInfo cache_file(cache_dir, img_file);

		if (cache_file.exists()
				  && (cache_file.lastModified() > img.lastModified()))
		{
			logFiner(QString("FlamFileIconProvider::icon : found %1")
					.arg(cache_file.absoluteFilePath()));
			return QIcon(cache_file.absoluteFilePath());
		}
		else
		{
			logInfo(QString("FlamFileIconProvider::icon : creating %1")
					.arg(cache_file.absoluteFilePath()));
			QImage(img.absoluteFilePath())
					.scaled(128,128,Qt::KeepAspectRatio, Qt::SmoothTransformation)
					.save(cache_file.absoluteFilePath());
			return QIcon(cache_file.absoluteFilePath());
		}
	}
    return QFileIconProvider::icon(info);
}

QIcon FlamFileIconProvider::icon(IconType type) const
{
    return QFileIconProvider::icon(type);
}

QString FlamFileIconProvider::type(const QFileInfo& info) const
{
    return QFileIconProvider::type(info);
}

