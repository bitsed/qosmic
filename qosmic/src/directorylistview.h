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
		void hideEvent(QHideEvent*);

	private:
		MainViewer* viewer;
		QImage viewerImage;
		QSize viewerSize;
};


#endif
