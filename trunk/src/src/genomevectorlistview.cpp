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
#include "logger.h"


GenomeVectorListView::GenomeVectorListView(QWidget* parent)
	: QListView(parent)
{
	setItemDelegate(new GenomeVectorListModelItemDelegate);
}

void GenomeVectorListView::mousePressEvent(QMouseEvent* event)
{
	QListView::mousePressEvent(event);
	dragStartIndex = indexAt(event->pos());
}

void GenomeVectorListView::dropEvent(QDropEvent* event)
{
	if (!(event->source() == this && (event->possibleActions() & Qt::MoveAction)))
		return;

	QListView::dropEvent(event);
	event->acceptProposedAction();
	// Process the data from the event.
	QModelIndexList idxlist;
	idxlist << indexAt(event->pos()) << dragStartIndex;
	reset();
	emit indexesMoved(idxlist);
}

void GenomeVectorListView::commitData(QWidget* editor)
{
	QListView::commitData(editor);
	emit genomesModified();
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
	if (idx > 0)
		m_timeLineEdit->setMinimum( (current - 1)->time );
	else
		m_timeLineEdit->setMinimum( 0.0 );

	if (idx < (genomes->size() - 1))
		m_timeLineEdit->setMaximum( (current + 1)->time );
	else
		m_timeLineEdit->setMaximum( 1000.0 );
}

void GenomeVectorListModelItemEditor::setModelData(GenomeVector* genomes, int idx)
{
	(genomes->data() + idx)->time = m_timeLineEdit->value();
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
	const GenomeVectorListModel* model = static_cast<const GenomeVectorListModel*>(index.model());
	editor->setEditorData(model->genomeVector(), index.row());
}

void GenomeVectorListModelItemDelegate::setModelData(QWidget* widget, QAbstractItemModel* model,
	 const QModelIndex& index) const
{
	logFine(QString("GenomeVectorListModelItemDelegate::setModelData : %1").arg(index.row()));
	GenomeVectorListModelItemEditor* editor = static_cast<GenomeVectorListModelItemEditor*>(widget);
	GenomeVectorListModel* vmodel = static_cast<GenomeVectorListModel*>(model);
	editor->setModelData(vmodel->genomeVector(), index.row());
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
