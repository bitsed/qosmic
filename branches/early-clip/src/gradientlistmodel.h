/***************************************************************************
 *   Copyright (C) 2007 by David Bitseff                                   *
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

#ifndef GRADIENTLISTMODEL_H
#define GRADIENTLISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QPixmap>

class GradientListModel : public QAbstractListModel
{
		Q_OBJECT

	public:
		GradientListModel ( QObject *parent = 0 );

		QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
		Qt::ItemFlags flags ( const QModelIndex &index ) const;
		bool removeRows ( int row, int count, const QModelIndex &parent=QModelIndex() );
		int rowCount ( const QModelIndex &parent ) const;
		void addGradient ( const QPixmap &pixmap );
		void clear();

	private:
		QList<QPixmap> pixmaps;
};



#endif
