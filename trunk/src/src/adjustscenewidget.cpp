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
#include <QColorDialog>

#include "adjustscenewidget.h"

AdjustSceneWidget::AdjustSceneWidget(FigureEditor* e, QWidget* parent)
	: QDialog(parent), editor(e)
{
	setupUi(this);

	m_gridCheckBox->setChecked(editor->gridVisible());
	m_guideCheckBox->setChecked(editor->guideVisible());
	m_guideColorButton->setEnabled(editor->guideVisible());
	m_previewCheckBox->setChecked(editor->previewVisible());
	m_previewEditor->setEnabled(editor->previewVisible());
	m_previewEditor->updateValue(editor->previewDensity());

	connect(m_previewEditor, SIGNAL(valueUpdated()), this, SLOT(previewUpdatedAction()));
	connect(m_previewCheckBox, SIGNAL(toggled(bool)), this, SLOT(togglePreviewAction(bool)));
	connect(m_gridCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleGridAction(bool)));
	connect(m_gridColorButton, SIGNAL(pressed()), this, SLOT(gridColorSelectAction()));
	connect(m_guideCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleGuideAction(bool)));
	connect(m_guideColorButton, SIGNAL(pressed()), this, SLOT(guideColorSelectAction()));
	connect(m_bgColorButton, SIGNAL(pressed()), this, SLOT(bgColorSelectAction()));
}


void AdjustSceneWidget::previewUpdatedAction()
{
	editor->setPreviewDensity(m_previewEditor->value());
}

void AdjustSceneWidget::togglePreviewAction(bool checked)
{
	editor->setPreviewVisible(checked);
	m_previewEditor->setEnabled(checked);
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

void AdjustSceneWidget::toggleGuideAction(bool checked)
{
	editor->setGuideVisible(checked);
	m_guideColorButton->setEnabled(checked);
}

void AdjustSceneWidget::guideColorSelectAction()
{
	QColor c = QColorDialog::getColor(editor->guideColor(), this);
	if (c.isValid())
		editor->setGuideColor(c);
}

void AdjustSceneWidget::bgColorSelectAction()
{
	QColor c = QColorDialog::getColor(editor->bgColor(), this);
	if (c.isValid())
		editor->setbgColor(c);
}

