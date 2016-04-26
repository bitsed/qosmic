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

#include "nodeitem.h"
#include "logger.h"


NodeItem::NodeItem( QGraphicsScene* c, Triangle* t, int id, const char* n )
	: canvas(c), edges(t), m_name(n), m_index(id)
{
	setRect(-4., -4., 8., 8.);
	setZValue( 0 );
	QGraphicsEllipseItem::setPen(QPen(Qt::gray));
	m_boundingRect = QRectF(-10.,-10.,20.,20.);
	label = new QGraphicsSimpleTextItem(m_name);
	label->setBrush(QBrush(Qt::gray));
	label->setZValue(0);
	label->moveBy(-15., 3.);
	canvas->addItem(this);
	canvas->addItem(label);
	setAcceptHoverEvents(true);
}

NodeItem::~NodeItem()
{
	canvas->removeItem(label);
	delete label;
}

QRectF NodeItem::boundingRect()
{
	return m_boundingRect;
}

void NodeItem::setTriangle(Triangle* e)
{
	edges = e;
}

Triangle* NodeItem::triangle()
{
	return edges;
}

void NodeItem::setPos(const QPointF& p)
{
	QGraphicsEllipseItem::setPos(p);
	label->setPos(p);
	label->moveBy(-15., 3.);
}

void NodeItem::setPen(const QPen& ppen, const QPen& lpen)
{
	QGraphicsEllipseItem::setPen(ppen);
	label->setBrush(lpen.color());
}

/**
 * This one is called by its Triangle
 */
void NodeItem::movePoint(double dx, double dy)
{
	logFinest(QString("NodeItem::movePoint : %1 (%2, %3)")
			.arg(m_name).arg(dx).arg(dy));
	QGraphicsEllipseItem::moveBy(dx, dy);
	label->moveBy(dx, dy);
}

/**
 * This one is called by the FigureEditor
 */
void NodeItem::moveBy(double dx, double dy)
{
	logFinest(QString("NodeItem::moveBy : %1 (%2, %3)")
			.arg(m_name).arg(dx).arg(dy));
	QGraphicsEllipseItem::moveBy( dx, dy );
	label->moveBy(dx, dy);
	edges->moveEdges();
}

int NodeItem::type() const
{
	return Type;
}

QString NodeItem::name() const
{
	return m_name;
}

int NodeItem::id() const
{
	return m_index;
}

void NodeItem::setVisible(bool flag)
{
	QGraphicsEllipseItem::setVisible(flag);
	label->setVisible(flag);
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	QPen p(pen());
	std_pen = p;
	p.setColor(p.color().lighter(200));
	QGraphicsEllipseItem::setPen(p);
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	QGraphicsEllipseItem::setPen(std_pen);
}
