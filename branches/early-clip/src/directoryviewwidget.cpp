/***************************************************************************
 *   Copyright (C) 2007, 2010 by David Bitseff                             *
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
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include "directoryviewwidget.h"
#include "logger.h"

DirectoryViewWidget::DirectoryViewWidget(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
	QSettings settings;
	int icon_size = settings.value("directoryview/iconsize", 48).toInt();
	path = settings.value("directoryview/lastdirectory", QDir::homePath()).toString();

	QStringList filters;
	filters << "*.flam3" << "*.flam" << "*.flame";
	model = new QFileSystemModel();
	model->setNameFilters(filters);
	model->setFilter(QDir::AllEntries | QDir::AllDirs);
	model->setNameFilterDisables(false);
	icon_provider = new FlamFileIconProvider;
	model->setIconProvider(icon_provider);

	m_dirListView->setIconSize(QSize(icon_size, icon_size));
	m_dirListView->setSpacing(2);
	m_dirListView->setModel(model);
	m_dirListView->setRootIndex(model->index(path));
	m_dirLineEdit->setText(path);
	model->setRootPath(path);

	connect(m_openButton, SIGNAL(clicked(bool)),
		this, SLOT(openDirectoryAction(bool)));
	connect(m_dirListView, SIGNAL(pressed(const QModelIndex&)),
		this, SLOT(selectFileAction(const QModelIndex&)));
	connect(m_dirLineEdit, SIGNAL(returnPressed()),
		this, SLOT(dirLineEditChangedAction()));
}


void DirectoryViewWidget::openDirectoryAction(bool /*t*/)
{
	QString openDir = QFileDialog::getExistingDirectory(this,
		tr("Select a directory"), path, QFileDialog::ShowDirsOnly);

	if (!openDir.isEmpty())
		setCurrentPath(openDir);
}

void DirectoryViewWidget::selectFileAction(const QModelIndex& idx)
{
	if (model->isDir(idx))
		setCurrentPath( model->fileInfo(idx).canonicalFilePath() );
	else
	{
		// Check for a contol modifier
		if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		{
			logInfo("DirectoryViewWidget::selectFileAction : file appended");
			emit flam3FileSelected(model->fileInfo(idx).absoluteFilePath(), true);
		}
		else
		{
			logInfo("DirectoryViewWidget::selectFileAction : file selected");
			emit flam3FileSelected(model->fileInfo(idx).absoluteFilePath());
		}
	}
}

DirectoryViewWidget::~DirectoryViewWidget()
{
	delete icon_provider;
	delete model;
}

void DirectoryViewWidget::setCurrentPath(QString p)
{
	if (p.isEmpty()) return;
	QModelIndex i = model->index(p);
	if (i.isValid() && model->isDir(i))
	{
		path = p;
		logFine(QString("DirectoryViewWidget::setCurrentPath : dir selected %1").arg(path));
		model->setRootPath(path);
		m_dirLineEdit->setText(path);
		m_dirListView->setRootIndex(i);
	}
}

QString DirectoryViewWidget::currentPath()
{
    return path;
}

void DirectoryViewWidget::closeEvent(QCloseEvent* /*e*/)
{
	logInfo("DirectoryViewWidget::closeEvent : saving settings");
	QSettings s;
	s.setValue("directoryview/lastdirectory", path);
	s.setValue("directoryview/iconsize", m_dirListView->iconSize().width());
}

void DirectoryViewWidget::dirLineEditChangedAction()
{
	QFileInfo file( m_dirLineEdit->text() );
	if (file.exists() && file.isDir())
	{
		if (file.isHidden())
			model->setFilter( QDir::AllEntries | QDir::AllDirs | QDir::Hidden );
		else
			model->setFilter( QDir::AllEntries | QDir::AllDirs );
		QString path( file.canonicalFilePath() );
		model->setRootPath(path);
		m_dirListView->setRootIndex(model->index(path));
	}
	else
		QMessageBox::information(this, tr("Invalid path"),
			tr("The directory path %1 doesn't exist.")
			.arg(file.absoluteFilePath()), QMessageBox::NoButton);
}
