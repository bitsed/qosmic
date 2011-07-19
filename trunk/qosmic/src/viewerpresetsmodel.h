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
#ifndef VIEWERPRESETSMODEL_H
#define VIEWERPRESETSMODEL_H

#include <QAbstractListModel>
#include <QMap>

#include "flam3util.h"

class ViewerPresetsModel : public QAbstractListModel
{
	Q_OBJECT

	QMap<QString, flam3_genome> presets;
	QStringList names;
	ViewerPresetsModel();
	static ViewerPresetsModel* singleInstance;

	public:
		static ViewerPresetsModel* getInstance();
		QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const;
		int rowCount(const QModelIndex& parent=QModelIndex() ) const;
		Qt::ItemFlags flags(const QModelIndex& index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
		bool setData(const QModelIndex& index, const QVariant& value, int role);
		QStringList presetNames();
		void applyPreset(const QString&, flam3_genome*);
		flam3_genome preset(const QString&);

	public slots:
		void addPresetSlot(const QString& name, flam3_genome* genome);
		void delPresetSlot(int idx);
		void updatePresetSlot(int idx, const QString& name, flam3_genome* genome);
		void moveUpSlot(int idx);
		void moveDownSlot(int idx);


	protected:
		void loadPresets();
		void savePresets();
		void loadDefaultPresets();
};


#endif
