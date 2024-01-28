/***************************************************************************
 *   Copyright (C) 2007-2024 by David Bitseff                              *
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
    return Type;
}

void PostTriangle::basisScaledSlot()
{
	const QTransform basis(basisTriangle->coordTransform());
	setTransform(basis);
	TriangleCoords tc = basisTriangle->getCoords(m_xform->post);
	setPoints(tc);
	QBrush b(brush());
	b.setTransform(basis);
	setBrush(b);
}


