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
#include <QMimeData>
#include <QDrag>
#include <QUrl>

#include "genomevectorlistview.h"
#include "flam3filestream.h"
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

void GenomeVectorListView::rowsInserted(const QModelIndex& /*parent*/, int start, int /*end*/)
{
	GenomeVector* genomes = qobject_cast<GenomeVector*>(model());
	genomes->setSelected(start);
	setCurrentIndex(genomes->selectedIndex());
	emit genomesModified();
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

		// Process the data from the event.
		QModelIndexList idxList;
		idxList << indexAt(event->pos()) << dragStartIndex;

		GenomeVector* genomes = qobject_cast<GenomeVector*>(model());
		int drop_row = idxList.at(0).row();
		int drag_row = idxList.at(1).row();
		int selected = genomes->selected();
		genomes->moveRow(drag_row, drop_row);

		// reselect the currently selected index
		int select_next = selected;
		if (drag_row == selected)
			select_next = drop_row;
		else if ((drag_row < selected) && (selected <= drop_row))
			select_next = selected - 1;
		else if (drag_row > selected && selected >= drop_row)
			select_next = selected + 1;

		genomes->setSelected(select_next);
		Flam3FileStream::autoSave(genomes);
		event->acceptProposedAction();
		emit indexesMoved(idxList);
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
			}
			Flam3FileStream::autoSave(genomes);
			event->acceptProposedAction();
		}
		else if (event->mimeData()->hasFormat("text/uri-list"))
		{
			QUrl url = event->mimeData()->urls().first();
			QString path = url.path();
			if (url.isValid() && path.contains(QRegExp("\\.flam[3e]?$")))
			{
				GenomeVector* genomes = qobject_cast<GenomeVector*>(model());
				QFile file(path);
				Flam3FileStream s(&file);
				s >> genomes;
				Flam3FileStream::autoSave(genomes);
				event->acceptProposedAction();
			}
		}
	}
}

void GenomeVectorListView::dragEnterEvent(QDragEnterEvent* event)
{
	if ((event->source() == this && (event->possibleActions() & Qt::MoveAction)))
		QListView::dragEnterEvent(event);
	else if (event->mimeData()->hasFormat("application/x-mutationpreviewwidget"))
		event->acceptProposedAction();
	else if (event->mimeData()->hasFormat("text/uri-list"))
	{
		QUrl url = event->mimeData()->urls().first();
		QString path = url.path();
		if (url.isValid() && path.contains(QRegExp("\\.flam[3e]?$")))
			event->acceptProposedAction();
	}
	else
		logWarn(QString("GenomeVectorListView::dragEnterEvent : unknown mimeData %1")
				.arg(event->mimeData()->formats().join(", ")));
}

void GenomeVectorListView::dragMoveEvent(QDragMoveEvent* event)
{
	if ((event->source() == this && (event->possibleActions() & Qt::MoveAction)))
		QListView::dragMoveEvent(event);
	else
	{
		if (event->mimeData()->hasFormat("application/x-mutationpreviewwidget")
		 || event->mimeData()->hasFormat("text/uri-list"))
		{
			QListView::dragMoveEvent(event);
			event->acceptProposedAction();
			viewport()->update(); // needed to draw the drop indicator
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
