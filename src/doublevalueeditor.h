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

#ifndef DOUBLEVALUEEDITOR_H
#define DOUBLEVALUEEDITOR_H

#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QWheelEvent>

#include "wheelvalueeditor.h"

class DoubleValueEditor : public QDoubleSpinBox
{
	Q_OBJECT

	public:
		DoubleValueEditor(QWidget* parent=0);
		void updateValue(double);
		void setWheelEventUpdate(bool);
		bool wheelEventUpdate() const;
		void restoreSettings();
		void stepUp();
		void stepDown();

	signals:
		void valueUpdated();
		void undoStateSignal();

	protected:
		void keyPressEvent(QKeyEvent*);
		void wheelEvent(QWheelEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);

	private:
		bool wheelEventSignal;
		double start_value;
		double default_step;
		QPointF start_pos;
		QPointF last_pos;
		QMouseEvent* last_press;
};


#endif
