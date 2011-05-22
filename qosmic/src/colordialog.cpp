/***************************************************************************
 *   Copyright (C) 2007 - 2011 by David Bitseff                            *
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
	setSelectedColor(QColor::fromHsv(h, s, v));
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

	m_rSpinBox->setValue(selectedColor.red());
	m_gSpinBox->setValue(selectedColor.green());
	m_bSpinBox->setValue(selectedColor.blue());

	m_hSpinBox->setValue(selectedColor.hue());
	m_sSpinBox->setValue(selectedColor.saturation());
	m_vSpinBox->setValue(selectedColor.value());
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


