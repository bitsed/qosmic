/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009  by David Bitseff                      *
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

#ifndef TRIANGLECOORDSWIDGET_H
#define TRIANGLECOORDSWIDGET_H

#include <QWidget>

#include "ui_trianglecoordswidget.h"

class Triangle;
class GenomeVector;

class TriangleCoordsWidget : public QWidget, private Ui::TriangleCoordsWidget
{
	Q_OBJECT

	public:
		enum CoordType { Rect, Degree, Radian };

		TriangleCoordsWidget(GenomeVector*, QWidget* parent=0);
		void setGenome(int);
		void updateFormData();
		CoordType coordType();

	signals:
		void dataChanged();
		void undoStateSignal();

	public slots:
		void triangleSelectedSlot(Triangle*);
		void triangleModifiedSlot(Triangle*);

	protected slots:
		void fieldEditedAction();
		void field2EditedAction();
		void coordsButtonGroupClickedAction();

	private:
		int genome_offset;
		GenomeVector* genome;
		Triangle* selectedTriangle;
		QButtonGroup* coordsButtonGroup;
		CoordType coords;
};

#endif
