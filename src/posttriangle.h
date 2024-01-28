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
 ***************************************************************************/
#ifndef POSTTRIANGLE_H
#define POSTTRIANGLE_H


#include "triangle.h"

class PostTriangle : public Triangle
{

	public:
		PostTriangle(FigureEditor* c, flam3_xform* x, BasisTriangle* b);
		void resetPosition();
		int type() const;
		void basisScaledSlot();
		void coordsToXForm();

		enum { Type = UserType + 4 };

};

#endif

