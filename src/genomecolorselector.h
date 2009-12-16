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

#ifndef GENOMECOLORSELECTOR_H
#define GENOMECOLORSELECTOR_H

#include <QLabel>
#include <QPainter>
#include <QMouseEvent>

#include "genomevector.h"

class GenomeColorSelector : public QLabel
{
	Q_OBJECT

	public:
		GenomeColorSelector(QWidget* parent=0, GenomeVector* g=0);
		void setGenome(int);
		void setGenomeVector(GenomeVector*);
		void mousePressEvent (QMouseEvent*);
		void mouseMoveEvent (QMouseEvent*);
		void mouseReleaseEvent (QMouseEvent*);
		void setSelectedIndex(int);

	signals:
		void colorSelected(double);
		void colorChanged(double);
		void undoStateSignal();

	protected:
		void repaintLabel();

	private:
		int genome_offset;
		GenomeVector* genome;
		int last_y;
};


#endif
