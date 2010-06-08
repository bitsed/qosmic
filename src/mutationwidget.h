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

#ifndef MUTATIONWIDGET_H
#define MUTATIONWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QMenu>

#include "ui_mutationwidget.h"
#include "genomevector.h"

class PreviewWidget;
class RenderRequest;
class RenderEvent;
class RenderThread;

class MutationWidget : public QWidget, private Ui::MutationWidget
{
	Q_OBJECT

	public:
		MutationWidget(GenomeVector*, RenderThread*, QWidget* parent=0);
		void showEvent(QShowEvent*);
		void mutate(int);
		void reset();

	public slots:
		void mutationSelectedAction(PreviewWidget*);
		void genomeSelectedAction(PreviewWidget*);

	private slots:
		void flameRenderedAction(RenderEvent*);
		void speedChangedAction(int);
		void selectVariationsAction();
		void clearMenuAction();

	signals:
		void genomeSelected(flam3_genome*);

	private:
		int genome_offset;
		GenomeVector* genome;
		RenderThread* rthread;
		QList<PreviewWidget*> labels;
		QList<flam3_genome*> mutations;
		QList<QAction*> varsActions;
		QList<RenderRequest*> requests;
		double speed;
		QMenu* varsMenu;
		QAction* clearMenuAct;

};

#include "renderthread.h"
#include "previewwidget.h"

#endif
