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
#ifndef VIEWERPRESETSWIDGET_H
#define VIEWERRESETSWIDGET_H

#include <QMap>
#include <QListWidgetItem>

#include "ui_viewerpresetswidget.h"
#include "genomevector.h"
#include "qosmicwidget.h"

class ViewerPresetsWidget
	: public QWidget, public QosmicWidget, private Ui::ViewerPresetsWidget
{
	Q_OBJECT

	GenomeVector* genomes;
	QMap<QString, flam3_genome> presets;

	public:
		ViewerPresetsWidget(GenomeVector*, QWidget* =0);
		QStringList presetNames();
		QString current();
		void selectedPreset(QString);
		void applyPreset(QString, flam3_genome*);
		flam3_genome preset(QString);
		void reset();

	public slots:
		void addPresetSlot();
		void delPresetSlot();
		void updatePresetSlot();
		void moveUpSlot();
		void moveDownSlot();
		void presetSelectedSlot(QListWidgetItem*);
		void presetDoubleClickedSlot(QListWidgetItem*);

	signals:
		void dataChanged();

	protected:
		void loadPresets();
		void savePresets();
		void loadDefaultPresets();
};

#endif
