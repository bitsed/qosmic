/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009 by David Bitseff                       *
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
#ifndef VARSTABLEWIDGET_H
#define VARSTABLEWIDGET_H

#include <QAbstractItemModel>
#include <QTreeView>
#include <QMouseEvent>

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

	const QBrush activeColor;
	const QBrush inactiveColor;

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

	protected slots:
		void commitData(QWidget*);

	private:
		double step;
		double start_value;
		QModelIndex start_item;
		QPointF last_pos;
		int vars_precision;
};

#endif
