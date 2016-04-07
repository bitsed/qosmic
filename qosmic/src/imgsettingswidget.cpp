/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
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

#include "imgsettingswidget.h"

ImageSettingsWidget::ImageSettingsWidget(GenomeVector* gen, QWidget* parent)
	: QWidget(parent), genome(gen)
{
	setupUi(this);

	// restore (Double/Int)ValueEditor settings
	m_qualityLineEdit->restoreSettings();
	m_filterLineEdit->restoreSettings();
	m_oversampleLineEdit->restoreSettings();
	m_batchesLineEdit->restoreSettings();
	m_estimatorLineEdit->restoreSettings();
	m_estimatorCurveLineEdit->restoreSettings();
	m_estimatorMinLineEdit->restoreSettings();
	m_symmetryLineEdit->restoreSettings();

	presets = new ViewerPresetsWidget(genome);
	presets->hide();
	m_presetsComboBox->clear();
	m_presetsComboBox->addItem(QString());
	m_presetsComboBox->addItems( presets->presetNames() );

	timer = new QTimer(this);
	timer->setInterval(2000);
	timer->setSingleShot(true);

	connect(m_qualityLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_filterLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_oversampleLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_batchesLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_estimatorLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_estimatorCurveLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_estimatorMinLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_symmetryLineEdit, SIGNAL(valueUpdated()), this, SLOT(fieldEditedAction()));
	connect(m_filterShapeBox, SIGNAL(activated(int)), this, SLOT(fieldEditedAction()));
	connect(m_applySymmetryButton, SIGNAL(pressed()), this, SLOT(applySymmetryAction()));

	connect(m_goLeftButton, SIGNAL(pressed()), this, SLOT(moveStackLeftAction()));
	connect(m_goRightButton, SIGNAL(pressed()), this, SLOT(moveStackRightAction()));
	connect(m_presetsButton, SIGNAL(clicked(bool)), this, SLOT(showPresetsDialog()));
	connect(m_presetsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectPresetAction(int)));
	connect(presets, SIGNAL(presetSelected()), this, SIGNAL(presetSelected()));
	connect(presets, SIGNAL(dataChanged()), this, SLOT(presetsDataChangedAction()));
	connect(timer, SIGNAL(timeout()), this, SLOT(resetPresetsComboBoxAction()));
}

void ImageSettingsWidget::reset()
{
	updateFormData();
}

#define genome_ptr (genome->selectedGenome())

void ImageSettingsWidget::hideEvent(QHideEvent* /*e*/)
{
	presets->hide();
	presets->close();
}

void ImageSettingsWidget::moveStackLeftAction()
{
	m_stackedWidget->setCurrentIndex(
		qBound(0, m_stackedWidget->currentIndex() - 1,  m_stackedWidget->count() - 1));
}

void ImageSettingsWidget::moveStackRightAction()
{
	m_stackedWidget->setCurrentIndex(
		qBound(0, m_stackedWidget->currentIndex() + 1,  m_stackedWidget->count() - 1));
}

void ImageSettingsWidget::showPresetsDialog()
{
	presets->show();
	presets->move(QCursor::pos() + QPoint(0, -presets->height()));
}

void ImageSettingsWidget::selectPresetAction(int idx)
{
	if (idx > 0)
		presets->selectPreset(idx - 1);
}

void ImageSettingsWidget::presetsDataChangedAction()
{
	m_presetsComboBox->blockSignals(true);
	m_presetsComboBox->clear();
	m_presetsComboBox->addItem(QString());
	m_presetsComboBox->addItems( presets->presetNames() );
	m_presetsComboBox->setCurrentIndex(0);
	m_presetsComboBox->blockSignals(false);
}

void ImageSettingsWidget::applySymmetryAction()
{
	int sym = m_symmetryLineEdit->value();
	if (sym == 1) return;
	flam3_add_symmetry(genome_ptr, sym);
	genome_ptr->symmetry = 1;
	m_symmetryLineEdit->setValue(1);
	emit symmetryAdded();
}

void ImageSettingsWidget::resetPresetsComboBoxAction()
{
	m_presetsComboBox->blockSignals(true);
	m_presetsComboBox->setCurrentIndex(0);
	m_presetsComboBox->blockSignals(false);
}

void ImageSettingsWidget::updateFormData()
{
	m_qualityLineEdit->updateValue(genome_ptr->sample_density);
	m_filterLineEdit->updateValue(genome_ptr->spatial_filter_radius);
	m_oversampleLineEdit->updateValue(genome_ptr->spatial_oversample);
	m_batchesLineEdit->updateValue(genome_ptr->nbatches);
	m_estimatorLineEdit->updateValue(genome_ptr->estimator);
	m_estimatorCurveLineEdit->updateValue(genome_ptr->estimator_curve);
	m_estimatorMinLineEdit->updateValue(genome_ptr->estimator_minimum);
	m_symmetryLineEdit->updateValue(genome_ptr->symmetry);

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

	timer->start();
}

void ImageSettingsWidget::fieldEditedAction()
{
	genome_ptr->sample_density = qMax(0.00001, m_qualityLineEdit->value());
	genome_ptr->spatial_filter_radius = m_filterLineEdit->value();
	genome_ptr->spatial_oversample = m_oversampleLineEdit->value();
	genome_ptr->nbatches = m_batchesLineEdit->value();
	genome_ptr->estimator = m_estimatorLineEdit->value();
	genome_ptr->estimator_curve = m_estimatorCurveLineEdit->value();
	genome_ptr->estimator_minimum
		= qMin(m_estimatorMinLineEdit->value(), genome_ptr->estimator);
	genome_ptr->symmetry = m_symmetryLineEdit->value();

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

	resetPresetsComboBoxAction();

	emit dataChanged();
}




