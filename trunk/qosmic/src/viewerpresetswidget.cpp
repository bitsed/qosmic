/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2011 by David Bitseff                 *
 *   bitsed@gmail.com                                                      *
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

	model = ViewerPresetsModel::getInstance();
	m_presetsList->setModel(model);

	connect(model, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)), this, SIGNAL(dataChanged()));
	connect(m_presetsList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(presetSelectedSlot(const QModelIndex&)));

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
		model->addPresetSlot(text, g);
		m_presetsList->setCurrentIndex(model->index( model->rowCount() - 1 ));
	 }
}

void ViewerPresetsWidget::delPresetSlot()
{
	QModelIndex idx = m_presetsList->currentIndex();
	model->delPresetSlot(idx.row());
}

void ViewerPresetsWidget::updatePresetSlot()
{
	QModelIndex idx = m_presetsList->currentIndex();
	QString oldname = idx.data().toString();
	bool ok;

	QString text =
		QInputDialog::getText(this, tr("Rename preset"),
				tr("Enter a name for the preset"),
				QLineEdit::Normal, oldname, &ok, Qt::Dialog);
	if (ok && !text.isEmpty())
	{
		flam3_genome* g = genomes->selectedGenome();
		model->updatePresetSlot(idx.row(), idx.data().toString(), g);
	}
}

void ViewerPresetsWidget::moveUpSlot()
{
	QModelIndex idx = m_presetsList->currentIndex();
	int row = idx.row();
	model->moveUpSlot(row);
	m_presetsList->setCurrentIndex(model->index(row - 1));
}

void ViewerPresetsWidget::moveDownSlot()
{
	QModelIndex idx = m_presetsList->currentIndex();
	int row = idx.row();
	model->moveDownSlot(row);
	m_presetsList->setCurrentIndex(model->index(row + 1));
}

QStringList ViewerPresetsWidget::presetNames()
{
	return model->presetNames();
}

void ViewerPresetsWidget::selectPreset(const QString& name)
{
	QModelIndex item = model->match(model->index(0), Qt::DisplayRole, name, Qt::MatchFixedString).first();
	if (item.isValid())
	{
		m_presetsList->setCurrentIndex(item);
		presetSelectedSlot(item);
	}
}

void ViewerPresetsWidget::selectPreset(int idx)
{
	QModelIndex item = model->index(idx);

	if (item.isValid())
	{
		m_presetsList->setCurrentIndex(item);
		presetSelectedSlot(item);
	}
}

QString ViewerPresetsWidget::current()
{
	// return the name of the last selected preset
	QModelIndex item = m_presetsList->currentIndex();
	if (item.isValid())
		return item.data().toString();
	else
		return QString();
}

int ViewerPresetsWidget::selectedIndex() const
{
	return m_presetsList->currentIndex().row();
}

void ViewerPresetsWidget::presetSelectedSlot(const QModelIndex& item)
{
	flam3_genome* current = genomes->selectedGenome();
	model->applyPreset(item.data().toString(), current);
	emit presetSelected();

}

