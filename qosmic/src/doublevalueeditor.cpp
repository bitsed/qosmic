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
#include <QSettings>

#include "doublevalueeditor.h"
#include "logger.h"

DoubleValueEditor::DoubleValueEditor(QWidget* parent)
	: QDoubleSpinBox(parent), wheelEventSignal(true), last_press(0)
{
	setKeyboardTracking(false);
	connect(this, SIGNAL(valueChanged(double)), this, SIGNAL(valueUpdated()));
}

void DoubleValueEditor::restoreSettings()
{
	QSettings settings;

	if (objectName().isEmpty())
		logWarn("DoubleValueEditor::restoreSettings : nameless object found");

	setSingleStep(settings.value(
		QString("doublevalueeditor/%1/singlestep").arg(objectName()), singleStep()).toDouble());
	setDecimals(settings.value(
		QString("doublevalueeditor/%1/decimals").arg(objectName()), decimals()).toInt());

	default_step = singleStep();
}

void DoubleValueEditor::updateValue(double v)
{
	if (value() == v)
		return;
	blockSignals(true);
	QDoubleSpinBox::setValue(v);
	blockSignals(false);
}


void DoubleValueEditor::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key_BracketLeft:
			setSingleStep(singleStep() * 10.0);
			QSettings().setValue(QString("doublevalueeditor/%1/singlestep").arg(objectName()), singleStep());
			break;

		case Qt::Key_BracketRight:
			setSingleStep(singleStep() * 0.10);
			QSettings().setValue(QString("doublevalueeditor/%1/singlestep").arg(objectName()), singleStep());
			break;

		case Qt::Key_Return:
			QDoubleSpinBox::keyPressEvent(e);
			emit undoStateSignal();
			break;

		case Qt::Key_Backslash:
			WheelValueEditor::WHEEL_EVENTS_ENABLED
					= ! WheelValueEditor::WHEEL_EVENTS_ENABLED;
			break;

		case Qt::Key_Plus:
			if (e->modifiers() & Qt::ControlModifier)
			{
				setDecimals(decimals() + 1);
				QSettings().setValue(QString("doublevalueeditor/%1/decimals").arg(objectName()), decimals());
			}
			break;

		case Qt::Key_Minus:
			if (e->modifiers() & Qt::ControlModifier)
			{
				setDecimals(qMax(1, decimals() - 1));
				QSettings().setValue(QString("doublevalueeditor/%1/decimals").arg(objectName()), decimals());
			}
			break;

		default:
			QDoubleSpinBox::keyPressEvent(e);
	}
}

void DoubleValueEditor::wheelEvent(QWheelEvent* e)
{
	if (!QDoubleSpinBox::isActiveWindow())
		QDoubleSpinBox::activateWindow();
	QDoubleSpinBox::setFocus(Qt::MouseFocusReason);

	double step = default_step = singleStep();
	if (e->modifiers() & Qt::ShiftModifier)
		setSingleStep(step /= 10.0);
	else if (e->modifiers() & Qt::ControlModifier)
		setSingleStep(step *= 10.0);

	if (e->delta() > 0)
		stepUp();
	else
		stepDown();

	if (step != default_step)
		setSingleStep(default_step);

	if (wheelEventSignal && WheelValueEditor::WHEEL_EVENTS_ENABLED)
		emit valueUpdated();
}

void DoubleValueEditor::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		start_pos = last_pos = e->posF();
		start_value = value();
		last_press = e;
	}
}


void DoubleValueEditor::mouseMoveEvent(QMouseEvent* e)
{
	if (e->buttons() & Qt::LeftButton)
	{
		double dy = e->y() - last_pos.y();
		double step = default_step = singleStep();
		last_pos = e->posF();
		last_press = 0;

		if (e->modifiers() & Qt::ShiftModifier)
			setSingleStep(step /= 10.0);
		else if (e->modifiers() & Qt::ControlModifier)
			setSingleStep(step *= 10.0);

		if (dy < 0)
		{
			stepUp();
			emit valueUpdated();
		}
		else if (dy > 0)
		{
			stepDown();
			emit valueUpdated();
		}

		if (step != default_step)
			setSingleStep(default_step);
	}
}

void DoubleValueEditor::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		if (last_press && start_pos == e->posF())
		{
			QDoubleSpinBox::mousePressEvent(last_press);
			QDoubleSpinBox::mouseReleaseEvent(e);
			last_press = 0;
		}

		if (start_value != value())
			emit undoStateSignal();
	}
}


void DoubleValueEditor::setWheelEventUpdate(bool value)
{
	wheelEventSignal = value;
}

// redefine these since the qdoublespinbox versions also activate the selection
void DoubleValueEditor::stepUp()
{
	updateValue(value() + singleStep());
}

void DoubleValueEditor::stepDown()
{
	updateValue(value() - singleStep());
}

