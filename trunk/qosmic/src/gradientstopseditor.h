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
#ifndef GRADIENTSTOPSEDITOR_H
#define GRADIENTSTOPSEDITOR_H

#include <QMenu>
#include <QWidget>
#include <QGradientStop>
#include <QLinearGradient>

class GradientStop
{
	public:
		enum { RGB = 0, HSV = 1, HSVR = 2 } ColorSpace;
		double first;
		QColor second;
		int colorspace;
		int blending;

		GradientStop() : colorspace(0), blending(0) {}
//		GradientStop(GradientStop& stop)
//		: QGradientStop(stop.first, stop.second), colorspace(stop.colorspace), blending(stop.blending) {}
//		GradientStop(const QPair<double,QColor> stop)
//		: QGradientStop(stop), colorspace(0), blending(0) {}
		GradientStop(double pos, QColor color, int space=0, int blend=0)
		: first(pos), second(color), colorspace(space), blending(blend) {}

		int getColorSpace() { return colorspace ; }
		static bool lessThanGradientStopComparison(const GradientStop& s1, const GradientStop& s2);
};

typedef QVector<GradientStop> GradientStops;

class GradientStopsEditor : public QWidget
{
	Q_OBJECT

	public:
		GradientStopsEditor(QWidget* parent=0);
		GradientStops& getStops();
		void setStops(const GradientStops&);
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
		GradientStops stops;
		QLinearGradient gradient;
		Qt::ArrowType arrow_type;
};


#endif
