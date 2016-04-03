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
#include <QPainter>

#include "colordialog.h"


ColorDialog::ColorDialog(QWidget* parent) : QDialog(parent)
{
	setupUi(this);

	setSelectedColor(QColor(Qt::white));

	connect(m_hsLabel, SIGNAL(colorSelected(QPoint)),
			this, SLOT(indexSelected(QPoint)));
	connect(m_vLabel, SIGNAL(colorSelected(QPoint)),
			this, SLOT(indexSelected(QPoint)));
	connect(m_okButton, SIGNAL(pressed()), this, SLOT(accept()));
	connect(m_cancelButton, SIGNAL(pressed()), this, SLOT(reject()));

	connect(m_rSpinBox, SIGNAL(valueChanged(int)), this, SLOT(rgbSpinValueChanged()));
	connect(m_gSpinBox, SIGNAL(valueChanged(int)), this, SLOT(rgbSpinValueChanged()));
	connect(m_bSpinBox, SIGNAL(valueChanged(int)), this, SLOT(rgbSpinValueChanged()));
	connect(m_aSpinBox, SIGNAL(valueChanged(int)), this, SLOT(rgbSpinValueChanged()));

	connect(m_hSpinBox, SIGNAL(valueChanged(int)), this, SLOT(hsvSpinValueChanged()));
	connect(m_sSpinBox, SIGNAL(valueChanged(int)), this, SLOT(hsvSpinValueChanged()));
	connect(m_vSpinBox, SIGNAL(valueChanged(int)), this, SLOT(hsvSpinValueChanged()));
}


ColorDialog::~ColorDialog()
{
}


void ColorDialog::indexSelected(QPoint /*p*/)
{
	QPoint hs_pos(m_hsLabel->selectedIndex());
	QPoint v_pos(m_vLabel->selectedIndex());
	int h = qBound(0, hs_pos.x(), 359);
	int s = qBound(0, hs_pos.y(), 255);
	int v = qBound(0, v_pos.y(), 255);
	setSelectedColor(QColor::fromHsv(h, s, v, m_aSpinBox->value()));
	emit colorSelected(selectedColor);
}


QColor ColorDialog::getSelectedColor()
{
	return selectedColor;
}


void ColorDialog::setSelectedColor(QColor c)
{
	selectedColor = c;
	m_selectedColorLabel->setColor(selectedColor);
	m_vLabel->setSelectedIndex(c);
	m_hsLabel->setSelectedIndex(c);

	m_rSpinBox->blockSignals(true);
	m_gSpinBox->blockSignals(true);
	m_bSpinBox->blockSignals(true);
	m_aSpinBox->blockSignals(true);
	m_hSpinBox->blockSignals(true);
	m_sSpinBox->blockSignals(true);
	m_vSpinBox->blockSignals(true);

	m_rSpinBox->setValue(selectedColor.red());
	m_gSpinBox->setValue(selectedColor.green());
	m_bSpinBox->setValue(selectedColor.blue());
	m_aSpinBox->setValue(selectedColor.alpha());

	m_hSpinBox->setValue(selectedColor.hue());
	m_sSpinBox->setValue(selectedColor.saturation());
	m_vSpinBox->setValue(selectedColor.value());

	m_rSpinBox->blockSignals(false);
	m_gSpinBox->blockSignals(false);
	m_bSpinBox->blockSignals(false);
	m_aSpinBox->blockSignals(false);
	m_hSpinBox->blockSignals(false);
	m_sSpinBox->blockSignals(false);
	m_vSpinBox->blockSignals(false);

}

void ColorDialog::setAlphaEnabled(bool flag)
{
	label_7->setEnabled(flag);
	m_aSpinBox->setEnabled(flag);
}


void ColorDialog::rgbSpinValueChanged()
{
	setSelectedColor(QColor(m_rSpinBox->value(), m_gSpinBox->value(), m_bSpinBox->value(), m_aSpinBox->value()));
}


void ColorDialog::hsvSpinValueChanged()
{
	setSelectedColor(QColor::fromHsv(m_hSpinBox->value(), m_sSpinBox->value(), m_vSpinBox->value(), m_aSpinBox->value()));
}


ValSelector::ValSelector(QWidget* parent)
	: ColorSelector(parent)
{
	QSize s = size();
	background = QImage(s.width(), 256, QImage::Format_RGB32);
	QPainter p(&background);
	for (int i = 0 ; i < 256 ; i++)
	{
		p.setPen(QPen(qRgb(i, i, i)));
		p.drawLine(0, i, s.width(), i);
	}
	repaintLabel();
}

void ValSelector::setSelectedIndex(QColor c)
{
	ColorSelector::setSelectedIndex(QPoint(0, c.value()));
}

void ValSelector::repaintLabel()
{
	QImage label(background);
	QPainter p(&label);

	int last_y = last_pos.y();
	p.setPen(QPen(qRgb(0, 255 - last_y, 255 - last_y)));
	p.drawLine(0, last_y, size().width(), last_y);

	setPixmap(QPixmap::fromImage( label ));
}


HueSatSelector::HueSatSelector(QWidget* parent)
	: ColorSelector(parent), value(0)
{
	background = QImage(360, 256, QImage::Format_RGB32);
	setSelectedIndex(Qt::white);
}

void HueSatSelector::setSelectedIndex(QColor c)
{
	if (value != c.value())
	{
		value = c.value();
		for (int i = 0 ; i < 360 ; i++)
			for (int j = 0 ; j < 256 ; j++)
				background.setPixel(i, j, QColor::fromHsv(i, j, value).rgb());
	}
	ColorSelector::setSelectedIndex(QPoint(c.hue(),c.saturation()));
}

void HueSatSelector::repaintLabel()
{
	QImage label(background);
	QPainter p(&label);
	p.setPen(QPen(qRgb(255 - value, 255 - value, 255 - value)));
	p.drawEllipse(qBound(0, last_pos.x(), 359) - 5,
		qBound(0, last_pos.y(), 255) - 5, 10, 10);
	setPixmap(QPixmap::fromImage( label ));
}

