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

#include "transformablegraphicsitem.h"
#include "transformablegraphicsguide.h"

TransformableGraphicsItem::TransformableGraphicsItem() : m_guide(0)
{
}

void TransformableGraphicsItem::setGraphicsGuide(TransformableGraphicsGuide* guide)
{
	m_guide = guide;
	if (guide)
		guide->setParentItem(dynamic_cast<QGraphicsItem*>(this));
}

TransformableGraphicsGuide* TransformableGraphicsItem::graphicsGuide() const
{
	return m_guide;
}
