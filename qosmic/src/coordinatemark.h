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
#ifndef COORDINATEMARK_H
#define COORDINATEMARK_H

#include <QStyleOptionGraphicsItem>
#include <QGraphicsPolygonItem>
#include <QPainter>
#include <QPainterPath>

class CoordinateMark : public QGraphicsPolygonItem
{
	QPainterPath m_path;

	public:
		static const int RTTI = 887872;

		CoordinateMark(QGraphicsItem* parent =0);
		int type() const;
		QPointF center() const;
		void centerOn(const QPointF&);

	protected:
		void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* =0);
};

#endif
