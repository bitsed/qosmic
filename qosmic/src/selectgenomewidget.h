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
#include "qosmicwidget.h"

class SelectGenomeWidget : public QWidget, public QosmicWidget,
	private Ui::SelectGenomeWidget
{
	Q_OBJECT

	public:
		SelectGenomeWidget(GenomeVector*, QWidget* parent=0);
		void setSelectedGenome(int);
		void reset();

	signals:
		void genomeSelected(int);
		void genomesModified();

	protected slots:
		void addButtonPressedSlot();
		void delButtonPressedSlot();
		void configButtonPressedSlot();
		void clearTrianglesButtonPressedSlot();
		void listViewClickedAction(const QModelIndex& idx);

	protected:
		void showEvent(QShowEvent*);
		void hideEvent(QHideEvent*);

	private:
		GenomeVector* genomes;
};


#include "ui_selectgenomeconfigdialog.h"

class SelectGenomeConfigDialog : public QDialog, private Ui::SelectGenomeConfigDialog
{
	Q_OBJECT

	public:
		SelectGenomeConfigDialog(QWidget* parent=0);
		void setPreviewSize(const QSize& size);
		QSize previewSize() const;
		void setPreset(const QString &s);
		QString preset() const;
		void setAutoSave(const GenomeVector::AutoSave);
		GenomeVector::AutoSave autoSave() const;
};

#endif
