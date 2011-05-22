/***************************************************************************
 *   Copyright (C) 2007 - 2011 by David Bitseff                            *
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

	private slots:
		void indexSelected(QPoint);

	signals:
		void colorSelected(QColor);

	private:
		QColor selectedColor;

};



#endif
