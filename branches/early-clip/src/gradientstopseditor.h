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
#ifndef GRADIENTSTOPSEDITOR_H
#define GRADIENTSTOPSEDITOR_H

#include <QMenu>
#include <QWidget>
#include <QLinearGradient>

class GradientStopsEditor : public QWidget
{
	Q_OBJECT

	public:
		GradientStopsEditor(QWidget* parent=0);
		QGradientStops& getStops();
		void setStops(const QGradientStops&);
		void setArrowDirection(Qt::ArrowType);
		Qt::ArrowType arrowDirection() const;
		void setMenuEnabled(bool);
		bool menuEnabled() const;
		void resetStops();

	public slots:
		void addStop();
		void removeStop();
		void selectStopColor();
		void stopColorSelected(QColor);

	signals:
		void stopsChanged();

	protected:
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void paintEvent(QPaintEvent*);

	private:
		QMenu* popupMenu;
		QAction* addAction;
		QAction* delAction;
		QAction* colorAction;

		int moving_idx;
		bool menu_enabled;
		QList<int> selected_idx;
		QPoint moving_start;
		QPoint moving_global;
		QGradientStops stops;
		QLinearGradient gradient;
		Qt::ArrowType arrow_type;
};


#endif
