/***************************************************************************
 *   Copyright (C) 2007 by David Bitseff                                   *
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

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QLabel>
#include <QMouseEvent>

#include "flam3util.h"

class PreviewWidget;
class PreviewWidget : public QLabel
{
	Q_OBJECT

	public:
		PreviewWidget(QWidget* parent=0);
		PreviewWidget(flam3_genome*, QWidget* parent=0);
		void setGenome(flam3_genome*);
		flam3_genome* genome();

	signals:
		void previewClicked(PreviewWidget*, QMouseEvent*);
		void mutationSelected(PreviewWidget*);
		void genomeSelected(PreviewWidget*);

	protected:
		void mousePressEvent (QMouseEvent*);

	private:
		flam3_genome* g;

};


#endif
