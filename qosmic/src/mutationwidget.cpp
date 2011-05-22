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


#include <QAction>

#include "mutationwidget.h"
#include "logger.h"

MutationWidget::MutationWidget(GenomeVector* gen,  RenderThread* t,
							   QWidget* parent)
	: QWidget(parent), genome_offset(0), genome(gen), rthread(t), speed(0.1)
{
	setupUi(this);

	labels << label << label_2 << label_3  // labels are defined in the .ui file
			<< label_4 << label_5 << label_6
			<< label_7 << label_8 << label_9;

	varsMenu = new QMenu(this);
	QMap<QString, int>map = Util::flam3_variations();
	foreach(QString s, map.keys())
	{
		QAction* act = new QAction(s, this);
		act->setCheckable(true);
		varsActions << act;
		varsMenu->addAction(act);
	}
	clearMenuAct = new QAction(tr("clear selected"), this);
	clearMenuAct->setCheckable(false);
	varsMenu->addAction(clearMenuAct);

	for ( int n = 0 ;  n < 9 ; n++)
	{
		flam3_genome* g = new flam3_genome;
		memset(g, 0, sizeof(flam3_genome));
		clear_cp(g, flam3_defaults_off);
		mutations << g;
		labels[n]->setGenome(g);
		RenderRequest* r = new RenderRequest(genome->data(), QSize(80,80), "mutation",
											 RenderRequest::Queued);
		requests << r;
		connect(labels[n], SIGNAL(mutationSelected(PreviewWidget*)),
				this, SLOT(mutationSelectedAction(PreviewWidget*)));
		connect(labels[n], SIGNAL(genomeSelected(PreviewWidget*)),
				this, SLOT(genomeSelectedAction(PreviewWidget*)));
	}

	m_speedSlider->setValue(10);
	speedChangedAction(10);
	connect(m_speedSlider, SIGNAL(valueChanged(int)),
			this, SLOT(speedChangedAction(int)));
	connect(m_variationsButton, SIGNAL(released()),
			this, SLOT(selectVariationsAction()));

	connect(rthread, SIGNAL(flameRendered(RenderEvent*)),
			this, SLOT(flameRenderedAction(RenderEvent*)));

	connect(clearMenuAct, SIGNAL(triggered(bool)),
			this, SLOT(clearMenuAction()));
}

void MutationWidget::genomeSelectedAction(PreviewWidget* ptr)
{
	int idx = mutations.indexOf(ptr->genome(),0);
	logFine(QString("MutationWidget::genomeSelectedAction : genome %1 selected").arg(idx));
	if (idx >= 0)
		emit genomeSelected(mutations[idx]);
}

void MutationWidget::mutationSelectedAction(PreviewWidget* ptr)
{
	if (ptr)
	{
		int idx = mutations.indexOf(ptr->genome(),0);
		logFine(QString("MutationWidget::mutationSelectedAction : mutating %1")
				.arg(idx));
		if (idx == -1)
			reset();
		else
		{
			if (idx > 0)
			{
				mutations.swap(idx, 0);
				labels[0]->setGenome(ptr->genome());
			}
			mutate(0);
		}
	}
	else
		reset();
}

void MutationWidget::flameRenderedAction(RenderEvent* e)
{
	RenderRequest* req = e->request();
	if (!(req->type() == RenderRequest::Queued
			   && requests.contains(req)) )
		return;

	int idx = requests.indexOf(req, 0);
	logFine(QString("MutationWidget::flameRenderedAction : setting genome %1,g=0x%2,req=0x%3")
			.arg(idx).arg((long)req->genome(),0,16).arg((long)req,0,16));
	labels[idx]->setGenome(req->genome());
	labels[idx]->setPixmap(QPixmap::fromImage(req->image()));
	e->accept();
}

void MutationWidget::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
	{
		logFine(QString("MutationWidget::showEvent : resetting"));
		reset();
	}

}

#define genome_ptr (genome->selectedGenome())

void MutationWidget::mutate(int /*idx*/)
{
	flam3_genome tmp_genome;
	memset(&tmp_genome, 0, sizeof(flam3_genome));
	clear_cp(&tmp_genome, flam3_defaults_on);

	int variations[flam3_nvariations];
	int nvars = 0;
	foreach (QAction* a, varsActions)
		if (a->isChecked() && (a != clearMenuAct))
			variations[nvars++] = Util::variation_number(a->text());
	if (nvars == 0)
	{
		variations[0] = flam3_variation_random;
		nvars = 1;
	}

	int symmetry = 0;
	for ( int n = 0 ;  n < 9 ; n++)
	{
		flam3_genome* g = mutations.at(n);
		logFine(QString("MutationWidget::mutate : genome 0x%1 at %2")
				 .arg((long)g,0,16).arg(n));
		if (n > 0)
		{
			if (n % 3 == 0)
				flam3_copy(&tmp_genome, mutations.at(n - 3));
			else
				flam3_copy(&tmp_genome, mutations.at(n - 1));

			// flam3_mutate() calls add_to_action() which needs this size char[]
			char modstr[flam3_max_action_length] = "";
			flam3_mutate(&tmp_genome, -1, variations, nvars, symmetry, speed,
				Util::get_isaac_randctx(), modstr);
			if (tmp_genome.num_xforms > 0)
			{
				flam3_copy(g, &tmp_genome);
				logFine(QString("MutationWidget::mutate : modstr '%1'").arg(QString(modstr)));
			}
			else
				logWarn("MutationWidget::mutate : zero xforms in mutation");
		}

		RenderRequest* req = requests.at(n);
		req->setGenome(g);
		rthread->render(req);
	}
	clear_cp(&tmp_genome, flam3_defaults_on);
}

void MutationWidget::reset()
{
	if (genome_ptr->num_xforms > 0)
	{
		logFine(QString("MutationWidget::reset : genome has %1 xforms")
				.arg(genome_ptr->num_xforms));
		flam3_copy(mutations[0], genome_ptr);
		labels[0]->setGenome(mutations[0]);
		requests[0]->setGenome(mutations.at(0));
		if (isVisible())
			mutate(0);
	}
}

void MutationWidget::speedChangedAction( int )
{
	speed = m_speedSlider->value() / 100.;
	m_speedLabel->setText(QString::number(speed, 'f', 2));
}

void MutationWidget::selectVariationsAction( )
{
	varsMenu->popup(mapToGlobal(QPoint(0,0)));

}

void MutationWidget::clearMenuAction()
{
	foreach (QAction* act, varsActions)
		act->setChecked(false);
}
