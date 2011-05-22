/***************************************************************************
 *   Copyright (C) 2007 - 2011 by David Bitseff                            *
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
#include "genomevector.h"
#include "logger.h"


int GenomeVector::selectedIndex() const
{
	return selected_index;
}

void GenomeVector::setSelectedIndex(int value)
{
	selected_index = value;
}

flam3_genome* GenomeVector::selectedGenome()
{
	return data() + selected_index;
}

UndoRing* GenomeVector::undoRing(int idx)
{
	if (idx == -1)
		idx = selected_index;
	return &undoRings[idx];
}

void GenomeVector::append(const flam3_genome& g)
{
	QVector<flam3_genome>::append(g);
	undoRings.append(UndoRing());
	UndoState* state = undoRings.last().advance();
	int idx = size() - 1;
	flam3_copy(&(state->Genome), data() + idx);
}

void GenomeVector::insert(int i, const flam3_genome& g)
{
	QVector<flam3_genome>::insert(i, g);
	undoRings.insert(i, UndoRing());
	UndoState* state = undoRings[i].advance();
	flam3_copy(&(state->Genome), data() + i);
}

void GenomeVector::remove(int i)
{
	if (size() > 0)
	{
		flam3_genome* g = data() + i;
		// free the heap mem used by xforms in existing genomes
		clear_cp(g, flam3_defaults_on);
		QVector<flam3_genome>::remove(i);
		undoRings.removeAt(i);
		// change the selected_index if necessary
		if (selected_index >= i)
			selected_index = qMax(0, selected_index - 1);
	}
}

void GenomeVector::clear()
{
	while (size() > 0)
		remove(size() - 1);
	undoRings.clear();
}


// ----------------------------------------------------------------------------------------

GenomeVectorListModel::GenomeVectorListModel(GenomeVector* vector, QObject* parent)
: QAbstractListModel(parent), genomes(vector)
{

}

GenomeVector* GenomeVectorListModel::genomeVector() const
{
	return genomes;
}

int GenomeVectorListModel::rowCount(const QModelIndex& /*parent*/) const
{
	return genomes->size();
}

QVariant GenomeVectorListModel::data(const QModelIndex& idx, int role) const
{
	if (!idx.isValid())
		return QVariant();

	int row = idx.row();
	if (row > genomes->size() || row < 0)
	{
		logWarn(QString("GenomeVectorListModel::data : genome doesn't exist %1").arg(row));
		return QVariant();
	}

	logFiner(QString("GenomeVectorListModel::data : getting genome %1 role %2").arg(row).arg(role));
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		flam3_genome g = genomes->at(row);
		return QString("%1 xforms\ntime: %2").arg(g.num_xforms).arg(g.time);
	}
	if (role == Qt::DecorationRole)
		return previews.at(row);

	return QVariant();
}

bool GenomeVectorListModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
	if (!idx.isValid())
		return false;

	int row = idx.row();
	if (row > genomes->size() || row < 0)
	{
		logWarn(QString("GenomeVectorListModel::setData : genome doesn't exist %1").arg(row));
		return false;
	}
	if (role == Qt::DecorationRole)
	{
		if (previews.size() > row)
			previews.replace(row, value);
		else
			previews.insert(row, value);
	}
	return false;
}

Qt::ItemFlags GenomeVectorListModel::flags(const QModelIndex& idx) const
{
	if (!idx.isValid())
		return 0;

	if (idx.row() > genomes->size() || idx.row() < 0)
		return 0;

	return Qt::ItemIsSelectable | Qt::ItemIsEnabled |
		Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable ;
}

Qt::DropActions GenomeVectorListModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

bool GenomeVectorListModel::hasIndex(int row, int column, const QModelIndex& /*parent*/) const
{
	if (column == 0 && row < genomes->size())
		return true;
	return false;

}

QVariant GenomeVectorListModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	int row = section;
	return QString("Genome %1").arg(row);
}

bool GenomeVectorListModel::appendRow()
{
	int row = rowCount();
	beginInsertRows(index(0,0).parent(), row, row);
	flam3_genome* current = genomes->selectedGenome();
	flam3_genome gen = flam3_genome();
	flam3_copy(&gen, current);
	// preserve values not copied in flam3_apply_template()
	gen.pixels_per_unit = current->pixels_per_unit;
	gen.contrast   = current->contrast;
	gen.gamma      = current->gamma;
	gen.brightness = current->brightness;
	gen.vibrancy   = current->vibrancy;
	genomes->append(gen);
	if (previews.size() > row)
		previews.replace(row, QVariant());
	else
		previews.insert(row, QVariant());
	endInsertRows();
	return true;
}

bool GenomeVectorListModel::removeRow(int row)
{
	if (0 <= row && row < genomes->size())
	{
		beginRemoveRows(index(0,0).parent(), row, row);
		genomes->remove(row);
		previews.removeAt(row);
		endRemoveRows();
		return true;
	}
	return false;
}
