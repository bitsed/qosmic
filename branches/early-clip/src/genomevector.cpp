/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009 by David Bitseff                       *
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
	flam3_copy(&(state->Genome), data() + (size() - 1));
}

void GenomeVector::remove(int i)
{
	if (size() > 0)
	{
		flam3_genome* g = data() + i;
		if (g && g->xform && g->num_xforms > 0)
			// free the heap mem used by xforms in existing genomes
			while (g->num_xforms > 0)
				flam3_delete_xform(g, g->num_xforms - 1);
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
