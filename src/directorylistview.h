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
#ifndef DIRECTORYLISTVIEW_H
#define DIRECTORYLISTVIEW_H

#include <QListView>
#include <QMouseEvent>
#include <QHideEvent>

#include "mainviewer.h"

class DirectoryListView : public QListView
{
	Q_OBJECT

	QModelIndex dragStartIndex;

	public:
		DirectoryListView(QWidget* parent=0);
		~DirectoryListView();

	public slots:
		void viewerResizedAction(const QSize&);

	protected:
		void startDrag(Qt::DropActions);
		void mousePressEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);

	private:
		MainViewer* viewer;
		QImage viewerImage;
		QSize viewerSize;
};


#endif
