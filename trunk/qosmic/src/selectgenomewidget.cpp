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
#include <QPainter>
#include <QSettings>

#include "selectgenomewidget.h"
#include "viewerpresetsmodel.h"
#include "logger.h"

SelectGenomeWidget::SelectGenomeWidget(GenomeVector* g, QWidget* parent)
: QWidget(parent), QosmicWidget(this, "SelectGenomeWidget"), genomes(g)
{
	setupUi(this);

	connect(m_addButton, SIGNAL(pressed()), this, SLOT(addButtonPressedSlot()));
	connect(m_deleteButton, SIGNAL(pressed()), this, SLOT(delButtonPressedSlot()));
	connect(m_clearTrianglesButton, SIGNAL(pressed()), this, SLOT(clearTrianglesButtonPressedSlot()));
	connect(m_configButton, SIGNAL(pressed()), this, SLOT(configButtonPressedSlot()));

	m_genomesListView->setModel(genomes);
	m_genomesListView->setGridSize(genomes->previewSize() + QSize(4,4));

	connect(m_genomesListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(listViewClickedAction(const QModelIndex&)));
	connect(m_genomesListView, SIGNAL(indexesMoved(const QModelIndexList&)), this, SLOT(indexesMovedSlot(const QModelIndexList&)));
	connect(m_genomesListView, SIGNAL(genomesModified()), this, SIGNAL(genomesModified()));
}

void SelectGenomeWidget::listViewClickedAction(const QModelIndex& idx)
{
	int row(idx.row());
	if (row != genomes->selected())
	{
		logFine("SelectGenomeWidget::genomeSelectedAction : genome %d selected", row + 1);
		genomes->setSelected(row);
		emit genomeSelected(row);
	}
}

void SelectGenomeWidget::setSelectedGenome(int n)
{
	logInfo("SelectGenomeWidget::setSelectedGenome : selecting genome %d", n);
	genomes->setSelected(n);
	m_genomesListView->setCurrentIndex(genomes->index(n));
	emit genomeSelected(n);
}

void SelectGenomeWidget::reset()
{
	logFine("SelectGenomeWidget::reset : genomes %d", genomes->size());
	if (genomes->size() > 0)
	{
		double ltime = -1.0;
		for (int i = 0 ; i < genomes->size() ; i++)
		{
			flam3_genome* g = genomes->data() + i;
			if ( g->time <= ltime ) // "normalize" the flam3_genome.time attributes.
				g->time = ltime + 1.0;
			ltime = g->time;
		}
		m_genomesListView->selectionModel()->setCurrentIndex(genomes->selectedIndex(), QItemSelectionModel::ClearAndSelect);
	}
}

void SelectGenomeWidget::addButtonPressedSlot()
{
	int lastIdx = genomes->rowCount() - 1;
	double ltime = (genomes->data() + lastIdx)->time;
	if (genomes->appendRow())
	{
		lastIdx += 1;
		(genomes->data() + lastIdx)->time = ltime + 1.0;
		genomes->setSelected(lastIdx);
		emit genomesModified();
	}
}

void SelectGenomeWidget::delButtonPressedSlot()
{
	if (genomes->size() <= 1)
		// don't remove the only genome, just clear it
		clearTrianglesButtonPressedSlot();
	else
	{
		int idx = genomes->selected();
		if (genomes->removeRow(idx))
			emit genomesModified();
	}
}

void SelectGenomeWidget::configButtonPressedSlot()
{
	SelectGenomeConfigDialog d(this);
	QSize l_size(genomes->previewSize());
	QString l_preset(genomes->previewPreset());
	d.setPreviewSize(l_size);
	d.setPreset(l_preset);
	d.move(QCursor::pos());
	d.exec();
	QSize s = d.previewSize();
	QString p = d.preset();
	bool render = false;
	if (s != l_size)
	{
		genomes->setPreviewSize(s);
		m_genomesListView->setGridSize(s + QSize(4,4));
		render = true;
	}
	if (p != l_preset)
	{
		genomes->setPreviewPreset(p);
		render = true;
	}
	if (render)
	{
		genomes->updatePreviews();
		m_genomesListView->reset();
	}
}

void SelectGenomeWidget::clearTrianglesButtonPressedSlot()
{
	int idx = genomes->selected();
	flam3_genome* g = genomes->data() + idx;
	if (g && g->xform && g->num_xforms > 0)
	{
		while (g->num_xforms > 0)
			flam3_delete_xform(g, g->num_xforms - 1);

		UndoState* state = genomes->undoRing(idx)->advance();
		flam3_copy(&(state->Genome), g);

		genomes->updatePreview(idx);

		emit genomesModified();
	}
}

void SelectGenomeWidget::indexesMovedSlot(const QModelIndexList& idxList)
{
	int drop_row = idxList[0].row();
	int drag_row = idxList[1].row();
	int selected = genomes->selected();
	genomes->moveRow(drag_row, drop_row);

	// reselect the currently selected index
	blockSignals(true);
	if (drag_row == selected)
		setSelectedGenome(drop_row);
	else if ((drag_row < selected) && (selected <= drop_row))
		setSelectedGenome(selected - 1);
	else if (drag_row > selected && selected >= drop_row)
		setSelectedGenome(selected + 1);
	blockSignals(false);
}


//-------------------------------------------------------------------------------------------

SelectGenomeConfigDialog::SelectGenomeConfigDialog(QWidget* parent) : QDialog(parent)
{
	setupUi(this);
	qualityComboBox->setModel(ViewerPresetsModel::getInstance());
}

void SelectGenomeConfigDialog::setPreviewSize(const QSize& size)
{
	sizewLineEdit->updateValue(size.width());
	sizehLineEdit->updateValue(size.height());
}

QSize SelectGenomeConfigDialog::previewSize() const
{
	return QSize(sizewLineEdit->value(), sizehLineEdit->value());
}

QString SelectGenomeConfigDialog::preset() const
{
	return qualityComboBox->currentText();
}

void SelectGenomeConfigDialog::setPreset(const QString& s)
{
	qualityComboBox->setCurrentIndex(qualityComboBox->findText(s));
}

void SelectGenomeWidget::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
		genomes->usingPreviews(true);
}

void SelectGenomeWidget::hideEvent(QHideEvent* e)
{
	if (!e->spontaneous())
		genomes->usingPreviews(false);
}
