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
#include "editmodeselectorwidget.h"

EditModeSelectorWidget::EditModeSelectorWidget(QWidget* parent)
: QWidget(parent)
{
	setupUi(this);

	m_buttonGroup.addButton(m_moveButton, FigureEditor::Move);
	m_buttonGroup.addButton(m_rotateButton, FigureEditor::Rotate);
	m_buttonGroup.addButton(m_scaleButton, FigureEditor::Scale);
	m_buttonGroup.addButton(m_flipButton, FigureEditor::Flip);

	connect(&m_buttonGroup, SIGNAL(buttonPressed(int)), this, SLOT(groupButtonPressedSlot(int)));
}

void EditModeSelectorWidget::setSelectedButton(FigureEditor::EditMode id)
{
	if (m_buttonGroup.checkedId() != id)
	{
		blockSignals(true);
		m_buttonGroup.button(id)->setChecked(true);
		blockSignals(false);
	}
}

int EditModeSelectorWidget::selectedButton()
{
	return m_buttonGroup.checkedId();
}

void EditModeSelectorWidget::groupButtonPressedSlot(int id)
{
	emit buttonPressed((FigureEditor::EditMode)id);
}
