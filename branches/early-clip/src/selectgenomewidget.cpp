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
#include <QDockWidget>
#include <QSettings>
#include <QScrollBar>

#include "selectgenomewidget.h"
#include "logger.h"

SelectGenomeWidget::SelectGenomeWidget(GenomeVector* g, RenderThread* r, QWidget* parent)
: QWidget(parent), QosmicWidget(this, "SelectGenomeWidget"), genome(g), r_thread(r)
{
	setupUi(this);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->setContentsMargins(2,2,2,2);
	m_buttonFrame->setLayout(layout);
	m_buttonFrame->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
	scrollArea->setWidgetResizable(false);
	scrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	label_size = QSettings().value("selectgenomewidget/labelsize", QSize(80, 80)).toSize();
	setSelectedGenome(0);
	connect(m_genomeSelector, SIGNAL(currentIndexChanged(int)),
			this, SLOT(selectorIndexChangedSlot(int)));
	connect(r_thread, SIGNAL(flameRendered(RenderEvent*)),
			this, SLOT(flameRenderedAction(RenderEvent*)));
	connect(m_addButton, SIGNAL(pressed()),
			this, SLOT(addButtonPressedSlot()));
	connect(m_deleteButton, SIGNAL(pressed()),
			this, SLOT(delButtonPressedSlot()));
}

void SelectGenomeWidget::genomeSelectedAction(PreviewWidget* ptr, QMouseEvent* e)
{
	if (e == 0 || (e->buttons() & Qt::LeftButton))
	{
		int idx = labels.indexOf(ptr,0);
		logFine(QString("SelectGenomeWidget::genomeSelectedAction : "
					"genome %1 selected").arg(idx));
		m_genomeSelector->blockSignals(true);
		m_genomeSelector->setCurrentIndex(idx);
		m_genomeSelector->blockSignals(false);
		labels[last_idx]->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
		labels[idx]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
		genome->setSelectedIndex(idx);
		r_thread->render(r_requests[last_idx]);
		last_idx = idx;
		emit genomeSelected(idx);
	}
}


void SelectGenomeWidget::setSelectedGenome(int n)
{
	genome->setSelectedIndex(n);
	reset();
	last_idx = n;
}

void SelectGenomeWidget::selectorIndexChangedSlot(int idx)
{
	genomeSelectedAction(labels[idx], 0);
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
	labels[idx]->setVisible(true);
	labels[idx]->setPixmap(QPixmap::fromImage(req->image()));
	e->accept();
}

void SelectGenomeWidget::reset()
{
	logFine(QString("SelectGenomeWidget::reset : genomes %1, labels %2, requests %3")
			.arg(genome->size()).arg(labels.size()).arg(r_requests.size()));
	if (genome->size() < 1) return;

	m_genomeSelector->blockSignals(true);
	m_genomeSelector->clear();
	for (int i = 1 ; i <= genome->size() ; i++)
		m_genomeSelector->addItem(QString::number(i));
	m_genomeSelector->setCurrentIndex(genome->selectedIndex());
	m_genomeSelector->blockSignals(false);

	if (r_requests.size() < genome->size())
	{
		int thneeds = genome->size() - labels.size();
		for (int n = 0 ; n < thneeds ; n++)
		{
			r_requests.append(new RenderRequest(0, label_size, "selector",
						  RenderRequest::Queued));
		}
	}
	int needs = genome->size() - labels.size();
	for (int n = 0 ; n < needs ; n++)
	{
		PreviewWidget* l = new PreviewWidget(m_buttonFrame);
		l->resize(label_size);
		l->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
		l->setLineWidth(2);
		l->sizePolicy().setHorizontalPolicy(QSizePolicy::Fixed);
		l->sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);
		l->setMinimumSize(label_size);
		l->setMaximumSize(label_size);
		connect(l, SIGNAL(previewClicked(PreviewWidget*,QMouseEvent*)),
			this, SLOT(genomeSelectedAction(PreviewWidget*,QMouseEvent*)));
		labels.append(l);
		m_buttonFrame->layout()->addWidget(l);
	}
	logFine(QString("SelectGenomeWidget::reset : genomes %1, labels %2, requests %3")
			.arg(genome->size()).arg(labels.size()).arg(r_requests.size()));

	for (int n = genome->size() ; n < labels.size() ; n++)
	{
		QLabel* l = labels.at(n);
		l->clear();
		l->setFrameStyle(QFrame::NoFrame);
		l->setVisible(false);
	}
	for (int i = 0 ; i < genome->size() ; i++)
	{
		RenderRequest* req = r_requests[i];
		logFine(QString("SelectGenomeWidget::reset : sending request %1").arg(i));
		req->setGenome(genome->data() + i);
		labels[i]->setGenome(genome->data() + i);
		labels[i]->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
		labels[i]->setVisible(true);
		if (isVisible())
			r_thread->render(req);
	}
	labels[genome->selectedIndex()]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	int min_width  = label_size.width() + 4;
	int min_height =  ( label_size.height() + 4 ) * genome->size();
	scrollAreaWidgetContents->setMinimumHeight( min_height );
	scrollAreaWidgetContents->setMinimumWidth( min_width );
	if (scrollArea->maximumViewportSize().height() < min_height)
		scrollArea->setMinimumWidth( min_width + scrollArea->verticalScrollBar()->sizeHint().width());
	else
		scrollArea->setMinimumWidth( min_width );
	m_buttonFrame->adjustSize();
	scrollAreaWidgetContents->adjustSize();
}

void SelectGenomeWidget::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
		for (int i = 0 ; i < genome->size() ; i++)
			r_thread->render(r_requests[i]);
}

void SelectGenomeWidget::closeEvent(QCloseEvent* e)
{
	QSettings().setValue("selectgenomewidget/labelsize", label_size);
	e->accept();
}

void SelectGenomeWidget::wheelEvent(QWheelEvent* e)
{
	if (e->modifiers() & Qt::ControlModifier)
	{
		int dx( 8 * ( e->delta() > 0 ? 1 : -1 ) );
		if (e->modifiers() & Qt::ShiftModifier)
		{
			if (label_size.width() < 9 && dx < 0)
				return;
			label_size += QSize(dx, 0);
		}
		else
		{
			if (label_size.height() < 9 && dx < 0)
				return;
			label_size += QSize(dx, dx);
		}

		logInfo(QString("SelectGenomeWidget::wheelEvent : label size %1, %2")
		.arg(label_size.width()).arg(label_size.height()));

		foreach (RenderRequest* r, r_requests)
			r->setSize(label_size);
		foreach (PreviewWidget* l, labels)
		{
			l->resize(label_size);
			l->setMinimumSize(label_size);
			l->setMaximumSize(label_size);
		}
		reset();
		e->accept();
	}
}

void SelectGenomeWidget::addButtonPressedSlot()
{
	flam3_genome* current = genome->selectedGenome();
	flam3_genome gen;
	Util::init_genome(&gen);
	flam3_apply_template(&gen, current);
	// preserve values not copied in flam3_apply_template()
	gen.pixels_per_unit = current->pixels_per_unit;
	gen.contrast   = current->contrast;
	gen.gamma      = current->gamma;
	gen.brightness = current->brightness;
	gen.vibrancy   = current->vibrancy;
	genome->append(gen);
	setSelectedGenome(genome->size() - 1);
	emit genomesModified();
}


void SelectGenomeWidget::delButtonPressedSlot()
{
	int idx = genome->selectedIndex();
	genome->remove(idx);
	labels[idx]->clear();
	labels[idx]->hide();
	last_idx = 0;
	setSelectedGenome(qMax(0, idx - 1));
	emit genomesModified();
}
