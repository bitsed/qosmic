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
#ifndef FLAMFILEICONPROVIDER_H
#define FLAMFILEICONPROVIDER_H

#include <QFileIconProvider>

class FlamFileIconProvider : public QFileIconProvider
{
	QDir icons_dir;
	bool has_icons;

	public:
		FlamFileIconProvider();
		~FlamFileIconProvider();

		QIcon icon(const QFileInfo& info) const;
		QIcon icon(IconType type) const;
		QString type(const QFileInfo& info) const;
};

#endif
