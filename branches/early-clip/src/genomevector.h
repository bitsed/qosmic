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
#ifndef GENOMEVECTOR_H
#define GENOMEVECTOR_H

#include <QVector>
#include <QAbstractListModel>
#include "flam3util.h"
#include "undoring.h"

class GenomeVector : public QVector<flam3_genome>
{
	protected:
		int selected_index;
		QList<UndoRing> undoRings;

	public:
		void setSelectedIndex(int value);
		int selectedIndex() const;
		flam3_genome* selectedGenome();
		UndoRing* undoRing(int idx=-1);

		void append(const flam3_genome&);
		void insert(int i, const flam3_genome& g);
		void remove(int);
		void clear();
};


class GenomeVectorListModel : public QAbstractListModel
{
	Q_OBJECT

	GenomeVector* genomes;
	QList<QVariant> previews;

	public:
		GenomeVectorListModel(GenomeVector* vector, QObject* parent=0);
		bool appendRow();
		bool removeRow(int row);
		GenomeVector* genomeVector() const;

		// the QAbstractListModel interface
		int rowCount(const QModelIndex& parent=QModelIndex()) const;
		QVariant data(const QModelIndex& idx, int role=Qt::DisplayRole) const;
		bool setData(const QModelIndex& idx, const QVariant& value, int role=Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex&) const;
		bool hasIndex(int row, int column, const QModelIndex& parent=QModelIndex()) const;
		QVariant headerData(int section, Qt::Orientation orientation,
							int role=Qt::DisplayRole) const;
		Qt::DropActions supportedDropActions() const;

};


#endif
