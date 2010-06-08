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
#ifndef EDITTMODESELECTORWIDGET_H
#define EDITTMODESELECTORWIDGET_H

#include <QButtonGroup>

#include "ui_editmodeselectorwidget.h"
#include "xfedit.h"

class EditModeSelectorWidget : public QWidget, private Ui::EditModeSelectorWidget
{
	Q_OBJECT

	public:
		EditModeSelectorWidget(QWidget* parent=0);
		int selectedButton();

	public slots:
		void setSelectedButton(FigureEditor::EditMode);

	signals:
		void buttonPressed(FigureEditor::EditMode);

	private slots:
		void groupButtonPressedSlot(int);

	private:
		QButtonGroup m_buttonGroup;

};


#endif
