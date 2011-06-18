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

#ifndef MAINPREVIEWWIDGET_H
#define MAINPREVIEWWIDGET_H

#include <QWidget>
#include <QShowEvent>
#include <QTimer>
#include <QMenu>

#include "ui_mainpreviewwidget.h"
#include "genomevector.h"
#include "qosmicwidget.h"

class MainPreviewWidget
	: public QWidget, public QosmicWidget, private Ui::MainPreviewWidget
{
	Q_OBJECT

	GenomeVector* genome;
	QMenu* popupMenu;
	QTimer* wheel_stopped_timer;
	bool wheel_moved;
	QPointF last_pos;
	QPointF start_pos;
	QSize last_size;
	QString null_preset;
	QString selected_preset;

	public:
		MainPreviewWidget(GenomeVector* g, QWidget* parent=0);
		void setPixmap(const QPixmap&);
		QSize getPreviewSize() const;
		void setPreviewMaximumSize(QSize);
		bool isPresetSelected() const;
		QString preset() const;

	signals:
		void previewResized(const QSize&);
		void previewMoved();
		void undoStateSignal();

	public slots:
		void popupMenuTriggeredSlot(QAction*);

	protected:
		void resizeEvent(QResizeEvent*);
		void showEvent(QShowEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void closeEvent(QCloseEvent*);
};


#endif
