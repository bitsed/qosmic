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
#include <QSettings>
#include <QButtonGroup>

#include "flam3util.h"
#include "trianglecoordswidget.h"
#include "xfedit.h"
#include "logger.h"

using namespace Util;

TriangleCoordsWidget::TriangleCoordsWidget(GenomeVector* gen, QWidget* parent)
	: QWidget(parent), genome(gen)
{
	setupUi(this);

	m_AxLineEdit->restoreSettings();
	m_AyLineEdit->restoreSettings();
	m_BxLineEdit->restoreSettings();
	m_ByLineEdit->restoreSettings();
	m_CxLineEdit->restoreSettings();
	m_CyLineEdit->restoreSettings();

	m_aLineEdit->restoreSettings();
	m_bLineEdit->restoreSettings();
	m_cLineEdit->restoreSettings();
	m_dLineEdit->restoreSettings();
	m_eLineEdit->restoreSettings();
	m_fLineEdit->restoreSettings();

	connect(m_AxLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldCoordsEditedAction()));
	connect(m_AxLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_AyLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldCoordsEditedAction()));
	connect(m_AyLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_BxLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldCoordsEditedAction()));
	connect(m_BxLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_ByLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldCoordsEditedAction()));
	connect(m_ByLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_CxLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldCoordsEditedAction()));
	connect(m_CxLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_CyLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldCoordsEditedAction()));
	connect(m_CyLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));

	connect(m_aLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldVariablesEditedAction()));
	connect(m_aLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_bLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldVariablesEditedAction()));
	connect(m_bLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_cLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldVariablesEditedAction()));
	connect(m_cLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_dLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldVariablesEditedAction()));
	connect(m_dLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_eLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldVariablesEditedAction()));
	connect(m_eLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_fLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldVariablesEditedAction()));
	connect(m_fLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));

	coordsButtonGroup = new QButtonGroup();
	coordsButtonGroup->addButton(m_rectRadioButton);
	coordsButtonGroup->addButton(m_degRadioButton);
	coordsButtonGroup->addButton(m_radRadioButton);

	QSettings settings;
	QString coordstr = settings.value("coords_widget_coordtype", "rect").toString();
	if (coordstr == m_radRadioButton->text())
	{
		m_radRadioButton->setChecked(true);
		coords = Radian;
	}
	else if (coordstr == m_degRadioButton->text())
	{
		m_degRadioButton->setChecked(true);
		coords = Degree;
	}
	else
	{
		m_rectRadioButton->setChecked(true);
		coords = Rect;
	}
	connect(coordsButtonGroup, SIGNAL(buttonClicked(int)),
		this, SLOT(coordsButtonGroupClickedAction()));
}

void TriangleCoordsWidget::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
		updateFormData();
}

void TriangleCoordsWidget::coordsButtonGroupClickedAction()
{
	QSettings settings;
	settings.setValue("coords_widget_coordtype",
		coordsButtonGroup->checkedButton()->text());
	if (m_rectRadioButton->isChecked())
		coords = Rect;
	else if (m_degRadioButton->isChecked())
		coords = Degree;
	else
		coords = Radian;
	updateFormData();
}

void TriangleCoordsWidget::setGenome(int n)
{
	genome_offset = n;
}

#define genome_ptr (genome->selectedGenome())

void TriangleCoordsWidget::updateFormData()
{
	if (!isVisible())
		return;

	logFiner("TriangleCoordsWidget::updateFormData : updating");
	updateVariablesFormData();
	updateCoordsFormData();

}

void TriangleCoordsWidget::updateCoordsFormData()
{
	Triangle* t = selectedTriangle;
	FigureEditor* editor = selectedTriangle->editor();
	if (editor->postEnabled())
		t = editor->post();
	TriangleCoords tc = t->getCoords();
	switch (coords)
	{
		case Rect:
		{
			m_AxLineEdit->updateValue(tc[0].x());
			m_AyLineEdit->updateValue(tc[0].y());
			m_BxLineEdit->updateValue(tc[1].x());
			m_ByLineEdit->updateValue(tc[1].y());
			m_CxLineEdit->updateValue(tc[2].x());
			m_CyLineEdit->updateValue(tc[2].y());
			m_xColumnLabel->setText("x");
			m_yColumnLabel->setText("y");
			break;
		}
		case Radian:
		{
			double r, p;
			rectToPolar(tc[0].x(), tc[0].y(), &r, &p);
			m_AxLineEdit->updateValue(r);
			m_AyLineEdit->updateValue(p);
			rectToPolar(tc[1].x() - tc[0].x(), tc[1].y() - tc[0].y(), &r, &p);
			m_BxLineEdit->updateValue(r);
			m_ByLineEdit->updateValue(p);
			rectToPolar(tc[2].x() - tc[0].x(), tc[2].y() - tc[0].y(), &r, &p);
			m_CxLineEdit->updateValue(r);
			m_CyLineEdit->updateValue(p);
			m_xColumnLabel->setText("r");
			m_yColumnLabel->setText("phi");
			break;
		}
		case Degree:
		{
			double r, p;
			rectToPolarDeg(tc[0].x(), tc[0].y(), &r, &p);
			m_AxLineEdit->updateValue(r);
			m_AyLineEdit->updateValue(p);
			rectToPolarDeg(tc[1].x() - tc[0].x(), tc[1].y() - tc[0].y(), &r, &p);
			m_BxLineEdit->updateValue(r);
			m_ByLineEdit->updateValue(p);
			rectToPolarDeg(tc[2].x() - tc[0].x(), tc[2].y() - tc[0].y(), &r, &p);
			m_CxLineEdit->updateValue(r);
			m_CyLineEdit->updateValue(p);
			m_xColumnLabel->setText("r");
			m_yColumnLabel->setText("deg");
		}
	}

}

void TriangleCoordsWidget::updateVariablesFormData()
{
	FigureEditor* editor = selectedTriangle->editor();
	if (editor->postEnabled())
	{
		Triangle* t = editor->post();
		flam3_xform* xform = t->xform();
		m_aLineEdit->updateValue(xform->post[0][0]);
		m_bLineEdit->updateValue(xform->post[1][0]);
		m_cLineEdit->updateValue(xform->post[2][0]);
		m_dLineEdit->updateValue(xform->post[0][1]);
		m_eLineEdit->updateValue(xform->post[1][1]);
		m_fLineEdit->updateValue(xform->post[2][1]);
	}
	else
	{
		Triangle* t = selectedTriangle;
		flam3_xform* xform = t->xform();
		m_aLineEdit->updateValue(xform->c[0][0]);
		m_bLineEdit->updateValue(xform->c[1][0]);
		m_cLineEdit->updateValue(xform->c[2][0]);
		m_dLineEdit->updateValue(xform->c[0][1]);
		m_eLineEdit->updateValue(xform->c[1][1]);
		m_fLineEdit->updateValue(xform->c[2][1]);
	}
}

void TriangleCoordsWidget::fieldCoordsEditedAction()
{
	logFiner("TriangleCoordsWidget::fieldEditedAction : setting coords");
	FigureEditor* editor = selectedTriangle->editor();
	Triangle* t = editor->postEnabled() ? editor->post() : selectedTriangle ;
	TriangleCoords c = t->getCoords();
	switch (coords)
	{
		case Rect:
		{
			c[0].setX(m_AxLineEdit->value());
			c[0].setY(m_AyLineEdit->value());
			c[1].setX(m_BxLineEdit->value());
			c[1].setY(m_ByLineEdit->value());
			c[2].setX(m_CxLineEdit->value());
			c[2].setY(m_CyLineEdit->value());
			break;
		}
		case Degree:
		{
			double x, y;
			double r = m_AxLineEdit->value();
			double p = m_AyLineEdit->value();
			polarDegToRect(r, p, &x, &y);
			c[0].setX(x);
			c[0].setY(y);
			r = m_BxLineEdit->value();
			p = m_ByLineEdit->value();
			polarDegToRect(r, p, &x, &y);
			c[1].setX(x + c[0].x());
			c[1].setY(y + c[0].y());
			r = m_CxLineEdit->value();
			p = m_CyLineEdit->value();
			polarDegToRect(r, p, &x, &y);
			c[2].setX(x + c[0].x());
			c[2].setY(y + c[0].y());
			break;
		}
		case Radian:
		{
			double x, y;
			double r = m_AxLineEdit->value();
			double p = m_AyLineEdit->value();
			polarToRect(r, p, &x, &y);
			c[0].setX(x);
			c[0].setY(y);
			r = m_BxLineEdit->value();
			p = m_ByLineEdit->value();
			polarToRect(r, p, &x, &y);
			c[1].setX(x + c[0].x());
			c[1].setY(y + c[0].y());
			r = m_CxLineEdit->value();
			p = m_CyLineEdit->value();
			polarToRect(r, p, &x, &y);
			c[2].setX(x + c[0].x());
			c[2].setY(y + c[0].y());
		}
	}
	t->setPoints(c);
	t->coordsToXForm();
	updateVariablesFormData();
	editor->blockSignals(true);
	editor->triangleModifiedAction(selectedTriangle);
	editor->blockSignals(false);
	emit dataChanged();
}

void TriangleCoordsWidget::fieldVariablesEditedAction()
{
	logFiner("TriangleCoordsWidget::fieldVariablesEditedAction : setting values");
	FigureEditor* editor = selectedTriangle->editor();
	Triangle* t;
	TriangleCoords c;

	if (editor->postEnabled())
	{
		t = editor->post();
		flam3_xform* xform = t->xform();
		xform->post[0][0] = m_aLineEdit->value();
		xform->post[1][0] = m_bLineEdit->value();
		xform->post[2][0] = m_cLineEdit->value();
		xform->post[0][1] = m_dLineEdit->value();
		xform->post[1][1] = m_eLineEdit->value();
		xform->post[2][1] = m_fLineEdit->value();
		c = t->basis()->getCoords(xform->post);
	}
	else
	{
		t = selectedTriangle;
		flam3_xform* xform = t->xform();
		xform->c[0][0] = m_aLineEdit->value();
		xform->c[1][0] = m_bLineEdit->value();
		xform->c[2][0] = m_cLineEdit->value();
		xform->c[0][1] = m_dLineEdit->value();
		xform->c[1][1] = m_eLineEdit->value();
		xform->c[2][1] = m_fLineEdit->value();
		c = t->basis()->getCoords(xform->c);
	}

	t->setPoints(c);
	t->coordsToXForm();
	updateCoordsFormData();
	editor->blockSignals(true);
	editor->triangleModifiedAction(selectedTriangle);
	editor->blockSignals(false);
	emit dataChanged();
}

void TriangleCoordsWidget::triangleSelectedSlot(Triangle* t)
{
	logFiner(QString("TriangleCoordsWidget::triangleSelectedSlot : t=0x%1")
			.arg((long)t, 0, 16));
	selectedTriangle = t;
	updateFormData();
}

void TriangleCoordsWidget::triangleModifiedSlot(Triangle* t)
{
	logFiner(QString("TriangleCoordsWidget::triangleModifiedSlot : t=0x%1")
			.arg((long)t, 0, 16));
	if (t == selectedTriangle)
		updateFormData();
}

TriangleCoordsWidget::CoordType TriangleCoordsWidget::coordType()
{
	return coords;
}

