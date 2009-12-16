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
#include <QStringList>
#include <QScrollBar>

#include "variationswidget.h"
#include "flam3util.h"
#include "logger.h"

VariationsWidget::VariationsWidget(GenomeVector* gen, QWidget* parent)
	: QWidget(parent), genome(gen)
{
	setupUi(this);

	const QStringList vars(Util::flam3_variations().keys());
	model = new VarsTableModel();
	m_variationsTable->setModel(model);
	m_variationsTable->restoreSettings();
	m_variationsTable->header()->resizeSections(QHeaderView::ResizeToContents);

	connect(m_variationsTable, SIGNAL(valueUpdated(int)), this, SLOT(variationEditedSlot(int)));
	connect(m_variationsTable, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_variationsTable, SIGNAL(precisionChanged()), this, SLOT(updateFormData()));
}

VariationsWidget::~VariationsWidget()
{
	delete model;
}

void VariationsWidget::updateFormData()
{
	logFiner("VariationsWidget::updateFormData : setting variations");
	QLocale l;
	m_variationsTable->blockSignals(true);
	int variations_pre = m_variationsTable->precision();
	flam3_xform* xform = selectedTriangle->xform();
	double* var = xform->var;

	for (int row = 0; row < model->rowCount() ; row++)
	{
		QModelIndex nidx( model->index(row, 0) );
		QModelIndex vidx( model->index(row, 1) );
		double value(var[Util::variation_number(nidx.data().toString())]);
		if (value == 0.0)
			model->setData(vidx, 0);
		else
			model->setData(vidx, l.toString(value, 'f', variations_pre));

		if (model->hasChildren(nidx))
		{
			for (int row = 0; row < model->rowCount(nidx) ; row++)
			{
				QModelIndex cnidx( model->index(row, 0, nidx) );
				QModelIndex cvidx( model->index(row, 1, nidx) );
				// get the UserRole data for the variable name from the index
				// since the DisplayRole removes the variation part
				value = Util::get_xform_variable(xform, cnidx.data(Qt::UserRole).toString());
				if (value == 0.0)
					model->setData(cvidx, 0);
				else
					model->setData(cvidx, l.toString(value, 'f', variations_pre));
			}
		}
	}
	m_variationsTable->blockSignals(false);
	updateLabel();
}

void VariationsWidget::variationEditedSlot(int row)
{
	flam3_xform* xform = selectedTriangle->xform();
	double* var = xform->var;
	VarsTableItem* item = model->getVariation(row);
	QString name(item->data(0).toString());
	double value(item->data(1).toDouble());
	var[Util::variation_number(name)] = value;
	if (item->childCount() > 0)
	{
		for (int n = 0 ; n < item->childCount() ; n++)
		{
			VarsTableItem* child = item->child(n);
			name  = child->data(0).toString();
			value = child->data(1).toDouble();
			Util::set_xform_variable(xform, name, value);
		}
	}
	updateLabel();
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

void VariationsWidget::updateLabel()
{
	QStringList sl;
	QLocale l;
	int rows( model->rowCount() );
	for (int n = 0 ; n < rows ; n++)
	{
		VarsTableItem* item = model->getVariation(n);
		QString name(item->data(0).toString());
		double value(item->data(1).toDouble());
		if (value != 0.0)
			sl << QString("%1(%2)").arg(name).arg(l.toString(value, 'f', 2));
	}
	m_varListLabel->setWordWrap(false);
	m_varListLabel->setText(sl.join(" "));
}
