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
#include <QSettings>
#include <QInputDialog>

#include "viewerpresetswidget.h"

ViewerPresetsWidget::ViewerPresetsWidget(GenomeVector* g, QWidget* parent)
	: QWidget(parent), QosmicWidget(this, "ViewerPresetsWidget"), genomes(g)
{
	setupUi(this);
	loadPresets();

	connect(m_presetsList, SIGNAL(itemClicked(QListWidgetItem*)),
			this, SLOT(presetSelectedSlot(QListWidgetItem*)));
	connect(m_presetsList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
			this, SLOT(presetDoubleClickedSlot(QListWidgetItem*)));

	connect(m_addButton, SIGNAL(pressed()), this, SLOT(addPresetSlot()));
	connect(m_delButton, SIGNAL(pressed()), this, SLOT(delPresetSlot()));
	connect(m_updateButton, SIGNAL(pressed()), this, SLOT(updatePresetSlot()));
	connect(m_upButton, SIGNAL(pressed()), this, SLOT(moveUpSlot()));
	connect(m_downButton, SIGNAL(pressed()), this, SLOT(moveDownSlot()));
}

void ViewerPresetsWidget::addPresetSlot()
{
	bool ok;
	flam3_genome* g = genomes->selectedGenome();
	QString name = QString("q%1/o%2/e%3")
			.arg(g->sample_density)
			.arg(g->spatial_oversample)
			.arg(g->estimator);
	QString text = QInputDialog::getText(this, tr("Add a new preset"),
			tr("Enter a name for the preset"),
			  QLineEdit::Normal, name, &ok, Qt::Dialog);

     if (ok && !text.isEmpty())
	 {
		presets.insert(text, *g);
		m_presetsList->addItem(text);
		savePresets();
	 }
}

void ViewerPresetsWidget::delPresetSlot()
{
	QListWidgetItem* item = m_presetsList->currentItem();
	presets.remove(item->text());
	item = m_presetsList->takeItem(m_presetsList->row(item));
	delete item;
	savePresets();
}

void ViewerPresetsWidget::updatePresetSlot()
{
	flam3_genome* g = genomes->selectedGenome();
	QListWidgetItem* item = m_presetsList->currentItem();
	if (item)
	{
		presets[item->text()] = *g;
		savePresets();
		presetDoubleClickedSlot(item);
	}
}

void ViewerPresetsWidget::moveUpSlot()
{
	QListWidgetItem* item = m_presetsList->currentItem();
	int row = m_presetsList->row(item);
	if (row <= 0)
		return;
	item = m_presetsList->takeItem(row);
	m_presetsList->insertItem(row - 1, item);
	m_presetsList->setCurrentRow(row - 1);
	savePresets();
}

void ViewerPresetsWidget::moveDownSlot()
{
	QListWidgetItem* item = m_presetsList->currentItem();
	int row = m_presetsList->row(item);
	if (row < 0 || row >= m_presetsList->count())
		return;
	item = m_presetsList->takeItem(row);
	m_presetsList->insertItem(row + 1, item);
	m_presetsList->setCurrentRow(row + 1);
	savePresets();
}

QStringList ViewerPresetsWidget::presetNames()
{
	QStringList names;
	for (int n = 0 ; n < m_presetsList->count() ; n++)
		 names << m_presetsList->item(n)->text();

	return names;
}

void ViewerPresetsWidget::selectedPreset(QString preset)
{
	const QString name(preset);

	QListWidgetItem* item
		= m_presetsList->findItems(name, Qt::MatchFixedString).first();
	if (item)
	{
		m_presetsList->setCurrentItem(item);
		presetSelectedSlot(item);
	}
}

void ViewerPresetsWidget::presetDoubleClickedSlot(QListWidgetItem* item)
{
	bool ok;
	QString oldname = item->text();

	flam3_genome g = presets[oldname];

	QString text =
		QInputDialog::getText(this, tr("Rename preset"),
				tr("Enter a name for the preset"),
				QLineEdit::Normal, oldname, &ok, Qt::Dialog);
	if (ok && !text.isEmpty())
	{
		presets.remove(oldname);
		presets.insert(text, g);
		item->setText(text);
		savePresets();
	}

}

QString ViewerPresetsWidget::current()
{
	// return the name of the last selected preset
	QListWidgetItem* item = m_presetsList->currentItem();
	if (item)
		return item->text();
	else
		return QString();
}

void ViewerPresetsWidget::presetSelectedSlot(QListWidgetItem* item)
{
	flam3_genome* current = genomes->selectedGenome();
	applyPreset(item->text(), current);
	emit dataChanged();
}

flam3_genome ViewerPresetsWidget::preset(QString preset)
{
	if (presets.contains(preset))
		return presets[preset];
	return presets.values().last();
}

void ViewerPresetsWidget::applyPreset(QString name, flam3_genome* current)
{
	flam3_genome g = preset(name);

	// this tries to adjust the brightness/nbatches ratio even
	// thought people shouldn't ever use more than 1 nbatches
	current->brightness *= (double)current->nbatches / g.nbatches;

	current->sample_density =            g.sample_density;
	current->spatial_filter_radius =     g.spatial_filter_radius;
	current->spatial_oversample =        g.spatial_oversample;
	current->nbatches =                  g.nbatches;
	current->ntemporal_samples =         g.ntemporal_samples;
	current->estimator =                 g.estimator;
	current->estimator_curve =           g.estimator_curve;
	current->estimator_minimum =         g.estimator_minimum;

}

void ViewerPresetsWidget::loadPresets()
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
			m_presetsList->addItem(name);
		}
	settings.endArray();
}

void ViewerPresetsWidget::savePresets()
{
	QSettings settings;
	settings.remove("presets");
	settings.beginWriteArray("presets");
	for (int n = 0 ; n < m_presetsList->count() ; n++)
	{
		settings.setArrayIndex(n);
		QString name(m_presetsList->item(n)->text());
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

void ViewerPresetsWidget::loadDefaultPresets()
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
