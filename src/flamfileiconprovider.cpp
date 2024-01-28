/***************************************************************************
 *   Copyright (C) 2007-2024 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#include <QDir>
#include <QDateTime>

#include "flamfileiconprovider.h"
#include "qosmic.h"
#include "logger.h"

FlamFileIconProvider::FlamFileIconProvider()
: QFileIconProvider(), icons_dir(QOSMIC_USERDIR), has_icons(false)
{
	QString path("icons");
	if (!icons_dir.exists(path))
		icons_dir.mkpath(path);
	has_icons = icons_dir.cd(path);
	logFine(QString("FlamFileIconProvider::const : icons dir %1").arg(icons_dir.canonicalPath()));
}


FlamFileIconProvider::~FlamFileIconProvider()
{
}


QIcon FlamFileIconProvider::icon(const QFileInfo& info) const
{
	QString file_name(info.fileName());
	QRegExp rex("flam(3|e)$");
	if (has_icons && file_name.contains(rex))
	{
		QString img_file(file_name);
		img_file.replace(rex, "png");
		QFileInfo img(info.dir(), img_file);

		if (!img.exists())
			return QFileIconProvider::icon(info);

		QDir cache_dir(icons_dir.canonicalPath() + info.dir().canonicalPath());
		if (!cache_dir.exists() && !cache_dir.mkpath("."))
			return QFileIconProvider::icon(info);

		QFileInfo cache_file(cache_dir, img_file);

		if (cache_file.exists() && (cache_file.lastModified() > img.lastModified()))
		{
			logFiner(QString("FlamFileIconProvider::icon : found cached %1").arg(cache_file.absoluteFilePath()));
			return QIcon(cache_file.absoluteFilePath());
		}
		else
		{
			QString cache(cache_file.absoluteFilePath());
			QImage buf(img.absoluteFilePath());
			if (!buf.isNull() && buf.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(cache))
			{
				logInfo(QString("FlamFileIconProvider::icon : creating icon  %1").arg(cache));
				return QIcon(cache);
			}
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

