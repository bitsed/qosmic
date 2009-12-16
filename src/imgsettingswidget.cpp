/***************************************************************************
 *   Copyright (C) 2007 by David Bitseff                                   *
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

#include "imgsettingswidget.h"
#include <QAction>

ImageSettingsWidget::ImageSettingsWidget(GenomeVector* gen, QWidget* parent)
	: QWidget(parent), genome(gen)
{
	setupUi(this);

	// restore (Double/Int)ValueEditor settings
	m_qualityLineEdit->restoreSettings();
	m_filterLineEdit->restoreSettings();
	m_oversampleLineEdit->restoreSettings();
	m_batchesLineEdit->restoreSettings();
	m_temporalLineEdit->restoreSettings();
	m_estimatorLineEdit->restoreSettings();
	m_estimatorCurveLineEdit->restoreSettings();
	m_estimatorMinLineEdit->restoreSettings();
	m_symmetryLineEdit->restoreSettings();
	m_timeLineEdit->restoreSettings();

	connect(m_qualityLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_filterLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_oversampleLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_batchesLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_temporalLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_estimatorLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_estimatorCurveLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_estimatorMinLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_symmetryLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_filterShapeBox, SIGNAL(activated(int)), this, SLOT(fieldEditedAction()));
	connect(m_applySymmetryButton, SIGNAL(pressed()), this, SLOT(applySymmetryAction()));
	connect(m_timeLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_interpolationBox, SIGNAL(activated(int)), this, SLOT(fieldEditedAction()));
	connect(m_interpolationTypeBox, SIGNAL(activated(int)), this, SLOT(fieldEditedAction()));
}

void ImageSettingsWidget::reset()
{
	updateFormData();
}

#define genome_ptr (genome->selectedGenome())


void ImageSettingsWidget::applySymmetryAction()
{
	int sym = m_symmetryLineEdit->value();
	if (sym == 1) return;
	flam3_add_symmetry(genome_ptr, sym);
	genome_ptr->symmetry = 1;
	m_symmetryLineEdit->setValue(1);
	emit symmetryAdded();
}

void ImageSettingsWidget::updateFormData()
{
	m_qualityLineEdit->updateValue(genome_ptr->sample_density);
	m_filterLineEdit->updateValue(genome_ptr->spatial_filter_radius);
	m_oversampleLineEdit->updateValue(genome_ptr->spatial_oversample);
	m_batchesLineEdit->updateValue(genome_ptr->nbatches);
	m_temporalLineEdit->updateValue(genome_ptr->ntemporal_samples);
	m_estimatorLineEdit->updateValue(genome_ptr->estimator);
	m_estimatorCurveLineEdit->updateValue(genome_ptr->estimator_curve);
	m_estimatorMinLineEdit->updateValue(genome_ptr->estimator_minimum);
	m_symmetryLineEdit->updateValue(genome_ptr->symmetry);
	m_timeLineEdit->updateValue(genome_ptr->time);

	// filter_shape
	QString option;
	if (genome_ptr->spatial_filter_select == flam3_gaussian_kernel)
		option ="gaussian";
	else if (genome_ptr->spatial_filter_select == flam3_hermite_kernel)
		option ="hermite";
	else if (genome_ptr->spatial_filter_select == flam3_box_kernel)
		option ="box";
	else if (genome_ptr->spatial_filter_select == flam3_triangle_kernel)
		option ="triangle";
	else if (genome_ptr->spatial_filter_select == flam3_bell_kernel)
		option ="bell";
	else if (genome_ptr->spatial_filter_select == flam3_b_spline_kernel)
		option ="bspline";
	else if (genome_ptr->spatial_filter_select == flam3_mitchell_kernel)
		option ="mitchell";
	else if (genome_ptr->spatial_filter_select == flam3_blackman_kernel)
		option ="blackman";
	else if (genome_ptr->spatial_filter_select == flam3_catrom_kernel)
		option ="catrom";
	else if (genome_ptr->spatial_filter_select == flam3_hanning_kernel)
		option ="hanning";
	else if (genome_ptr->spatial_filter_select == flam3_hamming_kernel)
		option ="hamming";
	else if (genome_ptr->spatial_filter_select == flam3_lanczos3_kernel)
		option ="lanczos3";
	else if (genome_ptr->spatial_filter_select == flam3_lanczos2_kernel)
		option ="lanczos2";
	else if (genome_ptr->spatial_filter_select == flam3_quadratic_kernel)
		option ="quadratic";
	m_filterShapeBox->setCurrentIndex(m_filterShapeBox->findText(option));

	if (genome_ptr->interpolation == flam3_interpolation_linear)
		option = "linear";
	else
		option = "smooth";
	m_interpolationBox->setCurrentIndex(m_interpolationBox->findText(option));

	if (genome_ptr->interpolation_type == flam3_inttype_linear)
		option = "linear";
	else if (genome_ptr->interpolation_type == flam3_inttype_log)
		option = "log";
	else if (genome_ptr->interpolation_type == flam3_inttype_compat)
		option = "compat";
	else
		option = "older";
	m_interpolationTypeBox->setCurrentIndex(m_interpolationTypeBox->findText(option));
}

void ImageSettingsWidget::fieldEditedAction()
{
	genome_ptr->sample_density = qMax(0.00001, m_qualityLineEdit->value());
	genome_ptr->spatial_filter_radius = m_filterLineEdit->value();
	genome_ptr->spatial_oversample = m_oversampleLineEdit->value();
	genome_ptr->nbatches = m_batchesLineEdit->value();
	genome_ptr->ntemporal_samples = QString(m_temporalLineEdit->text()).toInt();
	genome_ptr->estimator = m_estimatorLineEdit->value();
	genome_ptr->estimator_curve = m_estimatorCurveLineEdit->value();
	genome_ptr->estimator_minimum
		= qMin(m_estimatorMinLineEdit->value(), genome_ptr->estimator);
	genome_ptr->symmetry = m_symmetryLineEdit->value();
	genome_ptr->time = m_timeLineEdit->value();

	QString option = m_filterShapeBox->currentText();
	if (option == "gaussian")
		genome_ptr->spatial_filter_select = flam3_gaussian_kernel;
	else if (option == "hermite")
		genome_ptr->spatial_filter_select = flam3_hermite_kernel;
	else if (option == "box")
		genome_ptr->spatial_filter_select = flam3_box_kernel;
	else if (option == "triangle")
		genome_ptr->spatial_filter_select = flam3_triangle_kernel;
	else if (option == "bell")
		genome_ptr->spatial_filter_select = flam3_bell_kernel;
	else if (option == "bspline")
		genome_ptr->spatial_filter_select = flam3_b_spline_kernel;
	else if (option == "mitchell")
		genome_ptr->spatial_filter_select = flam3_mitchell_kernel;
	else if (option == "blackman")
		genome_ptr->spatial_filter_select = flam3_blackman_kernel;
	else if (option == "catrom")
		genome_ptr->spatial_filter_select = flam3_catrom_kernel;
	else if (option == "hanning")
		genome_ptr->spatial_filter_select = flam3_hanning_kernel;
	else if (option == "hamming")
		genome_ptr->spatial_filter_select = flam3_hamming_kernel;
	else if (option == "lanczos3")
		genome_ptr->spatial_filter_select = flam3_lanczos3_kernel;
	else if (option == "lanczos2")
		genome_ptr->spatial_filter_select = flam3_lanczos2_kernel;
	else if (option == "quadratic")
		genome_ptr->spatial_filter_select = flam3_quadratic_kernel;
	else
		genome_ptr->spatial_filter_select = flam3_gaussian_kernel;

	option = m_interpolationBox->currentText();
	if (option == "linear")
		genome_ptr->interpolation = flam3_interpolation_linear;
	else
		genome_ptr->interpolation = flam3_interpolation_smooth;

	option = m_interpolationTypeBox->currentText();
	if (option == "linear")
		genome_ptr->interpolation_type = flam3_inttype_linear;
	else if (option == "log")
		genome_ptr->interpolation_type = flam3_inttype_log;
	else if (option == "compat")
		genome_ptr->interpolation_type = flam3_inttype_compat;
	else
		genome_ptr->interpolation_type = flam3_inttype_older;

	emit dataChanged();
}




