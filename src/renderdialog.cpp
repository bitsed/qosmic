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
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include "renderdialog.h"

RenderDialog::RenderDialog(QWidget* parent,
		QString file, QString lastPath, QSize seyz, QStringList list)
	: QDialog(parent),
	fileName(file), lastDir(lastPath), imgSize(seyz), presets(list),
	nopreset(tr("genome quality")),
	sizeValidator(QRegExp("\\d+\\s*x\\s*\\d+"), this)
{
	setupUi(this);
	setModal(true);

	QSettings settings;

	m_filePathLineEdit->setText(QDir(lastDir).absoluteFilePath(fileName));
	m_qualityComboBox->addItem(nopreset);
	m_qualityComboBox->addItems(presets);

	QString quality(settings.value("renderdialog/last_quality").toString());
	m_qualityComboBox->setCurrentIndex(m_qualityComboBox->findText(quality));

	int cnt = settings.beginReadArray("renderdialog/sizes");
	if (cnt == 0)
	{
		QStringList dims;
		dims << "640x480" << "600x800" << "1024x768" << "1280x960" << "1280x1024" << "1920x1080";
		m_sizeComboBox->addItems(dims);
	}
	else
	{
		for (int i = 0; i < cnt; ++i)
		{
			settings.setArrayIndex(i);
			QString dim( settings.value("dim").toString() );
			m_sizeComboBox->addItem(dim);
		}
		settings.endArray();
		sizeText = QString("%1x%2").arg(imgSize.width()).arg(imgSize.height());
		if (m_sizeComboBox->findText(sizeText) == -1)
		{
			m_sizeComboBox->insertItem(0, sizeText);
			m_sizeComboBox->setCurrentIndex(0);
		}
	}

	connect(m_filePathButton, SIGNAL(pressed()), this, SLOT(filePathButtonSlot()));
	connect(m_addSizeButton, SIGNAL(pressed()), this, SLOT(addSizeButtonSlot()));
	connect(m_delSizeButton, SIGNAL(pressed()), this, SLOT(delSizeButtonSlot()));
}


RenderDialog::~RenderDialog()
{
}

void RenderDialog::addSizeButtonSlot()
{
	m_sizeComboBox->setEditable(true);
	m_sizeComboBox->setValidator(&sizeValidator);
	m_sizeComboBox->clearEditText();
	m_sizeComboBox->setFocus(Qt::OtherFocusReason);
}

void RenderDialog::delSizeButtonSlot()
{
	m_sizeComboBox->removeItem(m_sizeComboBox->currentIndex());
}

void RenderDialog::accept()
{
	if (QFileInfo(absoluteFilePath()).exists())
	{
		QMessageBox::StandardButton b = QMessageBox::question(this,
			tr("File already exists"),
			tr("Do you want to overwrite %1?")
				.arg(QFileInfo(absoluteFilePath()).fileName()),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::NoButton );
		if (b == QMessageBox::No)
			return;
	}

	QSettings settings;
	size = m_sizeComboBox->currentText();
	preset = m_qualityComboBox->currentText();

	m_sizeComboBox->removeItem(m_sizeComboBox->currentIndex());

	settings.beginWriteArray("renderdialog/sizes");
	settings.setArrayIndex(0);
	settings.setValue("dim", size);

	int cnt = 10;
	for (int i = 0; i < cnt; ++i)
	{
		QString text(m_sizeComboBox->itemText(i));
		if (text.isEmpty())
			break;
		settings.setArrayIndex(i+1);
		settings.setValue("dim", text);
	}
	settings.endArray();
	settings.setValue("renderdialog/last_size", size);
	settings.setValue("renderdialog/last_quality", preset);

	QDialog::accept();
}

void RenderDialog::filePathButtonSlot()
{
	QString imageName = fileName;
	if (imageName.isEmpty())
		imageName = tr("untitled.png");

	imageName = QFileDialog::getSaveFileName(this,
		tr("Save an image as ..."),
		QDir(lastDir).absoluteFilePath(imageName));

	m_filePathLineEdit->setText(imageName);
}

QString RenderDialog::absoluteFilePath()
{
	return m_filePathLineEdit->text();
}


QString RenderDialog::selectedPreset()
{
	return preset;
}

bool RenderDialog::presetSelected()
{
	return preset != nopreset;
}

bool RenderDialog::sizeSelected()
{
	return size != sizeText;
}

QSize RenderDialog::selectedSize()
{
	QRegExp rx("(\\d+)\\s*x\\s*(\\d+)");
	if (rx.indexIn(size) != -1)
		return QSize(rx.cap(1).toInt(), rx.cap(2).toInt());
	return QSize(10,10);
}

