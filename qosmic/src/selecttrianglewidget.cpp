/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
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
#include "selecttrianglewidget.h"

#define genome_ptr (genome->selectedGenome())

SelectTriangleWidget::SelectTriangleWidget(GenomeVector* g, QWidget* parent)
	: QWidget(parent), QosmicWidget(this, "SelectTriangleWidget") ,genome(g)
{
	setupUi(this);

	// restore (Double/Int)ValueEditor settings
	m_densLineEdit->restoreSettings();
	m_densLineEdit->setWheelEventUpdate(true);

	connect(m_selectedTriangleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(triangleSelectedSlot(int)));
	connect(m_densLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_densLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_addTriangleButton, SIGNAL(pressed()), this, SLOT(addTriangleAction()));
	connect(m_delTriangleButton, SIGNAL(pressed()), this, SLOT(delTriangleAction()));
	connect(m_finalButton, SIGNAL(clicked(bool)), this, SLOT(finalStateChangedSlot(bool)));
}


SelectTriangleWidget::~SelectTriangleWidget()
{
}

// this one is supposed is be called externally
void SelectTriangleWidget::triangleSelectedSlot(Triangle* t)
{
	selectedTriangle = t;
	triangleScene = dynamic_cast<FigureEditor*>(selectedTriangle->scene());
	m_selectedTriangleBox->blockSignals(true);
	int num_triangles = triangleScene->getNumberOfTriangles();
	int count = m_selectedTriangleBox->count();
	m_selectedTriangleBox->setMaxCount(num_triangles);
	while (count < num_triangles)
		m_selectedTriangleBox->addItem(QString::number(count++ + 1));
	m_selectedTriangleBox->setCurrentIndex(triangleScene->selectedTriangleIndex());
	m_selectedTriangleBox->blockSignals(false);
	reset();
}

// and this one is connected to the doublevalueeditor
void SelectTriangleWidget::triangleSelectedSlot(int /*idx*/)
{
	triangleScene->selectTriangle(m_selectedTriangleBox->currentIndex());
}


void SelectTriangleWidget::reset()
{
	m_densLineEdit->updateValue(selectedTriangle->xform()->density);
	m_densLineEdit->setEnabled(selectedTriangle->index() != genome_ptr->final_xform_index);
	m_finalButton->setChecked(genome_ptr->final_xform_enable);
}


void SelectTriangleWidget::fieldEditedAction()
{
	selectedTriangle->xform()->density = m_densLineEdit->value();
	emit dataChanged();
}


void SelectTriangleWidget::addTriangleAction()
{
	triangleScene->addTriangleAction();
}

void SelectTriangleWidget::delTriangleAction()
{
	triangleScene->removeTriangleAction();
}

void SelectTriangleWidget::finalStateChangedSlot(bool checked)
{
	bool hasFinal = genome_ptr->final_xform_enable;
	bool finalSelected = (genome_ptr->final_xform_index == triangleScene->selectedTriangleIndex());
	if (checked)
		triangleScene->enableFinalXform(true);
	else
	{
		if (hasFinal)
		{
			if (!finalSelected)
			{
				triangleScene->selectTriangle(genome_ptr->final_xform_index);
				m_finalButton->setChecked(true);
			}
			else
			{
				triangleScene->enableFinalXform(false);
				m_finalButton->setChecked(false);
			}
		}
		else
		{
			triangleScene->enableFinalXform(true);
			m_finalButton->setChecked(true);
		}
	}
}
