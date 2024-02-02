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

		enum Edge { NONE, LEFT, RIGHT };

		MutationPreviewWidget(QWidget* parent=0);
		void setGenome(flam3_genome*);
		void setFrameColor(const QColor&);
		QColor frameColor() const;
		void setFrameEdge(Edge);
		Edge frameEdge() const;
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
		void paintEvent(QPaintEvent*);

	private:
		flam3_genome* g;
		QPoint dragStartPosition;
		QColor frame_color;
		Edge frame_edge;

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
