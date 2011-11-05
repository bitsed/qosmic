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

#include <QPainter>
#include <QSize>
#include <QImage>
#include <QPixmap>
#include <QBrush>

#include "colorlabel.h"
#include "colordialog.h"
#include "checkersbrush.h"

ColorLabel::ColorLabel(QWidget* parent)
	: QLabel(parent), lastcolor(Qt::black)
{
}

void ColorLabel::mousePressEvent(QMouseEvent* /*e*/)
{
	static QColor c;
	if (objectName() == "m_backgroundLabel")
	{
		ColorDialog d(this);
		d.setSelectedColor(lastcolor);
		d.setAlphaEnabled(false);
		connect(&d, SIGNAL(colorSelected(QColor)),
				 this, SIGNAL(colorSelected(QColor)));
		if (d.exec() == QDialog::Accepted)
		{
			c = d.getSelectedColor();
			if (c.isValid() && lastcolor != c)
				setColor(c);
			emit undoStateSignal();
		}
		else if (d.getSelectedColor() != lastcolor)
			emit colorSelected(lastcolor);
		disconnect(&d);
	}
	else if (objectName() == "m_colorLabel")
	{

	}
}


void ColorLabel::setColor(QColor c)
{
	QSize s = maximumSize();
	QImage label(s.width(), s.height(), QImage::Format_RGB32);
	QPainter p(&label);
	if (c.alpha() < 255)
		p.fillRect(label.rect(), CheckersBrush(15));
	p.fillRect(label.rect(), QBrush(c));
	setPixmap(QPixmap::fromImage( label ));
	lastcolor = c;
}



