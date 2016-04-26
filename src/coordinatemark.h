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
		enum { Type = UserType + 6 };

		CoordinateMark(QGraphicsItem* parent =0);
		int type() const;
		QPointF center() const;
		void centerOn(const QPointF&);

	protected:
		void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* =0);
};

#endif
