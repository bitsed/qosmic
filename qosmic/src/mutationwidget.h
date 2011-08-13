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

#ifndef MUTATIONWIDGET_H
#define MUTATIONWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QLabel>
#include <QMouseEvent>


#include "genomevector.h"
#include "renderthread.h"
#include "flam3util.h"


class MutationPreviewWidget;
class MutationPreviewWidget : public QLabel
{
	Q_OBJECT

	public:
		MutationPreviewWidget(QWidget* parent=0);
		MutationPreviewWidget(flam3_genome*, QWidget* parent=0);
		void setGenome(flam3_genome*);
		flam3_genome* genome();


	signals:
		void previewClicked(MutationPreviewWidget*, QMouseEvent*);
		void mutationASelected(MutationPreviewWidget*);
		void mutationBSelected(MutationPreviewWidget*);
		void genomeSelected(MutationPreviewWidget*);
		void genomeDropped(int);

	protected:
		void dragEnterEvent(QDragEnterEvent*);
		void dropEvent(QDropEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);

	private:
		flam3_genome* g;
		QPoint dragStartPosition;

};


#include "ui_mutationwidget.h"

class MutationWidget : public QWidget, private Ui::MutationWidget
{
	Q_OBJECT

	public:
		MutationWidget(GenomeVector*, RenderThread*, QWidget* parent=0);

	public slots:
		void mutationASelectedAction(MutationPreviewWidget*);
		void mutationBSelectedAction(MutationPreviewWidget*);
		void genomeSelectedAction(MutationPreviewWidget*);
		void reset();

	protected:
		void showEvent(QShowEvent*);
		void cancelRequests();
		void mutateAB(char);
		void cross();

	protected slots:
		void flameRenderedAction(RenderEvent*);
		void selectorAIndexChangedSlot(int);
		void selectorBIndexChangedSlot(int);
		void rotateAMutationsUp();
		void rotateAMutationsDown();
		void rotateBMutationsUp();
		void rotateBMutationsDown();
		void showConfigDialog();
		void mutate();

	signals:
		void genomeSelected(flam3_genome*);

	private:
		int genome_offset;
		QSize labels_size;
		qreal mutation_speed;
		int mutateA_start;
		int mutateB_start;
		QString quality_preset;
		GenomeVector* genome;
		RenderThread* rthread;
		QList<MutationPreviewWidget*> labels;
		QList<flam3_genome*> mutations;
		QList<RenderRequest*> requests;
};

#include "ui_mutationconfigdialog.h"

class MutationConfigDialog : public QDialog, private Ui::MutationConfigDialog
{
	Q_OBJECT

	public:
		MutationConfigDialog(QWidget* parent=0);
		void setPreviewSize(const QSize& size);
		QSize previewSize() const;
		void setSpeed(const qreal);
		qreal speed() const;
		void setPreset(const QString &s);
		QString preset() const;
};

#endif
