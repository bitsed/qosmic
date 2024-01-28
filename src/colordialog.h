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
#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include "colorselector.h"

class HueSatSelector : public ColorSelector
{
	int value;

	public:
		HueSatSelector(QWidget* parent=0);
		void setSelectedIndex(QColor);
		void repaintLabel();
};

class ValSelector : public ColorSelector
{
	public:
		ValSelector(QWidget* parent=0);
		void setSelectedIndex(QColor);
		void repaintLabel();
};



#include "ui_colordialog.h"


class ColorDialog : public QDialog, private Ui::ColorDialog
{
	Q_OBJECT

	public:
		ColorDialog(QWidget* parent=0);
		~ColorDialog();
		QColor getSelectedColor();
		void setSelectedColor(QColor);
		void setAlphaEnabled(bool);

	private slots:
		void indexSelected(QPoint);
		void rgbSpinValueChanged();
		void hsvSpinValueChanged();

	signals:
		void colorSelected(QColor);

	private:
		QColor selectedColor;

};



#endif
