/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
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
#ifndef VIEWERPRESETSWIDGET_H
#define VIEWERPRESETSWIDGET_H

#include "ui_viewerpresetswidget.h"

#include "genomevector.h"
#include "qosmicwidget.h"
#include "viewerpresetsmodel.h"

class ViewerPresetsWidget : public QWidget, public QosmicWidget,
	private Ui::ViewerPresetsWidget
{
	Q_OBJECT

	GenomeVector* genomes;
	ViewerPresetsModel* model;

	public:
		ViewerPresetsWidget(GenomeVector*, QWidget* =0);
		QStringList presetNames();
		QString current();
		void selectPreset(const QString&);
		void selectPreset(int);
		int selectedIndex() const;
		void reset();

	public slots:
		void addPresetSlot();
		void delPresetSlot();
		void updatePresetSlot();
		void moveUpSlot();
		void moveDownSlot();
		void presetSelectedSlot(const QModelIndex&);

	signals:
		void presetSelected();
		void dataChanged();

	protected:
		void keyPressEvent(QKeyEvent*);
};

#endif
