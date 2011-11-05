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

#include "snapslider.h"

SnapSlider::SnapSlider(QWidget* parent)
	: QSlider(Qt::Horizontal, parent)
{
	m_selected_value = 0;
	m_last_value = 0;
	m_snap = 0;
	connect(this, SIGNAL(rangeChanged(int,int)),
		this, SLOT(rangeChangedAction(int,int)));
	connect(this, SIGNAL(sliderReleased()),
		this, SLOT(sliderReleasedAction()));
	connect(this, SIGNAL(valueChanged(int)),
		this, SLOT(sliderChangedAction(int)));
}

int SnapSlider::dx()
{
	return m_dx;
}

void SnapSlider::setSnap(int value)
{
	m_snap = value;
}

int SnapSlider::snap()
{
	return m_snap;
}

int SnapSlider::selectedValue()
{
	return m_selected_value;
}

void SnapSlider::rangeChangedAction(int min, int max)
{
	if (m_snap < min || m_snap > max)
		m_snap = (int)((double)(max - min) / 2.0);
	setValue(m_snap);
}

void SnapSlider::sliderReleasedAction()
{
	m_selected_value = value();
	if (signalsBlocked())
		setValue(m_snap);
	else
	{
		blockSignals(true);
		setValue(m_snap);
		blockSignals(false);
	}
	m_last_value = m_snap;
	emit valueSelected(m_selected_value);
	emit undoStateSignal();
}

void SnapSlider::sliderChangedAction(int value)
{
	m_dx = value - m_last_value;
	m_last_value = value;
}

void SnapSlider::setToSnap()
{
	blockSignals(true);
	sliderReleasedAction();
	blockSignals(false);
}
