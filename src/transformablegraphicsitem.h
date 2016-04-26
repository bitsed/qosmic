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
#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H

class TransformableGraphicsGuide;

/**
 * An interface that accepts a GraphicsGuide decorator.
 *
 * It's also necessary for the child class to extend the QGraphicsItem
 * interface as well since the main purpose of this class is to handle
 * reparenting of the GraphicsGuide instance.
 *
 */
class TransformableGraphicsItem
{
	protected:
		TransformableGraphicsGuide* m_guide;

	public:
		TransformableGraphicsItem();
		virtual void setGraphicsGuide(TransformableGraphicsGuide*);
		TransformableGraphicsGuide* graphicsGuide() const;
};

//--------------------------------------------------------------------------

#endif // TRANSFORMABLE_H
