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
#include "selecttrianglewidget.h"

#define genome_ptr (genome->selectedGenome())

SelectTriangleWidget::SelectTriangleWidget(GenomeVector* g, QWidget* parent)
	: QWidget(parent), QosmicWidget(this, "SelectTriangleWidget") ,genome(g)
{
	setupUi(this);

	// restore (Double/Int)ValueEditor settings
	m_densLineEdit->restoreSettings();
	m_densLineEdit->setWheelEventUpdate(true);

	m_selectedTriangleEditor->setMinimum(1);

	connect(m_selectedTriangleEditor, SIGNAL(valueChanged(int)), this, SLOT(triangleSelectedSlot(int)));
	connect(m_densLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_densLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_addTriangleButton, SIGNAL(pressed()), this, SLOT(addTriangleAction()));
	connect(m_delTriangleButton, SIGNAL(pressed()), this, SLOT(delTriangleAction()));
	connect(m_finalButton, SIGNAL(clicked(bool)), this, SLOT(finalStateChangedSlot(bool)));
	connect(m_animateButton, SIGNAL(clicked(bool)), this, SLOT(animateStateChangedSlot(bool)));
}


SelectTriangleWidget::~SelectTriangleWidget()
{
}

// this one is supposed is be called externally
void SelectTriangleWidget::triangleSelectedSlot(Triangle* t)
{
	selectedTriangle = t;
	triangleScene = dynamic_cast<FigureEditor*>(selectedTriangle->scene());
	m_selectedTriangleEditor->blockSignals(true);
	m_selectedTriangleEditor->setMaximum(triangleScene->getNumberOfTriangles());
	m_selectedTriangleEditor->setValue(triangleScene->selectedTriangleIndex() + 1);
	m_selectedTriangleEditor->blockSignals(false);
	reset();
}

// and this one is connected to the doublevalueeditor
void SelectTriangleWidget::triangleSelectedSlot(int /*idx*/)
{
	triangleScene->selectTriangle(m_selectedTriangleEditor->value() - 1);
}


void SelectTriangleWidget::reset()
{
	m_densLineEdit->updateValue(selectedTriangle->xform()->density);
	m_finalButton->setChecked(genome_ptr->final_xform_enable);
	m_animateButton->setChecked(selectedTriangle->xform()->animate == 0.0);
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

void SelectTriangleWidget::animateStateChangedSlot(bool checked)
{
	selectedTriangle->xform()->animate = !checked;
}

