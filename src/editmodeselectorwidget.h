/***************************************************************************
 *   Copyright (C) 2007-2024 by David Bitseff                              *
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
#ifndef EDITMODESELECTORWIDGET_H
#define EDITMODESELECTORWIDGET_H

#include <QButtonGroup>

#include "ui_editmodeselectorwidget.h"
#include "xfedit.h"

class EditModeSelectorWidget : public QWidget, private Ui::EditModeSelectorWidget
{
	Q_OBJECT

	FigureEditor* m_xfeditor;
	Triangle* selectedTriangle;
	QButtonGroup m_buttonGroup;
	double lastRotateValue;

	public:
		EditModeSelectorWidget(FigureEditor*, QWidget* parent=0);
		void setFigureEditor(FigureEditor*);
		int selectedButton();
		void reset();

	public slots:
		void setSelectedButton(FigureEditor::EditMode);
		void triangleSelectedSlot(Triangle*);

	signals:
		void buttonPressed(FigureEditor::EditMode);
		void undoStateSignal();

	private slots:
		void groupButtonPressedSlot(int);
		void triangleRotateAction();
		void triangleRotateCWAction();
		void triangleRotateCCWAction();
		void moveTriangleLeftAction();
		void moveTriangleRightAction();
		void moveTriangleUpAction();
		void moveTriangleDownAction();
		void rotateTextEditedSlot();
		void toggleMarkAction(bool);
		void triangleScaleUpAction();
		void triangleScaleDownAction();
		void sceneAxesSelected(int);
		void selectionItemsChangedAction(int);
		void sceneScaledSlot();
		void sceneCenteredSlot(int);
		void sceneConfigSlot();
		void buttonReleasedSlot();

	protected:
		void closeEvent(QCloseEvent*);

};


#endif
