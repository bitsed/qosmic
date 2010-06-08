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


#ifndef MAINVIEWER_H
#define MAINVIEWER_H

#include <QPixmap>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QMenu>

#include "ui_mainviewer.h"
#include "qosmicwidget.h"
#include "renderthread.h"

class MainViewer : public QWidget, public QosmicWidget,
	private Ui::MainViewer
{
	Q_OBJECT

	QGraphicsScene m_scene;
	QPixmap m_pix;
	QSize m_orig_size;
	QSize m_scaled_size;
	QSize last_M;
	bool m_resizedself;
	double m_scale;
	QGraphicsPixmapItem* m_pitem;
	QTimer* timer;
	QMenu* popupMenu;
	QString selected_preset;
	QGraphicsTextItem* m_titem;
	QGraphicsRectItem* m_ritem;
	QAction* status_action;
	QString nullPresetText;
	bool show_status;

	public:
		MainViewer(QWidget* parent=0, QString title=QString("MainViewer"));
		~MainViewer();
		QSize getViewerSize();
		QPixmap pixmap();
		void setPixmap(QPixmap& p, bool resized=true);
		void setPixmap(const QPixmap& p, bool resized=true);
		void rescalePixmap();
		void rescaleViewer();
		void mousePressEvent(QMouseEvent*);
		bool isDockWidget();
		bool isPresetSelected();
		QString preset();

	public slots:
		void setRenderStatus(RenderStatus*);
		void popupMenuTriggeredSlot(QAction*);
		void closeWindowAction();
		void scaleDownAction();
		void scaleUpAction();
		void scaleLastAction();
		void scaleResetAction();
		void saveImageAction();

	signals:
		void viewerResized(const QSize&);
		void viewerHidden();

	protected:
		void resizeEvent(QResizeEvent*);
		void showEvent(QShowEvent*);
		void hideEvent(QHideEvent*);

	protected slots:
		void checkResized();
};


#endif
