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

#ifndef COLORLABEL_H
#define COLORLABEL_H

#include <QLabel>
#include <QColor>
#include <QMouseEvent>

class ColorLabel : public QLabel
{
	Q_OBJECT

	public:
		ColorLabel(QWidget* parent=0);
		void setColor(QColor);
		void mousePressEvent (QMouseEvent*);

	signals:
		void colorSelected(QColor);
		void undoStateSignal();

	private:
		QColor lastcolor;
};


#endif
