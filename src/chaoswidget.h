/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
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

#ifndef CHAOSWIDGET_H
#define CHAOSWIDGET_H

#include <QWidget>
#include <QWheelEvent>
#include <QStandardItemModel>
#include <QTableView>

#include "genomevector.h"
#include "triangle.h"

class ChaosTableView : public QTableView
{
	Q_OBJECT

	public:
		ChaosTableView(QWidget* =0);
		void setPrecision(int);
		int precision();
		void restoreSettings();

	signals:
		void valueUpdated();
		void undoStateSignal();
		void precisionChanged();

	protected:
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void keyPressEvent(QKeyEvent*);
		void resetChaosValue(QModelIndex&);

	protected slots:
		void commitData(QWidget*);

	private:
		double step;
		double start_value;
		QModelIndex start_item;
		QPointF last_pos;
		int vars_precision;
};

#include "ui_chaoswidget.h"

class ChaosWidget : public QWidget, private Ui::ChaosWidget
{
	Q_OBJECT

	public:
		ChaosWidget(GenomeVector*, QWidget* parent=0);
		~ChaosWidget();

	public slots:
		void triangleSelectedSlot(Triangle* t);
		void updateFormData();

	signals:
		void dataChanged();
		void undoStateSignal();

	protected:
		void wheelEvent(QWheelEvent*);

	private:
		GenomeVector* genomes;
		Triangle* selectedTriangle;
		QStandardItemModel* model;
};

#endif
