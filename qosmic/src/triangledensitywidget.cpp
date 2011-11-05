/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2011 by David Bitseff                 *
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
#include <QVBoxLayout>

#include "triangledensitywidget.h"
#include "selecttrianglewidget.h"
#include "logger.h"

#define genome_ptr (genome->selectedGenome())

TriangleDensityWidget::TriangleDensityWidget(GenomeVector* g, FigureEditor* e, QWidget* parent)
	: QWidget(parent), QosmicWidget(this, "TriangleDensityWidget"),
	triangleScene(e), genome(g), grouped_slider_idx(0),
	other_genome(), merged_genome(),
	merged_grouped_slider_idx(), is_merged(false)
{
	setupUi(this);

	Util::init_genome(&merged_genome);
	Util::init_genome(&other_genome);

	QVBoxLayout* vbox = new QVBoxLayout();
	m_scrollAreaWidgetContents->setLayout(vbox);
	vbox = new QVBoxLayout();
	m_scrollAreaWidgetContents_2->setLayout(vbox);
	m_mergeToolButton->setEnabled(false);

	QButtonGroup* bg = new QButtonGroup();
	bg->setExclusive(false);
	bg->addButton(m_zeroToolButton);
	bg->addButton(m_equalToolButton);
	bg->addButton(m_randToolButton);
	connect(bg, SIGNAL(buttonClicked(QAbstractButton*)),
		this, SLOT(distToolButtonSlot(QAbstractButton*)));

	bg = new QButtonGroup();
	bg->setExclusive(false);
	bg->addButton(m_zeroToolButton_2);
	bg->addButton(m_equalToolButton_2);
	bg->addButton(m_randToolButton_2);
	connect(bg, SIGNAL(buttonClicked(QAbstractButton*)),
		this, SLOT(distToolButtonSlot_2(QAbstractButton*)));

	connect(m_crossComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(mergeComboBoxChangedAction(int)));
	connect(m_mergeToolButton, SIGNAL(clicked(bool)),
		this, SLOT(mergeWithOtherGenomeAction()));
}


TriangleDensityWidget::~TriangleDensityWidget()
{
}


void TriangleDensityWidget::triangleSelectedSlot(Triangle* t)
{
	selectedTriangle = t;
	reset();
}

// each slider calls this method when being modified before emitting, and
// only one slider is modified at a time (by the user), so
// we know which slider changed (relative to group).
void TriangleDensityWidget::sliderGroupChanged(int idx)
{
	grouped_slider_idx = idx;
}

// groupedSliders are in the upper set of sliders
void TriangleDensityWidget::groupedSliderChangedSlot(int value)
{
	TriangleList triangles = triangleScene->triangles();
	double norm = getNorm();
	triangles.at(grouped_slider_idx)->xform()->density
		= (double)value / 1000. * norm;
	resetSliders(norm);
	emit dataChanged();
}

void TriangleDensityWidget::groupedSliderPressedSlot()
{
	if (grouped_slider_idx != triangleScene->selectedTriangleIndex())
		triangleScene->selectTriangle(grouped_slider_idx);
}

void TriangleDensityWidget::reset()
{
	TriangleList triangles = triangleScene->triangles();

	// make more sliders if necessary
	while (sliders.size() < triangles.size())
	{
		GroupedSlider* s
			= new GroupedSlider(Qt::Horizontal, this, sliders.size());
		QLabel* name = new QLabel(QString::number(sliders.size() + 1));
		QHBoxLayout* hl = new QHBoxLayout();
		hl->addWidget(name);
		hl->addWidget(s);
		dynamic_cast<QVBoxLayout*>
			(m_scrollAreaWidgetContents->layout())->insertLayout(-1,hl,0);
		s->setRange(0, 1000);
		s->setVisible(false);
		sliders.append(s);
		slider_names.append(name);
		connect(s, SIGNAL(valueChanged(int)), this, SLOT(groupedSliderChangedSlot(int)));
		connect(s, SIGNAL(sliderPressed()), this, SLOT(groupedSliderPressedSlot()));
		connect(s, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	}
	// hide non-used sliders
	for (int n = triangles.size() ; n < sliders.size() ; n++)
	{
		sliders.at(n)->setVisible(false);
		slider_names.at(n)->setVisible(false);
	}
	resetSliders(getNorm());

	QStringList items;
	items << "None";
	for (int n = 1 ; n <= genome->size() ; n++)
		items << QString::number(n);
	int n = m_crossComboBox->currentIndex();
	m_crossComboBox->blockSignals(true);
	m_crossComboBox->clear();
	m_crossComboBox->addItems(items);
	m_crossComboBox->setCurrentIndex(n);
	m_crossComboBox->blockSignals(false);
}

// scale and adjust the upper slider group
void TriangleDensityWidget::resetSliders(double norm)
{
	TriangleList triangles = triangleScene->triangles();
	int n = 0;
	foreach (Triangle* t, triangles)
	{
		QSlider* sl = sliders[n];
		QLabel* la = slider_names[n];
		sl->blockSignals(true);
		sl->setSliderPosition((int)( t->xform()->density / norm * 1000. ));
		sl->blockSignals(false);
		sl->setVisible(true);
		la->setVisible(true);
		sl->setToolTip(triangleScene->getInfoLabel(t));
		if (genome_ptr->final_xform_enable
			&& genome_ptr->final_xform_index == n)
		{
			sl->setEnabled(false);
			la->setEnabled(false);
		}
		else
		{
			sl->setEnabled(true);
			la->setEnabled(true);
		}
		n++;
	}

	if (is_merged)
	{
		for (int n = 0 ; n < other_genome.num_xforms ; n++)
		{
			QSlider* sl = merged_sliders[n];
			QLabel* la = merged_slider_names[n];
			sl->blockSignals(true);
			sl->setSliderPosition((int)(other_genome.xform[n].density
					/ norm * 1000. ));
			sl->blockSignals(false);
			sl->setVisible(true);
			la->setVisible(true);
			sl->setToolTip(QString("density: %1")
				.arg(other_genome.xform[n].density));
			if (other_genome.final_xform_enable
				&& other_genome.final_xform_index == n)
			{
				other_genome.xform[n].density = 0.;
				sl->setEnabled(false);
				la->setEnabled(false);
			}
			else
			{
				sl->setEnabled(true);
				la->setEnabled(true);
			}
		}
	}

	// force the splitter to resize
	QList<int> sizes;
	sizes << m_scrollAreaWidgetContents->sizeHint().height()
		<< m_scrollAreaWidgetContents_2->sizeHint().height();
	splitter->setSizes(sizes);
}

double TriangleDensityWidget::getNorm()
{
	double norm = 0;
	flam3_genome* g = genome_ptr;
	for (int n = 0 ; n < g->num_xforms ; n++)
		norm += g->xform[n].density;
	if (is_merged)
	{
		g = &other_genome;
		for (int n = 0 ; n < g->num_xforms ; n++)
			norm += g->xform[n].density;
	}
	return norm;
}

void TriangleDensityWidget::distToolButtonSlot(QAbstractButton* b)
{
	TriangleList triangles = triangleScene->triangles();
	if (b->objectName() == "m_zeroToolButton")
	{
		foreach (Triangle* t, triangles)
			t->xform()->density = 0.0000;
		triangles.first()->xform()->density = 0.01;
	}
	else if (b->objectName() == "m_equalToolButton")
	{
		double size = triangles.size();
		if (genome_ptr->final_xform_enable
			&& genome_ptr->final_xform_index < genome_ptr->num_xforms)
			size--;
		int n = 0;
		foreach (Triangle* t, triangles)
		{
			if (genome_ptr->final_xform_index == n++
				&& genome_ptr->final_xform_enable)
				continue;
			else
				t->xform()->density = 1./size;
		}
	}
	else if (b->objectName() == "m_randToolButton")
	{
		int n = 0;
		foreach (Triangle* t, triangles)
		{
			if (genome_ptr->final_xform_index == n++
				&& genome_ptr->final_xform_enable)
				continue;
			else
				t->xform()->density = flam3_random01()*flam3_random01()*10.;
		}
	}
	resetSliders(getNorm());
	emit dataChanged();
	emit undoStateSignal();
}

void TriangleDensityWidget::distToolButtonSlot_2(QAbstractButton* b)
{
	if (is_merged)
	{
		if (b->objectName() == "m_zeroToolButton_2")
		{
			for (int n = 0 ; n < other_genome.num_xforms ; n++)
				other_genome.xform[n].density = 0.0000;
			other_genome.xform[0].density = 0.01;
		}
		else if (b->objectName() == "m_equalToolButton_2")
		{
			double size = other_genome.num_xforms;
			if (other_genome.final_xform_enable
				&& other_genome.final_xform_index < other_genome.num_xforms)
				size--;
			for (int n = 0 ; n < other_genome.num_xforms ; n++)
				if (other_genome.final_xform_enable
				&& other_genome.final_xform_index == n)
					continue;
				else
					other_genome.xform[n].density = 1./size;
		}
		else if (b->objectName() == "m_randToolButton_2")
		{
			for (int n = 0 ; n < other_genome.num_xforms ; n++)
				if (other_genome.final_xform_enable
				&& other_genome.final_xform_index == n)
					continue;
				else
					other_genome.xform[n].density
						= flam3_random01()*flam3_random01()*10.;
		}
		resetSliders(getNorm());
		emit dataChanged();
	}
}

void TriangleDensityWidget::mergeComboBoxChangedAction(int idx)
{
	if (idx > 0)
	{
		flam3_genome* g = genome->data() + idx - 1;
		flam3_copy(&other_genome, g);

		logInfo(QString("TriangleDensityWidget::mergeWithGenomeAction : "
			"merging %1 with %2")
			.arg(genome->selected()).arg(idx - 1));

		// make more sliders if necessary
		while (merged_sliders.size() < other_genome.num_xforms)
		{
			GroupedSlider* s
				= new GroupedSlider(Qt::Horizontal, this, merged_sliders.size());
			QLabel* name = new QLabel(QString::number(merged_sliders.size() + 1));
			QHBoxLayout* hl = new QHBoxLayout();
			hl->addWidget(name);
			hl->addWidget(s);
			dynamic_cast<QVBoxLayout*>
				(m_scrollAreaWidgetContents_2->layout())->insertLayout(-1,hl,0);
			s->setRange(0, 1000);
			s->setVisible(false);
			merged_sliders.append(s);
			merged_slider_names.append(name);
			connect(s, SIGNAL(valueChanged(int)),
					this, SLOT(groupedMergedSliderChangedSlot(int)));
		}
		// hide non-used sliders
		for (int n = other_genome.num_xforms ; n < merged_sliders.size() ; n++)
		{
			merged_sliders.at(n)->setVisible(false);
			merged_slider_names.at(n)->setVisible(false);
		}
		is_merged = true;
		resetSliders(getNorm());
		m_mergeToolButton->setEnabled(true);
	}
	else
	{
		for (int n = 0 ; n < other_genome.num_xforms ; n++)
		{
			merged_sliders[n]->setVisible(false);
			merged_slider_names[n]->setVisible(false);
		}
		m_mergeToolButton->setEnabled(false);

		is_merged = false;
		resetSliders(getNorm());
	}
	emit dataChanged();
}

void TriangleDensityWidget::groupedMergedSliderChangedSlot(int value)
{
	double merged_norm = getNorm();
	other_genome.xform[grouped_slider_idx].density
		= (double)value / 1000. * merged_norm;
	resetSliders(merged_norm);
	emit dataChanged();
}

flam3_genome* TriangleDensityWidget::getMergedGenome()
{
	flam3_copy(&merged_genome, genome_ptr);
	int idxn = merged_genome.num_xforms;
	// count the number of xform having density > 0
	int num_xforms = 0;
	for (int n = 0 ; n < other_genome.num_xforms ; n++)
		if (other_genome.xform[n].density != 0.
			|| (!merged_genome.final_xform_enable
				&& other_genome.final_xform_enable
				&& other_genome.final_xform_index == n))
				num_xforms++;
	flam3_add_xforms(&merged_genome, num_xforms, 0, 0);

	for (int n = 0 ; n < other_genome.num_xforms ; n++)
		if (other_genome.xform[n].density != 0.)
			flam3_copy_xform(merged_genome.xform + idxn++, other_genome.xform + n);
		else if (!merged_genome.final_xform_enable
				&& other_genome.final_xform_enable
				&& other_genome.final_xform_index == n)
		{
			// add the other_genome final xform if there isn't one set already
			logInfo(QString("TriangleDensityWidget::getMergedGenome : "
				"using other_genome.final_xform_index at %1").arg(idxn));
			flam3_copy_xform(merged_genome.xform + idxn++, other_genome.xform + n);
			merged_genome.final_xform_enable = 1;
			merged_genome.final_xform_index = idxn;
			merged_genome.xform[idxn].density = 0.;
		}

	return &merged_genome;
}

bool TriangleDensityWidget::hasMergedGenome()
{
	return is_merged;
}

void TriangleDensityWidget::mergeWithOtherGenomeAction()
{
	if (is_merged)
	{
		flam3_copy(genome_ptr, getMergedGenome());
		m_crossComboBox->blockSignals(true);
		m_crossComboBox->setCurrentIndex(0);
		m_crossComboBox->blockSignals(false);
		is_merged = false;
		blockSignals(true);
		mergeComboBoxChangedAction(0);
		blockSignals(false);
		emit genomeMerged();
		emit undoStateSignal();
	}
}

