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
#ifndef RENDERPROGRESSDIALOG_H
#define RENDERPROGRESSDIALOG_H

#include "ui_renderprogressdialog.h"
#include "renderthread.h"
#include "mainviewer.h"

class RenderProgressDialog : public QDialog, private Ui::RenderProgressDialog
{
	Q_OBJECT

	QLabel m_finishedLabel;
	RenderThread* m_rthread;
	bool m_showMainViewer;

	public:
		RenderProgressDialog(QWidget*, RenderThread*);
		~RenderProgressDialog();
		bool showMainViewer();

	public slots:
		void setRenderStatus(RenderStatus*);

	private slots:
		void yesButtonPressedSlot();
};



#endif

