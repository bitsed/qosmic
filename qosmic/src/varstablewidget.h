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
#ifndef VARSTABLEWIDGET_H
#define VARSTABLEWIDGET_H

#include <QAbstractItemModel>
#include <QTreeView>
#include <QMouseEvent>

#include "flam3util.h"

class VarsTableItem
{
	QList<VarsTableItem*> childItems;
	QList<QVariant> itemData;
	VarsTableItem *parentItem;

	public:
		VarsTableItem(const QList<QVariant>&, VarsTableItem* =0);
		~VarsTableItem();
		VarsTableItem* child(int);
		VarsTableItem* parent();
		int childCount() const;
		int columnCount() const;
		QVariant data(int) const;
		bool setData(int, const QVariant&);
		int row() const;
		void appendChild(VarsTableItem*);
};

class VarsTableModel : public QAbstractItemModel
{
	Q_OBJECT

	VarsTableItem* rootItem;
	flam3_xform* xform;
	int decimals;
	const QBrush activeColor;
	const QBrush inactiveColor;
	QMap<QString, VarsTableItem*> variationItems;

	public:
		VarsTableModel(QObject* parent=0);
		~VarsTableModel();

		QModelIndex index (int, int, const QModelIndex& =QModelIndex()) const;
		int columnCount (const QModelIndex& =QModelIndex()) const;
		int rowCount (const QModelIndex& =QModelIndex()) const;
		QVariant data(const QModelIndex&, int =Qt::DisplayRole) const;
		bool setData(const QModelIndex&, const QVariant&, int =Qt::EditRole);
		QModelIndex parent(const QModelIndex&) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant headerData(int, Qt::Orientation, int =Qt::DisplayRole) const;
		VarsTableItem* getVariation(int row) const;
		VarsTableItem* getItem(const QModelIndex&) const;
		int precision() const;
		void setPrecision(int);
		void setModelData(flam3_xform*);

	private:
		void updateVarsTableItem(VarsTableItem*, double);
};


class VarsTableWidget : public QTreeView
{
	Q_OBJECT

	public:
		VarsTableWidget(QWidget* =0);
		void setPrecision(int);
		int precision();
		void restoreSettings();

	signals:
		void valueUpdated(int);
		void undoStateSignal();
		void precisionChanged();

	protected:
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void keyPressEvent(QKeyEvent*);
		void showHideNullRows();

	protected slots:
		void commitData(QWidget*);
		void dataChanged(const QModelIndex&, const QModelIndex&);

	private:
		double step;
		double start_value;
		QModelIndex start_item;
		QPointF last_pos;
		int vars_precision;
};

#endif
