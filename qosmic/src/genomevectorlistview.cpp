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
#include "genomevectorlistview.h"
#include "mutationwidget.h"
#include "logger.h"


GenomeVectorListView::GenomeVectorListView(QWidget* parent)
	: QListView(parent)
{
	setItemDelegate(new GenomeVectorListModelItemDelegate);
}

void GenomeVectorListView::mousePressEvent(QMouseEvent* event)
{
	// save the currentIndex so it can be restored after a startDrag event
	QListView::mousePressEvent(event);
	dragStartIndex = indexAt(event->pos());
}

void GenomeVectorListView::startDrag(Qt::DropActions supportedActions)
{
	QDrag* drag = new QDrag(this);
	QMimeData* mimeData = new QMimeData;
	mimeData->setData("application/x-qabstractitemmodeldatalist", QByteArray::number(dragStartIndex.row()));
	drag->setMimeData(mimeData);
	drag->setPixmap(model()->data(dragStartIndex, Qt::DecorationRole).value<QPixmap>());
	drag->exec(supportedActions);
	selectionModel()->select(qobject_cast<GenomeVector*>(model())->selectedIndex(), QItemSelectionModel::ClearAndSelect);
}

void GenomeVectorListView::dropEvent(QDropEvent* event)
{
	if ((event->source() == this && (event->possibleActions() & Qt::MoveAction)))
	{
		QListView::dropEvent(event);
		event->acceptProposedAction();
		// Process the data from the event.
		QModelIndexList idxlist;
		idxlist << indexAt(event->pos()) << dragStartIndex;
		emit indexesMoved(idxlist);
	}
	else
	{
		if (event->mimeData()->hasFormat("application/x-mutationpreviewwidget"))
		{
			QModelIndex idx = indexAt(event->pos());
			flam3_genome* mutation = qobject_cast<MutationPreviewWidget*>(event->source())->genome();
			GenomeVector* genomes = qobject_cast<GenomeVector*>(model());
			if (idx.isValid())
			{
				// drop on an existing genome
				flam3_genome* g = genomes->data() + idx.row();
				double time = g->time;
				flam3_copy(g, mutation);
				g->time = time;
				genomes->addUndoState(idx.row());
				genomes->updatePreview(idx.row());
				if (genomes->selected() != idx.row())
					selectionModel()->select(genomes->selectedIndex(), QItemSelectionModel::ClearAndSelect);
				else
					emit genomesModified();
			}
			else
			{
				// add a new genome to the list
				double time = genomes->last().time + 1.0;
				flam3_genome tmp = flam3_genome();
				flam3_copy(&tmp, mutation);
				tmp.time = time;
				genomes->appendRow(tmp);
				selectionModel()->select(genomes->selectedIndex(), QItemSelectionModel::ClearAndSelect);
			}
			event->acceptProposedAction();
		}

	}
}

void GenomeVectorListView::dragEnterEvent(QDragEnterEvent* event)
{
	if ((event->source() == this && (event->possibleActions() & Qt::MoveAction)))
		QListView::dragEnterEvent(event);
	else
	{
		if (event->mimeData()->hasFormat("application/x-mutationpreviewwidget"))
			event->acceptProposedAction();
	}
}

void GenomeVectorListView::dragMoveEvent(QDragMoveEvent* event)
{
	if ((event->source() == this && (event->possibleActions() & Qt::MoveAction)))
		QListView::dragMoveEvent(event);
	else
	{
		if (event->mimeData()->hasFormat("application/x-mutationpreviewwidget"))
		{
			selectionModel()->select(indexAt(event->pos()), QItemSelectionModel::ClearAndSelect);
			event->acceptProposedAction();
		}
	}
}

void GenomeVectorListView::commitData(QWidget* editor)
{
	QListView::commitData(editor);
	QAbstractListModel* m = qobject_cast<QAbstractListModel*>(model());
	for (int n = 0 ; n < m->rowCount() ; n++ )
		update(m->index(n));
}

//-------------------------------------------------------------------------------------------


GenomeVectorListModelItemEditor::GenomeVectorListModelItemEditor(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
	connect(m_closeButton, SIGNAL(clicked()), this, SIGNAL(editingFinished()));
}

void GenomeVectorListModelItemEditor::setEditorData(GenomeVector* genomes, int idx)
{
	flam3_genome* current = genomes->data() + idx;
	m_timeLineEdit->updateValue(current->time);
	m_timeLineEdit->setMinimum( idx );
}

void GenomeVectorListModelItemEditor::setModelData(GenomeVector* genomes, int idx)
{
	int ngenomes = genomes->size();
	double time = qMax(idx, m_timeLineEdit->value());
	(genomes->data() + idx)->time = time;
	for (int i = idx - 1, t = time - 1 ; i >= 0 ; i--, t--)
	{
		if (t < (genomes->data() + i)->time)
			(genomes->data() + i)->time = t;
	}
	for (int i = idx + 1, t = time + 1 ; i < ngenomes ; i++, t++)
	{
		if (t > (genomes->data() + i)->time)
			(genomes->data() + i)->time = t;
	}
}


void GenomeVectorListModelItemEditor::mouseDoubleClickEvent(QMouseEvent* /*event*/)
{
	emit editingFinished();
}

//-------------------------------------------------------------------------------------------


GenomeVectorListModelItemDelegate::GenomeVectorListModelItemDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
}

QSize GenomeVectorListModelItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return QStyledItemDelegate::sizeHint(option, index);
}

QWidget* GenomeVectorListModelItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/,
	const QModelIndex& /*index*/) const
{
	GenomeVectorListModelItemEditor* editor = new GenomeVectorListModelItemEditor(parent);
	connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
	return editor;
}

void GenomeVectorListModelItemDelegate::setEditorData(QWidget* widget, const QModelIndex& index) const
{
	GenomeVectorListModelItemEditor* editor = static_cast<GenomeVectorListModelItemEditor*>(widget);
	const GenomeVector* model = static_cast<const GenomeVector*>(index.model());
	editor->setEditorData(const_cast<GenomeVector*>(model), index.row());
}

void GenomeVectorListModelItemDelegate::setModelData(QWidget* widget, QAbstractItemModel* model,
	 const QModelIndex& index) const
{
	logFine(QString("GenomeVectorListModelItemDelegate::setModelData : %1").arg(index.row()));
	GenomeVectorListModelItemEditor* editor = static_cast<GenomeVectorListModelItemEditor*>(widget);
	GenomeVector* vmodel = static_cast<GenomeVector*>(model);
	editor->setModelData(vmodel, index.row());
}

void GenomeVectorListModelItemDelegate::updateEditorGeometry(QWidget* editor,
	const QStyleOptionViewItem& option, const QModelIndex&/* index */) const
{
	editor->setGeometry(option.rect);
}


void GenomeVectorListModelItemDelegate::commitAndCloseEditor()
{
	logFine(QString("GenomeVectorListModelItemDelegate::commitAndCloseEditor : enter"));

	GenomeVectorListModelItemEditor* editor = static_cast<GenomeVectorListModelItemEditor*>(sender());
	emit commitData(editor);
	emit closeEditor(editor);
}
