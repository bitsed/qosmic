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
#include <QFileSystemModel>
#include <QMimeData>
#include <QDrag>
#include <QUrl>

#include "directorylistview.h"
#include "logger.h"

DirectoryListView::DirectoryListView(QWidget* parent)
: QListView(parent), viewer(0), viewerSize(320,200)
{
}

DirectoryListView::~DirectoryListView()
{
	if (viewer) delete viewer;
}

void DirectoryListView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton)
	{
		QModelIndex idx = indexAt(event->pos());
		if (idx.isValid())
		{
			QFileSystemModel* m = qobject_cast<QFileSystemModel*>(model());
			QFileInfo file( m->fileInfo(idx) );
			if (file.isFile())
			{
				QString s( file.absoluteFilePath() );
				s.replace(QRegExp("flam(3|e)?$"), "png");
				logFine(QString("DirectoryListView::mousePressEvent : showing %1").arg(s));
				viewerImage = QImage(s);
				if (viewerImage.isNull())
					logWarn(QString("DirectoryListView::mousePressEvent : null image %1").arg(s));
				else
					if (!viewer)
					{
						viewer = new MainViewer(0, "FileViewer");
						connect(viewer, SIGNAL(viewerResized(const QSize&)),
								this, SLOT(viewerResizedAction(const QSize&)));
						viewer->setPixmap(QPixmap::fromImage(viewerImage),false);
						viewer->show();
						viewer->raise();
						viewer->scaleResetAction();
					}
					else
					{
						viewer->setPixmap(QPixmap::fromImage(viewerImage),false);
						viewer->show();
						viewer->raise();
					}
			}
		}
	}
	else if (event->button() == Qt::MidButton)
	{
		// toggle showing hidden files in the model
		QFileSystemModel* fsm = qobject_cast<QFileSystemModel*>(model());
		fsm->setFilter( fsm->filter() ^ QDir::Hidden );
	}
	else
	{
		QListView::mousePressEvent(event);
		dragStartIndex = indexAt(event->pos());
	}
}

void DirectoryListView::startDrag(Qt::DropActions supportedActions)
{
	if (dragStartIndex.isValid())
	{
		QFileSystemModel* m = qobject_cast<QFileSystemModel*>(model());
		QFileInfo file( m->fileInfo(dragStartIndex) );
		if (file.isFile())
		{
			QDrag* drag = new QDrag(this);
			QMimeData* mimeData = new QMimeData;
			QList<QUrl> urlList;
			urlList.append(QUrl::fromLocalFile(file.absoluteFilePath()));
			mimeData->setUrls(urlList);
			drag->setMimeData(mimeData);
			drag->setPixmap(m->fileIcon(dragStartIndex).pixmap(iconSize()));
			drag->exec(supportedActions);
		}
	}
}

void DirectoryListView::wheelEvent(QWheelEvent* e)
{
	if (e->modifiers() & Qt::ControlModifier)
	{
		int dx( 8 * ( e->delta() > 0 ? 1 : -1 ) );
		QSize s(iconSize() + QSize(dx, dx));
		if (s.width() >= 8 && s.width() <= 128)
			setIconSize(s);
	}
	else
		QListView::wheelEvent(e);
}


void DirectoryListView::viewerResizedAction(const QSize& /*s*/)
{
	if (viewer->isVisible())
		viewer->rescalePixmap();
}


