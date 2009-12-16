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
#ifndef GENOMEVECTOR_H
#define GENOMEVECTOR_H

#include <QVector>
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
		void remove(int);
		void clear();
};

#endif
