/***************************************************************************
 *   Copyright (C) 2007, 2010 by David Bitseff                             *
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

#include "genomecolorselector.h"

GenomeColorSelector::GenomeColorSelector(QWidget* parent, GenomeVector* g)
	: QLabel(parent), genome_offset(0),  genome(g), last_y(0)
{
}

void GenomeColorSelector::mousePressEvent (QMouseEvent* e)
{
	last_y = 255 - e->pos().y();
	repaintLabel();
	emit colorSelected(last_y / 255.);
}

void GenomeColorSelector::mouseMoveEvent (QMouseEvent* e)
{
	if (e->buttons() & Qt::LeftButton
		   && e->y() >= 0
		   && e->y() < 256)
	{
		last_y = 255 - e->pos().y();
		repaintLabel();
		emit colorSelected(last_y / 255.);
	}
}

void GenomeColorSelector::mouseReleaseEvent (QMouseEvent* e)
{
	last_y = 255 - e->pos().y();
	repaintLabel();
	emit colorChanged(last_y / 255.);
	emit undoStateSignal();
}

#define genome_ptr (genome->selectedGenome())

void GenomeColorSelector::setGenome(int n)
{
	genome_offset = n;
	repaintLabel();
}

void GenomeColorSelector::setGenomeVector(GenomeVector* v)
{
	genome = v;
}

void GenomeColorSelector::repaintLabel()
{
	if (genome_ptr)
	{
		QSize s = maximumSize();
		QImage palette(s.width(), s.height(), QImage::Format_RGB32);
		QPainter p(&palette);

		for (int i = 0 ; i < 256 ; i++)
		{
			double rc, gc, bc;
			rc = genome_ptr->palette[i].color[0];
			gc = genome_ptr->palette[i].color[1];
			bc = genome_ptr->palette[i].color[2];
			if (rc < 0.0 || rc > 1.0 ||
				bc < 0.0 || bc > 1.0 ||
				gc < 0.0 || gc > 1.0)
				return;
			if (last_y - 1 == i) // roundoff voodoo
				p.setPen(QPen(QColor::fromRgbF(1.0 - rc, 1.0 - gc, 1.0 - bc)));
			else
				p.setPen(QPen(QColor::fromRgbF(rc, gc, bc)));
			p.drawLine(0, 255 - i, s.width(), 255 - i);
		}
		setPixmap(QPixmap::fromImage( palette ));
	}
}

void GenomeColorSelector::setSelectedIndex(int y)
{
	int this_y = 1 + y;
	if (last_y == this_y)
		return;
	last_y = this_y; // voodoo too
	repaintLabel();
}
