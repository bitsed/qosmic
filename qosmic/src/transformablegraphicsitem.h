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
#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H

class TransformableGraphicsGuide;

/**
 * An interface that accepts a GuideAdapter.
 *
 * It's also necessary for the child class to extend the QGraphicsItem
 * interface as well since the main purpose of this class is to handle
 * reparenting of the guideAdapter instance.
 *
 */
class TransformableGraphicsItem
{
	protected:
		TransformableGraphicsGuide* m_adapter;

	public:
		TransformableGraphicsItem();
		virtual void setGuideAdapter(TransformableGraphicsGuide*);
		TransformableGraphicsGuide* guideAdapter() const;
};

//--------------------------------------------------------------------------

#endif // TRANSFORMABLE_H
