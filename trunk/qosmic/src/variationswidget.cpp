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
#include <QStringList>
#include <QScrollBar>

#include "variationswidget.h"
#include "flam3util.h"
#include "logger.h"

VariationsWidget::VariationsWidget(GenomeVector* gen, QWidget* parent)
	: QWidget(parent), genome(gen)
{
	setupUi(this);

	model = new VarsTableModel();
	m_variationsTable->setModel(model);
	m_variationsTable->restoreSettings();
	m_variationsTable->header()->resizeSections(QHeaderView::ResizeToContents);
	m_variationValueEditor->restoreSettings();
	lastVariation = "";

	connect(m_variationsTable, SIGNAL(valueUpdated(int)), this, SLOT(variationEditedSlot(int)));
	connect(m_variationsTable, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_variationsTable, SIGNAL(precisionChanged()), this, SLOT(updateFormData()));
	connect(m_variationSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(variationSelectedSlot(int)));
	connect(m_variationValueEditor, SIGNAL(valueUpdated()), this, SLOT(valueEditorUpdatedSlot()));
	connect(m_applyButton, SIGNAL(clicked(bool)), this, SLOT(addVariationValueSlot()));
}

VariationsWidget::~VariationsWidget()
{
	delete model;
}

// connected to the '+' toolbutton above the treeview
void VariationsWidget::addVariationValueSlot()
{
	m_variationValueEditor->updateValue(0.0);
	updateFormData();
	emit undoStateSignal();
}

// connected to the doublevalueeditor above the treeview
void VariationsWidget::valueEditorUpdatedSlot()
{
	flam3_xform* xform = selectedTriangle->xform();
	double* var = xform->var;
	var[Util::variation_number(lastVariation)] = m_variationValueEditor->value();
	model->setModelData(xform);
	emit dataChanged();
}

// connected to the combobox above the treeview
void VariationsWidget::variationSelectedSlot(int /*idx*/)
{
	flam3_xform* xform = selectedTriangle->xform();
	double* var = xform->var;
	QString rowName = m_variationSelector->currentText();
	if (lastVariation == rowName)
		return;

	int row = Util::variation_number(rowName);
	double thisvalue = m_variationValueEditor->value();
	if (thisvalue == 0.0)
		lastVariation = rowName;
	else
	{
		if (!lastVariation.isEmpty())
			var[Util::variation_number(lastVariation)] = 0.0;
		var[row] = thisvalue;
		lastVariation = rowName;
		model->setModelData(xform);
		emit dataChanged();
	}
}

void VariationsWidget::resetVariationSelector()
{
	double* var = selectedTriangle->xform()->var;
	m_variationSelector->blockSignals(true);
	QString current = m_variationSelector->currentText();
	m_variationSelector->clear();
	for (int n = 0 ; n < flam3_nvariations ; n++)
	{
		double value = var[n];
		QString key = flam3_variation_names[n];
		if (value == 0.0)
			m_variationSelector->addItem(key);
	}
	m_variationSelector->blockSignals(false);
	lastVariation = m_variationSelector->currentText();
	m_variationValueEditor->updateValue(0.0);
}

void VariationsWidget::updateFormData()
{
	logFiner("VariationsWidget::updateFormData : setting variations");
	flam3_xform* xform = selectedTriangle->xform();
	model->setPrecision( m_variationsTable->precision() );
	model->setModelData(xform);
	resetVariationSelector();
}

void VariationsWidget::variationEditedSlot(int /*row*/)
{
	resetVariationSelector();
	emit dataChanged();
}

void VariationsWidget::triangleSelectedSlot(Triangle* t)
{
	selectedTriangle = t;
	updateFormData();
}

/**
 * For some reason the wheelEvent occuring over the table headers is handled
 * here.  Throwing it down to the handler for the tablewidget causes an
 * infinite loop.
 */
void VariationsWidget::wheelEvent(QWheelEvent* e)
{
	QAbstractSlider::SliderAction a = QAbstractSlider::SliderSingleStepSub;
	if (e->delta() < 0)
	{
		if (e->modifiers() & Qt::ControlModifier)
			a = QAbstractSlider::SliderPageStepAdd;
		else
			a = QAbstractSlider::SliderSingleStepAdd;
	}
	else
		if (e->modifiers() & Qt::ControlModifier)
			a = QAbstractSlider::SliderPageStepSub;

	if (m_variationsTable->isVisible())
		m_variationsTable->verticalScrollBar()->triggerAction(a);

	e->accept();
}
