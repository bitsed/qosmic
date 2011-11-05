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

#include <QImage>
#include <QPainter>

#include "checkersbrush.h"


CheckersBrush::CheckersBrush(int d) : QBrush()
{
	QImage img(d, d, QImage::Format_RGB32);
	QPainter p(&img);
	p.fillRect(0, 0, d, d, Qt::white);
	p.fillRect(0, 0, d/2, d/2, Qt::black);
	p.fillRect(d/2, d/2, d, d, Qt::black);
	setTexture(QPixmap::fromImage(img));
}
