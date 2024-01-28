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
#ifndef SNAPSLIDER_H
#define SNAPSLIDER_H

#include <QSlider>

class SnapSlider : public QSlider
{
	Q_OBJECT

	public:
		SnapSlider(QWidget* parent=0);
		int selectedValue();
		void setSnap(int value);
		void setToSnap();
		int snap();
		int dx();

	signals:
		void valueSelected(int);
		void undoStateSignal();

	private slots:
		void sliderReleasedAction();
		void rangeChangedAction(int, int);
		void sliderChangedAction(int);

	private:
		int m_selected_value;
		int m_last_value;
		int m_dx;
		int m_snap;
};

#endif


