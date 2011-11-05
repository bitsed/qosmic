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
#ifndef BASISTRIANGLE_H
#define BASISTRIANGLE_H

#include <QPolygonF>
#include <QList>
#include <QPointF>
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneWheelEvent>

typedef QPolygonF TriangleCoords;
typedef QList<QPointF>::iterator TriangleCoordsIterator;

class FigureEditor;

class BasisTriangle : public QGraphicsPolygonItem
{
	protected:
		QMatrix coordinateTransform;
		FigureEditor* canvas;

	public:
		QPointF A, B, C;
		static const int RTTI = 349493;

		BasisTriangle(QMatrix, QPolygonF p=QPolygonF());
		~BasisTriangle();
		int type() const;
		void setPoints(QPolygonF);
		void setPoints(QPointF, QPointF, QPointF);
		void setGraphicsScene(FigureEditor*);
		const QMatrix& coordTransform() const;
		void setCoordTransform(QMatrix);
		void scale(double, double);
		TriangleCoords getCoords(double c[3][2]);
		void applyTransform(TriangleCoords&, double[3][2]);
};


#endif
