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
#ifndef SELECTTRIANGLEWIDGET_H
#define SELECTTRIANGLEWIDGET_H

#include "ui_selecttrianglewidget.h"
#include "qosmicwidget.h"
#include "xfedit.h"


class SelectTriangleWidget
	: public QWidget, public QosmicWidget, private Ui::SelectTriangleWidget
{
	Q_OBJECT

	Triangle* selectedTriangle;
	FigureEditor* triangleScene;
	GenomeVector* genome;

	public:
		SelectTriangleWidget(GenomeVector*, QWidget* =0);
		~SelectTriangleWidget();

	signals:
		void dataChanged();
		void undoStateSignal();

	public slots:
		void triangleSelectedSlot(Triangle* t);
		void reset();

	protected slots:
		void fieldEditedAction();
		void triangleSelectedSlot(int);
		void addTriangleAction();
		void delTriangleAction();
		void finalStateChangedSlot(bool);
};


#endif
