/***************************************************************************
 *   Copyright (C) 2007, 2010 by David Bitseff                             *
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
#ifndef VARIATIONSWIDGET_H
#define VARIATIONSWIDGET_H

#include <QWidget>
#include <QWheelEvent>

#include "ui_variationswidget.h"
#include "genomevector.h"

class Triangle;
class VarsTableModel;

class VariationsWidget : public QWidget, private Ui::VariationsWidget
{
	Q_OBJECT

	public:
		VariationsWidget(GenomeVector*, QWidget* parent=0);
		~VariationsWidget();

	public slots:
		void triangleSelectedSlot(Triangle* t);
		void updateFormData();

	signals:
		void dataChanged();
		void undoStateSignal();

	protected:
		void wheelEvent(QWheelEvent*);
		void resetVariationSelector();

	protected slots:
		void variationEditedSlot(int);
		void valueEditorUpdatedSlot();
		void variationSelectedSlot(int);
		void addVariationValueSlot();

	private:
		GenomeVector* genome;
		Triangle* selectedTriangle;
		VarsTableModel* model;
		QString lastVariation;
};

#include "triangle.h"
#include "varstablewidget.h"

#endif
