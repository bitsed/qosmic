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
#include "basistriangle.h"
#include "xfedit.h"


BasisTriangle::BasisTriangle(QMatrix t, QPolygonF p) :
		coordinateTransform(t), canvas(0)
{
	if (p.isEmpty())
		p << QPointF(0.0,0.0) << QPointF(1.0,0.0) << QPointF(0.0,1.0);
	setPoints(p);
	setZValue(0);
	setPen( QPen(Qt::gray) );
	setTransform(QTransform(t));
}

BasisTriangle::~BasisTriangle()
{
}

void BasisTriangle::setGraphicsScene(FigureEditor* e)
{
	canvas = e;
}

void BasisTriangle::scale(double dx, double dy)
{
	coordinateTransform.scale(dx,dy);
	setTransform(QTransform(coordinateTransform));
}

const QMatrix& BasisTriangle::coordTransform() const
{
	return coordinateTransform;
}

void BasisTriangle::setCoordTransform(QMatrix t)
{
	coordinateTransform = t;
	setTransform(QTransform(coordinateTransform));
}

int BasisTriangle::type() const
{
	return RTTI;
}


void BasisTriangle::setPoints(QPolygonF pa)
{
	setPoints( pa[0], pa[1], pa[2] );
}

void BasisTriangle::setPoints(QPointF a, QPointF b, QPointF c)
{
	A = a; B = b; C = c;
	QPolygonF pa;
	pa << A << B << C;
	QGraphicsPolygonItem::setPolygon(pa);
}


void BasisTriangle::applyTransform(TriangleCoords& cList, double coords[3][2])
{
	double Ax = cList[0].x(); double Bx = cList[1].x(); double Cx = cList[2].x();
	double Ay = cList[0].y(); double By = cList[1].y(); double Cy = cList[2].y();

	double bAx = A.x(); double bAy = A.y();
	double bBx = B.x(); double bBy = B.y();
	double bCx = C.x(); double bCy = C.y();

	double a = (Ax*bBy - Ax*bCy - bAy*Bx + bCy*Bx + bAy*Cx - bBy*Cx) /
				((-bAy)*bBx + bAx*bBy + bAy*bCx - bBy*bCx - bAx*bCy + bBx*bCy );

	double b = ((-Ax)*bBx + Ax*bCx + bAx*Bx - bCx*Bx - bAx*Cx + bBx*Cx) /
				((-bAy)*bBx + bAx*bBy + bAy*bCx - bBy*bCx - bAx*bCy + bBx*bCy);

	double c = ((-Ax)*bBy*bCx + Ax*bBx*bCy + bAy*bCx*Bx - bAx*bCy*Bx - bAy*bBx*Cx + bAx*bBy*Cx) /
				((-bAy)*bBx + bAx*bBy + bAy*bCx - bBy*bCx - bAx*bCy + bBx*bCy);

	double d = (Ay*bBy - Ay*bCy - bAy*By + bCy*By + bAy*Cy - bBy*Cy) /
				((-bAy)*bBx + bAx*bBy + bAy*bCx - bBy*bCx - bAx*bCy + bBx*bCy );

	double e = ((-Ay)*bBx + Ay*bCx + bAx*By - bCx*By - bAx*Cy + bBx*Cy) /
				((-bAy)*bBx + bAx*bBy + bAy*bCx - bBy*bCx - bAx*bCy + bBx*bCy);

	double f = ((-Ay)*bBy*bCx + Ay*bBx*bCy + bAy*bCx*By - bAx*bCy*By - bAy*bBx*Cy + bAx*bBy*Cy) /
				((-bAy)*bBx + bAx*bBy + bAy*bCx - bBy*bCx - bAx*bCy + bBx*bCy);

	coords[0][0] = a;
	coords[0][1] = -d;
	coords[1][0] = -b;
	coords[1][1] = e;
	coords[2][0] = c;
	coords[2][1] = -f;
}

TriangleCoords BasisTriangle::getCoords(double coords[3][2])
{
	double a = coords[0][0];
	double d = -coords[0][1];
	double b = -coords[1][0];
	double e = coords[1][1];
	double c = coords[2][0];
	double f = -coords[2][1];
	double Nx, Ny;
	QPolygonF cList;

	Nx = (A.x()*a + A.y()*b + c);
	Ny = (A.x()*d + A.y()*e + f);
	cList << QPointF(Nx, Ny);

	Nx = (B.x()*a + B.y()*b + c);
	Ny = (B.x()*d + B.y()*e + f);
	cList << QPointF(Nx, Ny);

	Nx = (C.x()*a + C.y()*b + c);
	Ny = (C.x()*d + C.y()*e + f);
	cList << QPointF(Nx, Ny);

	return cList;
}

