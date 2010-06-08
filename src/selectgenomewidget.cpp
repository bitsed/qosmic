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
#include <QPainter>
#include <QSettings>

#include "selectgenomewidget.h"
#include "logger.h"

SelectGenomeWidget::SelectGenomeWidget(GenomeVector* g, RenderThread* r, QWidget* parent)
: QWidget(parent), QosmicWidget(this, "SelectGenomeWidget"), r_thread(r)
{
	setupUi(this);

	label_size = QSettings().value("selectgenomewidget/labelsize", QSize(72, 56)).toSize();

	connect(m_genomeSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectorIndexChangedSlot(int)));
	connect(r_thread, SIGNAL(flameRendered(RenderEvent*)), this, SLOT(flameRenderedAction(RenderEvent*)));
	connect(m_addButton, SIGNAL(pressed()), this, SLOT(addButtonPressedSlot()));
	connect(m_deleteButton, SIGNAL(pressed()), this, SLOT(delButtonPressedSlot()));
	connect(m_clearTrianglesButton, SIGNAL(pressed()), this, SLOT(clearTrianglesButtonPressedSlot()));
	connect(m_configButton, SIGNAL(pressed()), this, SLOT(configButtonPressedSlot()));

	model = new GenomeVectorListModel(g);
	m_genomesListView->setModel(model);
	m_genomesListView->setGridSize(label_size + QSize(4,4));
	connect(m_genomesListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(genomeSelectedAction(const QModelIndex&)));
	connect(m_genomesListView, SIGNAL(indexesMoved(const QModelIndexList&)), this, SLOT(indexesMovedSlot(const QModelIndexList&)));
	connect(m_genomesListView, SIGNAL(genomesModified()), this, SIGNAL(genomesModified()));

	setSelectedGenome(0);
}

void SelectGenomeWidget::genomeSelectedAction(const QModelIndex& idx)
{
	int row = idx.row();
	logFine(QString("SelectGenomeWidget::genomeSelectedAction : genome %1 selected").arg(row + 1));
	m_genomeSelector->blockSignals(true);
	m_genomeSelector->setCurrentIndex(row);
	m_genomeSelector->blockSignals(false);
	model->genomeVector()->setSelectedIndex(row);
	emit genomeSelected(row);
}


void SelectGenomeWidget::setSelectedGenome(int n)
{
	model->genomeVector()->setSelectedIndex(n);
	reset();
	m_genomesListView->setCurrentIndex(model->index(n));
}

void SelectGenomeWidget::selectorIndexChangedSlot(int idx)
{
	model->genomeVector()->setSelectedIndex(idx);
	m_genomesListView->setCurrentIndex(model->index(idx));
	emit genomeSelected(idx);
}

void SelectGenomeWidget::updateSelectedPreview()
{
	if (isVisible())
		r_thread->render(r_requests[model->genomeVector()->selectedIndex()]);
}

void SelectGenomeWidget::flameRenderedAction(RenderEvent* e)
{
	RenderRequest* req = e->request();
	if (!(req->type() == RenderRequest::Queued
			   && r_requests.contains(req)) )
		return;

	int idx = r_requests.indexOf(req, 0);
	logFine(QString("SelectGenomeWidget::flameRenderedAction : setting genome %1,g=0x%2,req=0x%3")
			.arg(idx).arg((long)req->genome(),0,16).arg((long)req,0,16));

	QModelIndex model_idx = model->index(idx);
	model->setData(model_idx, QPixmap::fromImage(req->image()), Qt::DecorationRole);
	m_genomesListView->update(model_idx);
	e->accept();
}

void SelectGenomeWidget::reset()
{
	GenomeVector* genomes = model->genomeVector();
	logFine(QString("SelectGenomeWidget::reset : genomes %1, requests %2")
			.arg(genomes->size()).arg(r_requests.size()));
	if (genomes->size() < 1) return;

	m_genomeSelector->blockSignals(true);
	m_genomeSelector->clear();
	for (int i = 1 ; i <= genomes->size() ; i++)
		m_genomeSelector->addItem(QString::number(i));
	m_genomeSelector->setCurrentIndex(genomes->selectedIndex());
	m_genomeSelector->blockSignals(false);

	int thneeds = genomes->size() - r_requests.size();
	if (thneeds > 0)
	{
		for (int n = 0 ; n < thneeds ; n++)
			r_requests.append(new RenderRequest(0, label_size, "selector", RenderRequest::Queued));
	}

	logFine(QString("SelectGenomeWidget::reset : genomes %1, requests %2")
			.arg(genomes->size()).arg(r_requests.size()));

	clearPreviews();

	double ltime = -1.0;
	genomes->data()->time = -1.0;
	for (int i = 0 ; i < genomes->size() ; i++)
	{
		logFine(QString("SelectGenomeWidget::reset : sending request %1").arg(i));
		RenderRequest* req = r_requests[i];
		flam3_genome* g = genomes->data() + i;
		if ( g->time <= ltime ) // "normalize" the flam3_genome.time attributes.
			g->time = ltime + 1.0;
		ltime = g->time;
		req->setGenome(g);
		if (isVisible())
			r_thread->render(req);
	}
}

void SelectGenomeWidget::clearPreviews()
{
	QImage img(label_size, QImage::Format_RGB32);
	QPainter p(&img);
	QRect bounds(QPoint(0,0), label_size);
	p.fillRect(bounds, Qt::white);
	int lx = label_size.width()  / 2 - 8;
	int ly = label_size.height() / 2 - 8;
	p.drawPixmap( lx, ly, 16, 16, QPixmap(":icons/silk/clock.xpm"));
	p.setPen(Qt::gray);
	p.setBrush(QBrush(Qt::gray, Qt::NoBrush));
	p.drawRect(bounds.adjusted(0, 0, -1, -1));
	QPixmap clock(QPixmap::fromImage(img));
	for (int n = 0 ; n < model->rowCount() ; n++)
	{
		QModelIndex model_idx = model->index(n);
		model->setData(model_idx, clock, Qt::DecorationRole);
	}
	m_genomesListView->reset();
}

void SelectGenomeWidget::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
		for (int i = 0 ; i < model->genomeVector()->size() ; i++)
			r_thread->render(r_requests[i]);
}

void SelectGenomeWidget::closeEvent(QCloseEvent* e)
{
	QSettings().setValue("selectgenomewidget/labelsize", label_size);
	e->accept();
}

void SelectGenomeWidget::addButtonPressedSlot()
{
	GenomeVector* genomes = model->genomeVector();
	int lastIdx = model->rowCount() - 1;
	double ltime = (genomes->data() + lastIdx)->time;
	if (model->appendRow())
	{
		(genomes->data() + lastIdx + 1)->time = ltime + 1.0;
		setSelectedGenome(model->rowCount() - 1);
		emit genomesModified();
	}
}


void SelectGenomeWidget::delButtonPressedSlot()
{
	int idx = model->genomeVector()->selectedIndex();
	if (model->removeRow(idx))
	{
		setSelectedGenome(qMax(0, idx - 1));
		emit genomesModified();
	}
}

void SelectGenomeWidget::configButtonPressedSlot()
{
	SelectGenomeConfigDialog d(this);
	d.setPreviewSize(label_size);
	d.move(QCursor::pos());
	d.exec();
	QSize s = d.previewSize();
	if (s != label_size)
	{
		label_size = s;
		m_genomesListView->setGridSize(label_size + QSize(4,4));
		clearPreviews();
		foreach (RenderRequest* r, r_requests)
		{
			r->setSize(label_size);
			r_thread->render(r);
		}
	}
}

void SelectGenomeWidget::clearTrianglesButtonPressedSlot()
{
	int idx = model->genomeVector()->selectedIndex();
	flam3_genome* g = model->genomeVector()->data() + idx;
	if (g && g->xform && g->num_xforms > 0)
	{
		while (g->num_xforms > 0)
			flam3_delete_xform(g, g->num_xforms - 1);

		r_thread->render(r_requests[idx]);

		UndoState* state = model->genomeVector()->undoRing(idx)->advance();
		flam3_copy(&(state->Genome), g);

		emit genomesModified();
	}
}

void SelectGenomeWidget::indexesMovedSlot(const QModelIndexList& idxList)
{
	QVector<double> times;
	int drop_row  = idxList[0].row();
	int drag_row = idxList[1].row();
	logFine(QString("SelectGenomeWidget::indexesMovedSlot : dropped at row %1 from %2").arg(drop_row).arg(drag_row));
	GenomeVector* genomes = model->genomeVector();
	flam3_genome* drag_genome = genomes->data() + drag_row;
	flam3_genome swap_genome;
	Util::init_genome(&swap_genome);
	flam3_copy(&swap_genome, drag_genome);

	for (int n = 0 ; n < genomes->size() ; n++)
		times.append((genomes->data() + n)->time);

	if (drag_row < drop_row) // move above to lower
	{
		genomes->insert(drop_row, swap_genome);
		genomes->remove(drag_row);
		drop_row -= 1;
	}
	else
	{
		genomes->remove(drag_row);
		genomes->insert(drop_row, swap_genome);
	}

	for (int n = 0 ; n < genomes->size() ; n++)
		(genomes->data() + n)->time = times[n];

	setSelectedGenome(drop_row);
	emit genomesModified();
}


//-------------------------------------------------------------------------------------------

SelectGenomeConfigDialog::SelectGenomeConfigDialog(QWidget* parent) : QDialog(parent)
{
	setupUi(this);
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

