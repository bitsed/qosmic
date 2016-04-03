/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
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

#ifndef TRANSFORMABLEGRAPHICSGUIDE_H
#define TRANSFORMABLEGRAPHICSGUIDE_H

#include <QGraphicsObject>
#include <QGraphicsRectItem>

#include "xfedit.h"

class GraphicsGuideScaleButton : public QGraphicsRectItem
{
	public:
		static const int RTTI = 148198;

		enum Location { TopRight, TopLeft, BottomRight, BottomLeft };

		Location location;
		QPen std_pen;

		GraphicsGuideScaleButton(QGraphicsItem* =0);
		int type() const;

	protected:
		void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
};


class TransformableGraphicsGuide : public QGraphicsObject
{
	Q_OBJECT

	FigureEditor* editor;
	QRectF outerRect;
	QPolygonF hFlipPoly;
	QPolygonF vFlipPoly;
	GraphicsGuideScaleButton topLeftRect;
	GraphicsGuideScaleButton topRightRect;
	GraphicsGuideScaleButton bottomLeftRect;
	GraphicsGuideScaleButton bottomRightRect;
	GraphicsGuideScaleButton bottomRect;
	GraphicsGuideScaleButton topRect;
	GraphicsGuideScaleButton leftRect;
	GraphicsGuideScaleButton rightRect;

	public:
		explicit TransformableGraphicsGuide(FigureEditor*, QGraphicsItem* =0);
		QRectF boundingRect() const;
		void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* =0);
		void setParentItem(QGraphicsItem*);
		void setVisible(bool);
		void update();
		QPainterPath shape() const;
};


#endif // TRANSFORMABLEGRAPHICSGUIDE_H
