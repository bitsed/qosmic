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

};

#endif
