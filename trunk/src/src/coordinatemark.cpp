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
#include "coordinatemark.h"


CoordinateMark::CoordinateMark(QGraphicsItem* parent)
	: QGraphicsPolygonItem(QRectF(0.,0.,10.,10.),parent)
{
	m_path.moveTo(4.0, 0.0);
	m_path.lineTo(4.0, 4.0);
	m_path.lineTo(0.0, 4.0);

	m_path.moveTo(0.0, 6.0);
	m_path.lineTo(4.0, 6.0);
	m_path.lineTo(4.0, 10.0);

	m_path.moveTo(6.0, 10.0);
	m_path.lineTo(6.0, 6.0);
	m_path.lineTo(10.0, 6.0);

	m_path.moveTo(10.0, 4.0);
	m_path.lineTo(6.0, 4.0);
	m_path.lineTo(6.0, 0.0);
}

void CoordinateMark::paint(QPainter* painter,
						    const QStyleOptionGraphicsItem* /*option*/,
							QWidget* /*widget*/)
{
	painter->setPen(pen());
	painter->drawPath(m_path);
}

int CoordinateMark::type() const
{
	return RTTI;
}

QPointF CoordinateMark::center() const
{
	return pos() + QPointF(5.,5.);
}

void CoordinateMark::centerOn(const QPointF& p)
{
	setPos(p + QPointF(-5.,-5.));
}

