/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
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

#include <QMenu>
#include <QLabel>
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
		int selectedIndex();

	signals:
		void colorSelected(double);
		void undoStateSignal();

	public slots:
		void repaintLabel();
		void toggleShowHistAction(bool);

	private:
		GenomeVector* genome;
		QMenu* popupMenu;
		QAction* histMenuAction;
		int selected_y;
		int last_y;
		bool show_histogram;
};


#endif
