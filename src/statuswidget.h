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
#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QWidget>
#include <QResizeEvent>

#include "ui_statuswidget.h"
#include "renderthread.h"

class StatusWidget : public QWidget, private Ui::StatusWidget
{
	Q_OBJECT

	public:
		StatusWidget(QWidget* parent);
		~StatusWidget();

	public slots:
		void setRenderStatus(RenderStatus*);


	protected:
		void resizeEvent(QResizeEvent*);
};

#endif
