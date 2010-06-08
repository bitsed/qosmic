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
#include <QSettings>

#include "intvalueeditor.h"
#include "logger.h"

IntValueEditor::IntValueEditor(QWidget* parent)
		: QSpinBox(parent), wheelEventSignal(true)
{
}

void IntValueEditor::restoreSettings()
{
	if (objectName().isEmpty())
		logWarn("IntValueEditor::restoreSettings : nameless object found");

	setSingleStep(QSettings().value(
		QString("intvalueeditor/%1/singlestep").arg(objectName()), singleStep()).toInt());

	default_step = singleStep();
}

void IntValueEditor::updateValue(int v)
{
	if (value() == v)
		return;
	blockSignals(true);
	QSpinBox::setValue(v);
	blockSignals(false);
}


void IntValueEditor::wheelEvent(QWheelEvent* e)
{
	if (!QSpinBox::isActiveWindow())
		QSpinBox::activateWindow();
	QSpinBox::setFocus(Qt::MouseFocusReason);

	int step = default_step = singleStep();
	if (e->modifiers() & Qt::ShiftModifier)
	{
		if (step >= 10.0)
			setSingleStep(step = (int)(step / 10.0));
	}
	else if (e->modifiers() & Qt::ControlModifier)
		setSingleStep(step = step * 10);

	if (e->delta() > 0)
		stepUp();
	else
		stepDown();

	if (step != default_step)
		setSingleStep(default_step);

	if (wheelEventSignal && WheelValueEditor::WHEEL_EVENTS_ENABLED)
		emit valueUpdated();
}

bool IntValueEditor::wheelEventUpdate() const
{
	return wheelEventSignal;
}

void IntValueEditor::setWheelEventUpdate(bool value)
{
	wheelEventSignal = value;
}

void IntValueEditor::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key_BracketLeft:
			setSingleStep((int)(singleStep() * 10.0));
			QSettings().setValue(QString("intvalueeditor/%1/singlestep").arg(objectName()), singleStep());
			break;

		case Qt::Key_BracketRight:
			setSingleStep(qMax((int)(default_step * 0.10), 1));
			QSettings().setValue(QString("intvalueeditor/%1/singlestep").arg(objectName()), singleStep());
			break;

		case Qt::Key_Return:
			emit valueUpdated();
			break;

		case Qt::Key_Backslash:
			WheelValueEditor::WHEEL_EVENTS_ENABLED
			= ! WheelValueEditor::WHEEL_EVENTS_ENABLED;
			break;

		default:
			QSpinBox::keyPressEvent(e);
	}
}

void IntValueEditor::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		last_pos = e->posF();
		start_value = value();
	}
}

void IntValueEditor::mouseMoveEvent(QMouseEvent* e)
{
	if (e->buttons() & Qt::LeftButton)
	{
		double dy = e->y() - last_pos.y();
		last_pos = e->posF();
		double step = default_step = singleStep();

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

void IntValueEditor::mouseReleaseEvent(QMouseEvent* e)
{
	if ( (e->button() == Qt::LeftButton) && start_value != value())
		emit undoStateSignal();
}

// redefine these since the qintspinbox versions also activate the selection
void IntValueEditor::stepUp()
{
	updateValue(value() + singleStep());
}

void IntValueEditor::stepDown()
{
	updateValue(value() - singleStep());
}
