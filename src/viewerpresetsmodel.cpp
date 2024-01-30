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
#include <QSettings>

#include "viewerpresetsmodel.h"
#include "logger.h"

ViewerPresetsModel* ViewerPresetsModel::singleInstance = 0;

ViewerPresetsModel::ViewerPresetsModel() : QAbstractListModel()
{
	loadPresets();
	logFine(QString("ViewerPresetsModel::ViewerPresetsModel : %1 items in the model").arg(names.size()));
}

ViewerPresetsModel* ViewerPresetsModel::getInstance()
{
	if (ViewerPresetsModel::singleInstance == 0)
		singleInstance = new ViewerPresetsModel();
	return singleInstance;
}

int ViewerPresetsModel::rowCount(const QModelIndex& /*parent*/) const
{
	return names.size();
}

QVariant ViewerPresetsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= names.size())
		return QVariant();

	if (role == Qt::DisplayRole)
		return names.at(index.row());
	else
		return QVariant();
}

Qt::ItemFlags ViewerPresetsModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool ViewerPresetsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid() && role == Qt::EditRole)
	{
		int row = index.row();
		QString name( value.toString() );
		flam3_genome g = presets.value( names.at( row ) );
		updatePresetSlot(row, name, &g);
		return true;
	}
	return false;
}

QVariant ViewerPresetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
		return tr("Column %1").arg(section);
	else
		return tr("Row %1").arg(section);
}

void ViewerPresetsModel::addPresetSlot(const QString& name, flam3_genome* genome)
{
	presets.insert(name, *genome);
	names.append(name);
	savePresets();
	emit dataChanged(index(0), index(names.size() - 1));
}

void ViewerPresetsModel::delPresetSlot(int idx)
{
	if (idx < 0 || idx >= names.size())
		return;
	QString name( names.takeAt(idx) );
	presets.remove(name);
	savePresets();
	emit dataChanged(index(idx - 1), index(idx));
}

void ViewerPresetsModel::updatePresetSlot(int idx, const QString& name, flam3_genome* genome)
{
	if (idx < 0 || idx >= names.size())
		return;
	QString oldname( names.at(idx) );
	presets.remove(oldname);
	names[idx] = name;
	presets[name] = *genome;
	savePresets();
	emit dataChanged(index(idx), index(idx));
}

void ViewerPresetsModel::moveUpSlot(int row)
{
	if (row <= 0 || row >= names.size())
		return;
	names.swapItemsAt(row - 1, row);
	savePresets();
	emit dataChanged(index(0), index(names.size() - 1));
}

void ViewerPresetsModel::moveDownSlot(int row)
{
	if (row < 0 || row >= names.size() - 1)
		return;
	names.swapItemsAt(row, row + 1);
	savePresets();
	emit dataChanged(index(0), index(names.size() - 1));
}

QStringList ViewerPresetsModel::presetNames()
{
	return names;
}

flam3_genome ViewerPresetsModel::preset(const QString& preset)
{
	if (presets.contains(preset))
		return presets[preset];
	return presets.values().first();
}

void ViewerPresetsModel::applyPreset(const QString& name, flam3_genome* current)
{
	flam3_genome g = preset(name);

	// this tries to adjust the brightness/nbatches ratio even
	// thought people shouldn't ever use more than 1 nbatches
	current->brightness *= (double)current->nbatches / g.nbatches;

	current->sample_density =            g.sample_density;
	current->spatial_filter_radius =     g.spatial_filter_radius;
	current->spatial_filter_select =     g.spatial_filter_select;
	current->spatial_oversample =        g.spatial_oversample;
	current->nbatches =                  g.nbatches;
	current->ntemporal_samples =         g.ntemporal_samples;
	current->estimator =                 g.estimator;
	current->estimator_curve =           g.estimator_curve;
	current->estimator_minimum =         g.estimator_minimum;

}

void ViewerPresetsModel::loadPresets()
{
	QSettings settings;
	int ncps = 0;
	int size = settings.beginReadArray("presets");
	if (size == 0)
		loadDefaultPresets();
	else
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex(i);
			QString name = settings.value("name").toString();
			QString xml  = settings.value("genome").toString();
			presets.insert(name, *Util::read_xml_string(xml, &ncps));
			names.append(name);
		}
	settings.endArray();
}

void ViewerPresetsModel::savePresets()
{
	QSettings settings;
	settings.remove("presets");
	settings.beginWriteArray("presets");
	for (int n = 0 ; n < names.size() ; n++)
	{
		settings.setArrayIndex(n);
		QString name(names[n]);
		flam3_genome g = presets[name];
		g.num_xforms = 0;
		QString c(flam3_print_to_string(&g));
		// keep only the flame tag
		c.remove(c.indexOf('>') + 1, c.length());
		c.append("</flame>\n");
		settings.setValue("name", name);
		settings.setValue("genome", c);
	}
	settings.endArray();
}

void ViewerPresetsModel::loadDefaultPresets()
{
	QSettings settings;
	int idx = 0;
	settings.remove("presets");
	settings.beginWriteArray("presets");

	settings.setArrayIndex(idx++);
	settings.setValue("name", tr("very-low quality"));
	settings.setValue("genome", QString("<flame time=\"0\" size=\"1280 960\" "
					"center=\"0 0\" scale=\"200\" rotate=\"0\" supersample=\"1\" "
					"filter=\"0\" filter_shape=\"gaussian\" quality=\"5\" "
					"passes=\"1\" temporal_samples=\"1\" background=\"0 0 0\" "
					"brightness=\"4\" gamma=\"4.0\" vibrancy=\"1.0\" "
					"estimator_radius=\"0\" estimator_minimum=\"0\" "
					"estimator_curve=\"0.4\" gamma_threshold=\"0.01\"></flame>\n"));

	settings.setArrayIndex(idx++);
	settings.setValue("name", tr("low quality"));
	settings.setValue("genome", QString("<flame time=\"0\" size=\"1280 960\" "
					"center=\"0 0\" scale=\"200\" rotate=\"0\" supersample=\"1\" "
					"filter=\"0\" filter_shape=\"gaussian\" quality=\"40\" "
					"passes=\"1\" temporal_samples=\"1\" background=\"0 0 0\" "
					"brightness=\"4\" gamma=\"4.0\" vibrancy=\"1.0\" "
					"estimator_radius=\"0\" estimator_minimum=\"0\" "
					"estimator_curve=\"0.4\" gamma_threshold=\"0.01\"></flame>\n"));

	settings.setArrayIndex(idx++);
	settings.setValue("name", tr("med-low quality"));
	settings.setValue("genome", QString("<flame time=\"0\" size=\"1280 960\" "
					"center=\"0 0\" scale=\"200\" rotate=\"0\" supersample=\"1\" "
					"filter=\"0.8\" filter_shape=\"gaussian\" quality=\"80\" "
					"passes=\"1\" temporal_samples=\"1\" background=\"0 0 0\" "
					"brightness=\"4.0\" gamma=\"4.0\" vibrancy=\"1\" "
					"estimator_radius=\"4\" estimator_minimum=\"0\" "
					"estimator_curve=\"0.4\" gamma_threshold=\"0.01\"></flame>\n"));

	settings.setArrayIndex(idx++);
	settings.setValue("name", tr("med quality"));
	settings.setValue("genome", QString("<flame time=\"0\" size=\"1280 960\" "
					"center=\"0 0\" scale=\"200\" rotate=\"0\" supersample=\"1\" "
					"filter=\"0.8\" filter_shape=\"gaussian\" quality=\"200\" "
					"passes=\"1\" temporal_samples=\"1\" background=\"0 0 0\" "
					"brightness=\"4.0\" gamma=\"4.0\" vibrancy=\"1\" "
					"estimator_radius=\"4\" estimator_minimum=\"0\" "
					"estimator_curve=\"0.4\" gamma_threshold=\"0.01\"></flame>\n"));

	settings.setArrayIndex(idx++);
	settings.setValue("name", tr("med-high quality"));
	settings.setValue("genome", QString("<flame time=\"0\" size=\"1280 960\" "
					"center=\"0 0\" scale=\"200\" rotate=\"0\" supersample=\"1\" "
					"filter=\"0.8\" filter_shape=\"gaussian\" quality=\"500\" "
					"passes=\"1\" temporal_samples=\"1\" background=\"0 0 0\" "
					"brightness=\"4.0\" gamma=\"4.0\" vibrancy=\"1.0\" "
					"estimator_radius=\"9\" estimator_minimum=\"0\" "
					"estimator_curve=\"0.4\" gamma_threshold=\"0.01\"></flame>\n"));


	settings.setArrayIndex(idx++);
	settings.setValue("name", tr("high quality"));
	settings.setValue("genome", QString("<flame time=\"0\" size=\"1280 960\" "
					"center=\"0 0\" scale=\"200\" rotate=\"0\" supersample=\"3\" "
					"filter=\"0.8\" filter_shape=\"gaussian\" quality=\"500\" "
					"passes=\"1\" temporal_samples=\"1\" background=\"0 0 0\" "
					"brightness=\"4.0\" gamma=\"4.0\" vibrancy=\"1.0\" "
					"estimator_radius=\"11\" estimator_minimum=\"0\" "
					"estimator_curve=\"0.6\" gamma_threshold=\"0.01\"></flame>\n"));

	settings.endArray();
	loadPresets();
}
