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

#include "colorsettingswidget.h"
#include "logger.h"

ColorSettingsWidget::ColorSettingsWidget(GenomeVector* g, QWidget* parent)
	: QWidget(parent), genome(g)
{
	setupUi(this);

	m_colorSelector->setGenomeVector(genome);

	m_colorLineEdit->setWheelEventUpdate(true);
	m_colorLineEdit->restoreSettings();
	connect(m_colorLineEdit, SIGNAL(valueUpdated()), this, SLOT(colorChangedAction()));
	connect(m_colorLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));

	m_symLineEdit->setWheelEventUpdate(true);
	m_symLineEdit->restoreSettings();
	connect(m_symLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_symLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));

	m_opacityLineEdit->setWheelEventUpdate(true);
	m_opacityLineEdit->restoreSettings();
	connect(m_opacityLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));

	connect(m_backgroundLabel, SIGNAL(colorSelected(QColor)), this, SLOT(changeBackground(QColor)));
	connect(m_backgroundLabel, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_colorSelector, SIGNAL(colorSelected(double)), this, SLOT(colorSelectedAction(double)));
	connect(m_colorSelector, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
}

void ColorSettingsWidget::reset()
{
	updateFormData();
}

#define genome_ptr (genome->selectedGenome())

void ColorSettingsWidget::updateFormData()
{
	logFiner("ColorSettingsWidget::updateFormData : updating color settings");
	m_symLineEdit->updateValue(selectedTriangle->xform()->color_speed);
	m_opacityLineEdit->updateValue(selectedTriangle->xform()->opacity);
	m_backgroundLabel->setColor(
		QColor::fromRgbF(genome_ptr->background[0],
			genome_ptr->background[1],
			genome_ptr->background[2]));
	m_colorSelector->setGenome(0);
	updateColorLabel();
}

void ColorSettingsWidget::updateColorLabel()
{
	QColor color = Util::get_xform_color(genome_ptr, selectedTriangle->xform());
	QSize size = m_colorLabel->maximumSize();
	QImage im(size, QImage::Format_RGB32);
	QPainter p(&im);
	p.fillRect(im.rect(), QBrush(color));
	m_colorLabel->setPixmap(QPixmap::fromImage( im ));
	m_colorLineEdit->updateValue(selectedTriangle->xform()->color);
	m_colorSelector->setSelectedIndex((int)(m_colorLineEdit->value() * 255.0));
}

void ColorSettingsWidget::fieldEditedAction()
{
	selectedTriangle->xform()->color_speed = m_symLineEdit->value();
	selectedTriangle->xform()->opacity = m_opacityLineEdit->value();
	emit dataChanged();
}

// called when the spinbox is modified
void ColorSettingsWidget::colorChangedAction()
{
	colorSelectedAction(m_colorLineEdit->value());
	emit dataChanged();
}

// called when the button is held
void ColorSettingsWidget::colorSelectedAction(double idx)
{
	selectedTriangle->xform()->color = idx;
	updateColorLabel();
	emit colorSelected(idx);
}

void ColorSettingsWidget::changeBackground(QColor c)
{
	genome_ptr->background[0] = c.redF();
	genome_ptr->background[1] = c.greenF();
	genome_ptr->background[2] = c.blueF();
	emit dataChanged();
}


void ColorSettingsWidget::triangleSelectedSlot(Triangle* t)
{
    selectedTriangle = t;
	updateFormData();
}


void ColorSettingsWidget::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
		updateFormData();
}


