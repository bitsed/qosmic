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
 ****************************************************************************/
#include <QBrush>
#include "posttriangle.h"
#include "logger.h"


PostTriangle::PostTriangle(FigureEditor* c, flam3_xform* x, BasisTriangle* b)
	: Triangle(c, x, b, -1)
{
	logFiner("PostTriangle::PostTriangle : constr");
	TriangleCoords tc = basisTriangle->getCoords(m_xform->post);
	setPoints(tc);
}

void PostTriangle::resetPosition()
{
	m_xform->post[0][0] = 1.0;
	m_xform->post[0][1] = 0.0;
	m_xform->post[1][0] = 0.0;
	m_xform->post[1][1] = 1.0;
	m_xform->post[2][0] = 0.0;
	m_xform->post[2][1] = 0.0;

	TriangleCoords tc = basisTriangle->getCoords(m_xform->post);
	setPoints(tc);
}

void PostTriangle::coordsToXForm()
{
	logFiner("PostTriangle::coordsToXForm : enter");
	basisTriangle->applyTransform(cList, m_xform->post);
}

int PostTriangle::type() const
{
    return RTTI;
}

void PostTriangle::basisScaledSlot()
{
	const QMatrix basis(basisTriangle->coordTransform());
	setTransform(QTransform(basis));
	TriangleCoords tc = basisTriangle->getCoords(m_xform->post);
	setPoints(tc);
	QBrush b(brush());
	b.setMatrix(basis);
	setBrush(b);
}


