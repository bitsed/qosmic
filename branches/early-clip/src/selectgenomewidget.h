/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009 by David Bitseff                       *
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
#ifndef SELECTGENOMEWIDGET_H
#define SELECTGENOMEWIDGET_H

#include <QWidget>

#include "ui_selectgenomewidget.h"
#include "genomevector.h"
#include "renderthread.h"
#include "previewwidget.h"
#include "qosmicwidget.h"

class SelectGenomeWidget : public QWidget, public QosmicWidget,
	private Ui::SelectGenomeWidget
{
	Q_OBJECT

	public:
		SelectGenomeWidget(GenomeVector*, RenderThread*, QWidget* parent=0);
		void setSelectedGenome(int);
		void updateFormData();
		void reset();

	public slots:
		void flameRenderedAction(RenderEvent*);

	signals:
		void genomeSelected(int);
		void genomesModified();

	protected slots:
		void selectorIndexChangedSlot(int);
		void genomeSelectedAction(PreviewWidget*, QMouseEvent*);
		void addButtonPressedSlot();
		void delButtonPressedSlot();

	protected:
		void showEvent(QShowEvent*);
		void wheelEvent(QWheelEvent*);
		void closeEvent(QCloseEvent*);

	private:
		int last_idx;
		QSize label_size;
		QList<RenderRequest*> r_requests;
		QList<PreviewWidget*> labels;
		GenomeVector* genome;
		RenderThread* r_thread;

};

#endif
