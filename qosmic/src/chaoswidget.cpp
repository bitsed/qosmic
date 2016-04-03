/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
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
#include <QSettings>
#include <QLineEdit>

#include "chaoswidget.h"
#include "flam3util.h"

ChaosWidget::ChaosWidget(GenomeVector* g, QWidget* parent)
	: QWidget(parent), genomes(g)
{
	setupUi(this);

	model = new QStandardItemModel(1, 1);

	m_chaosTable->setModel(model);
	m_chaosTable->restoreSettings();
	m_chaosTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	connect(m_chaosTable, SIGNAL(valueUpdated(int)), this, SLOT(chaosEditedSlot(int)));
	connect(m_chaosTable, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_chaosTable, SIGNAL(precisionChanged()), this, SLOT(updateFormData()));
}

ChaosWidget::~ChaosWidget()
{
	delete model;
}

void ChaosWidget::updateFormData()
{
	flam3_genome* genome = genomes->selectedGenome();
	if (genome->final_xform_enable == 1
		&& genome->final_xform_index == selectedTriangle->index())
		// Disallow editing chaos values for the final xform since they don't exist.
		m_chaosTable->setEnabled(false);
	else
	{
		QLocale l;
		int precision = m_chaosTable->precision();
		int xform_idx = selectedTriangle->index();
		int rows = genome->num_xforms;
		if (genome->final_xform_enable == 1)
			// The chaos array for an xform_idx holds a positive multiplier for
			// each non-final xform.
			rows -= 1;
		model->setRowCount(rows);
		for (int n = 0; n < rows ; n++)
		{
			QModelIndex idx = model->index(n, 0);
			double value = genome->chaos[xform_idx][n];
			model->setData(idx, l.toString(value, 'f', precision));
		}
		m_chaosTable->setEnabled(true);
	}
}

void ChaosWidget::chaosEditedSlot(int row)
{
	int xform_idx = selectedTriangle->index();
	QVariant data = model->data(model->index(row, 0));
	flam3_genome* genome = genomes->selectedGenome();
	genome->chaos[xform_idx][row] = data.toDouble();
	emit dataChanged();
}


void ChaosWidget::triangleSelectedSlot(Triangle* t)
{
	selectedTriangle = t;
	updateFormData();
}

/**
 * For some reason the wheelEvent occuring over the table headers is handled
 * here.  Throwing it down to the handler for the tablewidget causes an
 * infinite loop.
 */
void ChaosWidget::wheelEvent(QWheelEvent* e)
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

	if (m_chaosTable->isVisible())
		m_chaosTable->verticalScrollBar()->triggerAction(a);

	e->accept();
}


ChaosTableView::ChaosTableView(QWidget* parent)
: QTableView(parent)
{
	step = 0.1;
	start_value = 0.0;
	vars_precision = 4;
	setEditTriggers(QAbstractItemView::DoubleClicked);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectItems);
	setAutoScroll(false);
	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setSectionsMovable(false);
}

void ChaosTableView::restoreSettings()
{
	QSettings s;
	step = s.value(QString("chaostableview/%1/step")
	.arg(objectName()), step).toDouble();

	vars_precision = s.value(QString("chaostableview/%1/precision")
	.arg(objectName()), vars_precision).toInt();
}

void ChaosTableView::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key_BracketLeft:
			step *= 10.0;
			QSettings().setValue(QString("chaostableview/%1/step").arg(objectName()), step);
			break;

		case Qt::Key_BracketRight:
			step *= 0.10;
			QSettings().setValue(QString("chaostableview/%1/step").arg(objectName()), step);
			break;

		case Qt::Key_Return:
			emit valueUpdated(0);
			break;

		case Qt::Key_Plus:
			if (e->modifiers() & Qt::ControlModifier)
			{
				setPrecision(precision() + 1);
				QSettings().setValue(QString("chaostableview/%1/precision").arg(objectName()), precision());
			}
			break;

		case Qt::Key_Minus:
			if (e->modifiers() & Qt::ControlModifier)
			{
				setPrecision(precision() - 1);
				QSettings().setValue(QString("chaostableview/%1/precision").arg(objectName()), precision());
			}
			break;

		default:
			QTableView::keyPressEvent(e);
	}
}

void ChaosTableView::mousePressEvent(QMouseEvent* e)
{
	switch (e->button())
	{
		case Qt::LeftButton:
		{
			QModelIndex idx( indexAt(e->pos()) );
			if (idx.column() == 0)
			{
				start_item = idx;
				last_pos = e->localPos();
				start_value = start_item.data().toDouble();
				e->accept();
			}
			else
				start_item = QModelIndex();
			break;
		}
		case Qt::MidButton:
		{
			QModelIndex idx( indexAt(e->pos()) );
			if (idx.column() == 0)
			{
				model()->setData(idx, QLocale().toString(1.0, 'f', vars_precision));
				emit valueUpdated(idx.row());
			}
			break;
		}
		default:
			;
	}
	QTableView::mousePressEvent(e);
}

void ChaosTableView::mouseMoveEvent(QMouseEvent* e)
{
	if ((e->buttons() & Qt::LeftButton) && start_item.isValid())
	{
		double item_data = start_item.data().toDouble();
		double nstep = step;
		if (e->modifiers() & Qt::ShiftModifier)
			nstep *= 0.10;
		else if (e->modifiers() & Qt::ControlModifier)
			nstep *= 10.0;

		double dy = e->y() - last_pos.y();
		last_pos = e->localPos();
		if (dy == 0.0) return;
		if (dy > 0)
			nstep *= -1.0;

		double inc_value = qMax(0.0, item_data + nstep);
		model()->setData(start_item, QLocale().toString(inc_value, 'f', vars_precision));

		emit valueUpdated(start_item.row());

		e->accept();
	}
}

void ChaosTableView::mouseReleaseEvent(QMouseEvent* e)
{
	if ((e->button() == Qt::LeftButton)
		&& start_item.isValid()
		&& start_value != start_item.data().toDouble())
	{
		start_item = QModelIndex();
		e->accept();
		emit undoStateSignal();
	}
}

int ChaosTableView::precision()
{
	return vars_precision;
}

void ChaosTableView::setPrecision(int n)
{
	if (vars_precision != n)
	{
		vars_precision = qMax(1, n);
		emit precisionChanged();
	}
}

void ChaosTableView::commitData(QWidget* editor)
{
	QModelIndex idx(currentIndex());
	bool ok;
	double current_value(idx.data().toDouble());
	double editor_value(qMax(0.0, qobject_cast<QLineEdit*>(editor)->text().toDouble(&ok)));
	if (ok && current_value != editor_value)
	{
		model()->setData(idx, QLocale().toString(editor_value, 'f', vars_precision));
		emit valueUpdated(idx.row());
	}
}
