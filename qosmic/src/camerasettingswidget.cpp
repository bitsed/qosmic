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

#include "camerasettingswidget.h"

CameraSettingsWidget::CameraSettingsWidget(GenomeVector* gen, QWidget* parent)
	: QWidget(parent), genome(gen)
{
	setupUi(this);

	// restore (Double/Int)ValueEditor settings
	m_zoomLineEdit->restoreSettings();
	m_xposLineEdit->restoreSettings();
	m_yposLineEdit->restoreSettings();
	m_scaleLineEdit->restoreSettings();
	m_rotateLineEdit->restoreSettings();
	m_sizewLineEdit->restoreSettings();
	m_sizehLineEdit->restoreSettings();

	connect(m_zoomLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_zoomLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_xposLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_xposLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_yposLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_yposLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_scaleLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_scaleLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_rotateLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_rotateLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_sizewLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_sizewLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_sizehLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_sizehLineEdit, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
}

void CameraSettingsWidget::reset()
{
	updateFormData();
}

#define genome_ptr (genome->selectedGenome())

void CameraSettingsWidget::updateFormData()
{
	m_zoomLineEdit->updateValue(genome_ptr->zoom);
	m_scaleLineEdit->updateValue(genome_ptr->pixels_per_unit);
	m_xposLineEdit->updateValue(genome_ptr->center[0]);
	m_yposLineEdit->updateValue(genome_ptr->center[1]);
	m_rotateLineEdit->updateValue(genome_ptr->rotate);
	m_sizewLineEdit->updateValue(genome_ptr->width);
	m_sizehLineEdit->updateValue(genome_ptr->height);
}

void CameraSettingsWidget::fieldEditedAction()
{
	int width = m_sizewLineEdit->value();
	if (genome_ptr->width != width)
	{
		// this "autoscales" the image scale to maintain the camera
		// when changing the image size
		genome_ptr->pixels_per_unit *= (double)(width) / (genome_ptr->width);
		m_scaleLineEdit->updateValue(genome_ptr->pixels_per_unit);
	}
	else
		genome_ptr->pixels_per_unit = m_scaleLineEdit->value();

	genome_ptr->zoom = m_zoomLineEdit->value();
	genome_ptr->center[0] = m_xposLineEdit->value();
	genome_ptr->center[1] = m_yposLineEdit->value();
	genome_ptr->rotate = m_rotateLineEdit->value();
	genome_ptr->width = m_sizewLineEdit->value();
	genome_ptr->height = m_sizehLineEdit->value();
	emit dataChanged();
}

