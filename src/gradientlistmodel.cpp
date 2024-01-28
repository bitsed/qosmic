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

#include <QIcon>

#include "gradientlistmodel.h"

GradientListModel::GradientListModel(QObject *parent)
	: QAbstractListModel(parent)
{
}

QVariant GradientListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DecorationRole)
		return QIcon(pixmaps.value(index.row()));
	else if (role == Qt::UserRole)
		return pixmaps.value(index.row());

	return QVariant();
}

void GradientListModel::addGradient(const QPixmap &pixmap)
{
	int row = pixmaps.size();
	beginInsertRows(QModelIndex(), row, row);
	pixmaps.insert(row, pixmap);
	endInsertRows();
}

Qt::ItemFlags GradientListModel::flags(const QModelIndex &index) const
{
	if (index.isValid()) {
		return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	}

	return Qt::ItemIsDropEnabled;
}

bool GradientListModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (parent.isValid())
		return false;

	if (row >= pixmaps.size() || row + count <= 0)
		return false;

	int beginRow = qMax(0, row);
	int endRow = qMin(row + count - 1, pixmaps.size() - 1);

	beginRemoveRows(parent, beginRow, endRow);

	while (beginRow <= endRow) {
		pixmaps.removeAt(beginRow);
		++beginRow;
	}

	endRemoveRows();
	return true;
}

int GradientListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return pixmaps.size();
}


void GradientListModel::clear()
{
	beginRemoveRows(QModelIndex(), 0, pixmaps.size());
	pixmaps.clear();
	endRemoveRows();
}
