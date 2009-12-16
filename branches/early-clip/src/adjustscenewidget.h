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

#ifndef ADJUSTSCENEWIDGET_H
#define ADJUSTSCENEWIDGET_H


#include "ui_adjustscenewidget.h"
#include "qosmicwidget.h"
#include "xfedit.h"

class AdjustSceneWidget : public QWidget, public QosmicWidget,
	private Ui::AdjustSceneWidget
{
	Q_OBJECT

	public:
		AdjustSceneWidget(FigureEditor*, QWidget* parent=0);

	protected slots:
		void sceneScaledAction();
		void toggleGridAction(bool);
		void gridColorSelectAction();
		void bgColorSelectAction();
		void sceneCenterOnAction(bool);

	private:
		FigureEditor* editor;
};


#endif

