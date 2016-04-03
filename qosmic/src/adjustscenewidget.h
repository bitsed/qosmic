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

#ifndef ADJUSTSCENEWIDGET_H
#define ADJUSTSCENEWIDGET_H

#include "ui_adjustscenewidget.h"
#include "xfedit.h"

class AdjustSceneWidget : public QDialog, private Ui::AdjustSceneWidget
{
	Q_OBJECT

	public:
		AdjustSceneWidget(FigureEditor*, QWidget* parent=0);

	protected slots:
		void toggleGridAction(bool);
		void gridColorSelectAction();
		void toggleGuideAction(bool);
		void guideColorSelectAction();
		void bgColorSelectAction();
		void togglePreviewAction(bool);
		void previewUpdatedAction();

	private:
		FigureEditor* editor;
};


#endif

