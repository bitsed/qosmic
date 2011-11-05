/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2011 by David Bitseff                 *
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
#ifndef NODEITEM_H
#define NODEITEM_H

#include <QGraphicsScene>
#include <QGraphicsEllipseItem>

#include "triangle.h"

class NodeItem : public QGraphicsEllipseItem
{
	QGraphicsScene* canvas;
	Triangle* edges;
	QString m_name;
	QRectF m_boundingRect;
	QGraphicsSimpleTextItem* label;
	QPen std_pen;
	int m_index;

	public:
		NodeItem( QGraphicsScene*, Triangle*, int, const char* n = "" );
		~NodeItem();
		Triangle* triangle();
		void setTriangle(Triangle*);
		void moveBy(double, double);
		void movePoint(double, double);
		int type() const;
		void setPos(const QPointF&);
		void setPen(const QPen&, const QPen&);
		QRectF boundingRect();
		QString name() const;
		int id() const;
		void setVisible(bool);

	protected:
		void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

	public:
		static const int RTTI = 349497;
};



#endif
