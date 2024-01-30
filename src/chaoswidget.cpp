/***************************************************************************
 *   Copyright (C) 2007-2024 by David Bitseff                              *
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
	connect(m_chaosTable, SIGNAL(valueUpdated()), this, SIGNAL(dataChanged()));
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
			// Store the Display/Edit Role and a reference to the
			// xform.chaos[n] value in the model.
			double* valueP = &genome->chaos[xform_idx][n];
			QModelIndex idx = model->index(n, 0);
			model->setData(idx, QLocale().toString(*valueP, 'f', precision));
			model->setData(idx, QVariant::fromValue((void*)valueP), Qt::UserRole);
		}
		m_chaosTable->setEnabled(true);
	}
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
	if (e->angleDelta().y() < 0)
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

void ChaosTableView::resetChaosValue(QModelIndex& idx)
{
	*(double*)model()->data(idx, Qt::UserRole).value<void*>() = 1.0;
	model()->setData(idx, QLocale().toString(1.0, 'f', vars_precision));
	emit valueUpdated();
	emit undoStateSignal();
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
			break;

		case Qt::Key_Plus:
		case Qt::Key_Minus:
			if (e->modifiers() & Qt::ControlModifier)
			{
				setPrecision(precision() + (e->key() == Qt::Key_Plus ? 1 : -1));
				QSettings().setValue(QString("chaostableview/%1/precision").arg(objectName()), precision());
				break;
			}
			// fall through
		case Qt::Key_Comma:
		case Qt::Key_Period:
		case Qt::Key_0:
		case Qt::Key_1:
		case Qt::Key_2:
		case Qt::Key_3:
		case Qt::Key_4:
		case Qt::Key_5:
		case Qt::Key_6:
		case Qt::Key_7:
		case Qt::Key_8:
		case Qt::Key_9:
		case Qt::Key_Space:
			{
				QModelIndex idx(currentIndex());
				QModelIndex vidx(idx.sibling(idx.row(), 0));
				setCurrentIndex(vidx);
				if (edit(vidx, QAbstractItemView::AllEditTriggers, e))
				{   // Display variation or variable text editor
					QLineEdit* editor(qobject_cast<QLineEdit*>(indexWidget(vidx)));
					if (e->key() != Qt::Key_Space)
					{   // Clear the editor value and add typed numeral character
						editor->setText(e->text());
					}
				}
				break;
			}

		case Qt::Key_Delete:
		case Qt::Key_Backspace:
			{
				QModelIndex idx(currentIndex());
				resetChaosValue(idx);
				break;
			}

		case Qt::Key_Up:
		case Qt::Key_Down:
			if (e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier ))
			{
				double nstep = step;
				if (e->modifiers() & Qt::ShiftModifier)
					nstep *= 0.10;
				else if (e->modifiers() & Qt::ControlModifier)
					nstep *= 10.0;

				if (e->key() == Qt::Key_Down)
					nstep *= -1.0;

				QModelIndex idx(currentIndex());
				QModelIndex vidx(idx.sibling(idx.row(), 0));
				double* item_ptr = (double*)vidx.data(Qt::UserRole).value<void*>();
				double inc_value = *item_ptr + nstep;
				if (qFuzzyCompare(1 + inc_value, 1 + 0.0))
					inc_value = 0.0;
				*item_ptr = inc_value;
				model()->setData(vidx,
					QLocale().toString(inc_value, 'f', vars_precision));
				e->accept();
				emit valueUpdated();
				break;
			}
			// fall through

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
				start_value = *(double*)start_item.data(Qt::UserRole).value<void*>();
				e->accept();
			}
			else
				start_item = QModelIndex();
			break;
		}
		case Qt::MiddleButton:
		{
			QModelIndex idx( indexAt(e->pos()) );
			if (idx.column() == 0)
			{
				resetChaosValue(idx);
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

		double* item_ptr = (double*)start_item.data(Qt::UserRole).value<void*>();
		double inc_value = qMax(0.0, *item_ptr + nstep);
		*item_ptr = inc_value;
		model()->setData(start_item,
			QLocale().toString(inc_value, 'f', vars_precision));

		emit valueUpdated();

		e->accept();
	}
}

void ChaosTableView::mouseReleaseEvent(QMouseEvent* e)
{
	if ((e->button() == Qt::LeftButton)
		&& start_item.isValid()
		&& start_value != start_item.data(Qt::UserRole).toDouble())
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
	double* item_ptr((double*)idx.data(Qt::UserRole).value<void*>());
	double current_value(*item_ptr);
	double editor_value(qMax(0.0,
		QLocale().toDouble(qobject_cast<QLineEdit*>(editor)->text(), &ok)));
	if (ok && current_value != editor_value)
	{
		*item_ptr = editor_value;
		model()->setData(idx,
			QLocale().toString(editor_value, 'f', vars_precision));
		emit valueUpdated();
		emit undoStateSignal();
	}
}
