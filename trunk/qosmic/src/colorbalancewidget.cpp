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

#include "colorbalancewidget.h"
#include "renderthread.h"
#include "logger.h"

ColorBalanceWidget::ColorBalanceWidget(GenomeVector* g, QWidget* parent)
: QWidget(parent), genome(g)
{
	setupUi(this);

	m_contrastEditor->restoreSettings();
	m_brightnessEditor->restoreSettings();
	m_gammaEditor->restoreSettings();
	m_vibrancyEditor->restoreSettings();
	m_hueEditor->restoreSettings();
	m_highlightPowerEditor->restoreSettings();

	connect(m_contrastEditor, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_brightnessEditor, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_gammaEditor, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_vibrancyEditor, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_hueEditor, SIGNAL(valueUpdated()), this, SLOT(hueEditedAction()));
	connect(m_alphaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setImageFormat(int)));
	connect(m_clipCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEarlyClip(bool)));
	connect(m_highlightPowerEditor, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
}

#define genome_ptr (genome->selectedGenome())

void ColorBalanceWidget::reset()
{
	palette.clear();
	for (int n = 0 ; n < 256 ; n++ )
	{
		double rc, gc, bc, ac;
		rc = genome_ptr->palette[n].color[0];
		gc = genome_ptr->palette[n].color[1];
		bc = genome_ptr->palette[n].color[2];
		ac = genome_ptr->palette[n].color[3];
		if (rc < 0.0 || rc > 1.0 ||
			bc < 0.0 || bc > 1.0 ||
			gc < 0.0 || gc > 1.0)
			return;
		palette << QColor::fromRgbF(rc ,gc, bc, ac);
	}
	updateFormData();
}

void ColorBalanceWidget::updateFormData()
{
	m_contrastEditor->updateValue(genome_ptr->contrast);
	m_brightnessEditor->updateValue(genome_ptr->brightness);
	m_gammaEditor->updateValue(genome_ptr->gamma);
	m_vibrancyEditor->updateValue(genome_ptr->vibrancy);
	m_hueEditor->updateValue(genome_ptr->hue_rotation);
	m_highlightPowerEditor->updateValue(genome_ptr->highlight_power);

	logFine(QString("ColorBalanceWidget::updateFormData : cont %1  brite %2  gamma %3  hue %4")
			.arg(genome_ptr->contrast).arg(genome_ptr->brightness).arg(genome_ptr->gamma)
			.arg(genome_ptr->hue_rotation));
}

void ColorBalanceWidget::fieldEditedAction()
{
	genome_ptr->contrast = m_contrastEditor->value();
	genome_ptr->brightness = m_brightnessEditor->value();
	genome_ptr->gamma = m_gammaEditor->value();
	genome_ptr->vibrancy = m_vibrancyEditor->value();
	genome_ptr->hue_rotation = m_hueEditor->value();

	static double last_power = -2.0;
	double power = m_highlightPowerEditor->value();
	if (last_power != power)
	{
		if (last_power == -1.0)
		{
			last_power = power = 0.0;
			m_highlightPowerEditor->updateValue(power);
		}
		else if (power < 0.0)
		{
			last_power = power = -1.0;
			m_highlightPowerEditor->updateValue(power);
		}
		genome_ptr->highlight_power = power;
	}
	logFine(QString("ColorBalanceWidget::fieldEditedAction : cont %1  brite %2  gamma %3  hue %4  vib %5 ")
		.arg(genome_ptr->contrast).arg(genome_ptr->brightness)
		.arg(genome_ptr->gamma).arg(genome_ptr->hue_rotation)
		.arg(genome_ptr->vibrancy));

	emit dataChanged();
}

void ColorBalanceWidget::hueEditedAction()
{
	genome_ptr->hue_rotation = m_hueEditor->value();
	logFine(QString("ColorBalanceWidget::hueEditedAction : hue %1")
			.arg(genome_ptr->hue_rotation));
	int n = 0;
	foreach(QColor c, palette)
	{
		double hue, saturation, value;
		c.getHsvF( &hue, &saturation, &value );
		hue = qMax(0.0, hue);
		hue += genome_ptr->hue_rotation;
		hue = hue >= 1.0 ? hue - 1.0 : hue ;
		c.setHsvF( hue , saturation, value );
		c.getRgbF( &genome_ptr->palette[n].color[0],
			&genome_ptr->palette[n].color[1],
			&genome_ptr->palette[n].color[2],
			&genome_ptr->palette[n].color[3]);
		n++;
	}
	genome_ptr->hue_rotation = 0.0;
	emit paletteChanged();
}

void ColorBalanceWidget::setImageFormat(int format)
{
	RenderThread::getInstance()->setFormat(static_cast<RenderThread::ImageFormat>(format));
	emit dataChanged();
}

void ColorBalanceWidget::setEarlyClip(bool flag)
{
	RenderThread::getInstance()->setEarlyClip(flag);
	emit dataChanged();
}
