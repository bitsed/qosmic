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
		void updateSelectedPreview();

	signals:
		void genomeSelected(int);
		void genomesModified();

	protected slots:
		void selectorIndexChangedSlot(int);
		void genomeSelectedAction(const QModelIndex& idx);
		void addButtonPressedSlot();
		void delButtonPressedSlot();
		void configButtonPressedSlot();
		void clearTrianglesButtonPressedSlot();
		void indexesMovedSlot(const QModelIndexList& idxList);

	protected:
		void showEvent(QShowEvent* event);
		void closeEvent(QCloseEvent* event);
		void clearPreviews();

	private:
		QSize label_size;
		QList<RenderRequest*> r_requests;
		GenomeVectorListModel* model;
		RenderThread* r_thread;


};


#include "ui_selectgenomeconfigdialog.h"

class SelectGenomeConfigDialog : public QDialog, private Ui::SelectGenomeConfigDialog
{
	Q_OBJECT

	public:
		SelectGenomeConfigDialog(QWidget* parent=0);
		void setPreviewSize(const QSize& size);
		QSize previewSize() const;
};

#endif
