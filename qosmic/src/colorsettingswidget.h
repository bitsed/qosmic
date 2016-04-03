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

#ifndef COLORSETTINGSWIDGET_H
#define COLORSETTINGSWIDGET_H

#include <QWidget>
#include <QColor>

#include "ui_colorsettingswidget.h"
#include "genomevector.h"
#include "triangle.h"


class ColorSettingsWidget : public QWidget, private Ui::ColorSettingsWidget
{
	Q_OBJECT

	public:
		ColorSettingsWidget(GenomeVector*, QWidget* parent=0);
		void updateFormData();
		void updateColorLabel();
		void reset();

	signals:
		void dataChanged();
		void colorChanged(double);
		void colorSelected(double);
		void undoStateSignal();

	public slots:
		void triangleSelectedSlot(Triangle*);

	protected slots:
		void fieldEditedAction();
		void colorChangedAction();
		void colorSelectedAction(double);
		void changeBackground(QColor);

	protected:
		void showEvent(QShowEvent*);

	private:
		GenomeVector* genome;
		Triangle* selectedTriangle;
};


#endif
