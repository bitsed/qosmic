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

	private:
		int genome_offset;
		GenomeVector* genome;
		Triangle* selectedTriangle;
		QButtonGroup* coordsButtonGroup;
		CoordType coords;

	public:
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
		void fieldCoordsEditedAction();
		void fieldVariablesEditedAction();
		void coordsButtonGroupClickedAction();

	protected:
		void showEvent(QShowEvent*);

	private:
		void updateCoordsFormData();
		void updateVariablesFormData();
};

#endif
