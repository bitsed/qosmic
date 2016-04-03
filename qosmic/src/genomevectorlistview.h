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
#ifndef GENOMEVECTORLISTVIEW_H
#define GENOMEVECTORLISTVIEW_H

#include <QListView>
#include <QStyledItemDelegate>

#include "ui_genomevectorlistmodelitemeditor.h"
#include "genomevector.h"

class GenomeVectorListModelItemEditor : public QWidget,
	private Ui::GenomeVectorListModelItemEditor
{
	Q_OBJECT

	public:
		GenomeVectorListModelItemEditor(QWidget* parent=0);
		void setEditorData(GenomeVector* genome, int idx);
		void setModelData(GenomeVector* genome, int idx);

	signals:
		void editingFinished();

	protected:
		void mouseDoubleClickEvent(QMouseEvent* event);
};


class GenomeVectorListModelItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	public:
		GenomeVectorListModelItemDelegate(QObject* parent=0);
		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
			const QModelIndex& index) const;
		void setEditorData(QWidget* widget, const QModelIndex& index) const;
		void setModelData(QWidget* widget, QAbstractItemModel* model,
			const QModelIndex& index) const;
		void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
			const QModelIndex& index ) const;

	private slots:
		void commitAndCloseEditor();
};


class GenomeVectorListView : public QListView
{
	Q_OBJECT

	QModelIndex dragStartIndex;

	public:
		GenomeVectorListView(QWidget* parent=0);

	signals:
		void genomesModified();

	protected slots:
		void commitData(QWidget* editor);
		void rowsInserted(const QModelIndex& parent, int start, int end);

	protected:
		void mousePressEvent(QMouseEvent* event);
		void startDrag(Qt::DropActions actions);
		void dropEvent(QDropEvent* event);
		void dragEnterEvent(QDragEnterEvent* event);
		void dragMoveEvent(QDragMoveEvent* event);
};


#endif
