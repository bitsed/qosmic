/***************************************************************************
 *   Copyright (C) 2009 by David Bitseff                                   *
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
#include <QColorDialog>

#include "adjustscenewidget.h"

AdjustSceneWidget::AdjustSceneWidget(FigureEditor* e, QWidget* parent)
	: QWidget(parent), QosmicWidget(this, "AdjustSceneWidget"), editor(e)
{
	setupUi(this);

	m_gridCheckBox->setChecked(editor->gridVisible());
	m_sceneCenterOnCheckbox->setChecked(editor->centeredScaling());

	connect(m_sceneScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(sceneScaledAction()));
	connect(m_gridCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleGridAction(bool)));
	connect(m_gridColorButton, SIGNAL(pressed()), this, SLOT(gridColorSelectAction()));
	connect(m_bgColorButton, SIGNAL(pressed()), this, SLOT(bgColorSelectAction()));
	connect(m_sceneCenterOnCheckbox, SIGNAL(toggled(bool)), this, SLOT(sceneCenterOnAction(bool)));
}


void AdjustSceneWidget::sceneScaledAction()
{
	int value = m_sceneScaleSlider->dx();
	Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
	double scale;
	if (value > 0)
	{
		if (mods & Qt::ShiftModifier)
			scale = 1.01;
		else if (mods & Qt::ControlModifier)
			scale = 1.10;
		else
			scale = 1.05;
	}
	else if (value < 0)
	{
		if (mods & Qt::ShiftModifier)
			scale = 0.99;
		else if (mods & Qt::ControlModifier)
			scale = 0.9090;
		else
			scale = 0.9523;
	}
	else
		return;

	editor->scaleBasis(scale, scale);
}

void AdjustSceneWidget::toggleGridAction(bool checked)
{
	editor->setGridVisible(checked);
}

void AdjustSceneWidget::gridColorSelectAction()
{
	QColor c = QColorDialog::getColor(editor->gridColor(), this);
	if (c.isValid())
		editor->setGridColor(c);
}

void AdjustSceneWidget::bgColorSelectAction()
{
	QColor c = QColorDialog::getColor(editor->bgColor(), this);
	if (c.isValid())
		editor->setbgColor(c);
}

void AdjustSceneWidget::sceneCenterOnAction(bool checked)
{
	editor->setCenteredScaling(checked);
}
