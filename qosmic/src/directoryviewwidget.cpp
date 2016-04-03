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
#include <QFileDialog>
#include <QSettings>

#include "directoryviewwidget.h"
#include "logger.h"

DirectoryViewWidget::DirectoryViewWidget(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
	QSettings s;
	s.beginGroup("directoryview");
	int icon_size = s.value("iconsize", 48).toInt();
	path = s.value("lastdirectory", QDir::homePath()).toString();
	histEntries = s.value("dirhistory", QStringList() << path).toStringList();
	sort_type  = (SortType)s.value("sorttype", NAME).toInt();
	sort_order = (Qt::SortOrder)s.value("sortorder", Qt::AscendingOrder).toInt();
	bool show_hidden = s.value("showhidden", false).toBool();
	view_type = (ViewType)s.value("viewtype", SHORT).toInt();

	QStringList filters;
	filters << "*.flam3" << "*.flam" << "*.flame" << "*.lua";
	model = new QFileSystemModel();
	model->setNameFilters(filters);
	model->setFilter(QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);
	model->setNameFilterDisables(false);
	iconProvider = new FlamFileIconProvider;
	model->setIconProvider(iconProvider);
	model->setRootPath(path);

	m_dirListView->setIconSize(QSize(icon_size, icon_size));
	m_dirListView->setSpacing(2);
	m_dirListView->setModel(model);
	m_dirListView->setRootIndex(model->index(path));

	m_treeView->setModel(model);
	m_treeView->setRootIndex(model->index(path));

	comboListModel = new QStringListModel(histEntries);
	m_dirComboBox->setModel(comboListModel);
	m_dirComboBox->setCurrentIndex(m_dirComboBox->findText(path));
	histEntries.clear();
	histEntries << path;
	currHistEntry = 0;
	m_backButton->setEnabled(currHistEntry > 0);
	m_forwardButton->setEnabled(currHistEntry < histEntries.size() - 1);

	connect(m_openButton, SIGNAL(clicked(bool)), this, SLOT(openDirectoryAction(bool)));
	connect(m_upButton, SIGNAL(clicked()), this, SLOT(upButtonClicked()));
	connect(m_forwardButton, SIGNAL(clicked()), this, SLOT(forwardButtonClicked()));
	connect(m_backButton, SIGNAL(clicked()), this, SLOT(backButtonClicked()));
	connect(m_dirComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(historyIndexChanged(const QString&)));
	connect(m_dirComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(historyIndexChanged(const QString&)));
	connect(m_dirListView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(selectFileAction(const QModelIndex&)));
	connect(m_treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(selectFileAction(const QModelIndex&)));
	connect(m_treeView->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(detailedViewSortTypeChanged(int,Qt::SortOrder)));
	connect(m_zoomInButton, SIGNAL(clicked()), this, SLOT(zoomInButtonClicked()));
	connect(m_zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOutButtonClicked()));
	connect(m_configButton, SIGNAL(clicked()), this, SLOT(configButtonClicked()));

	showHiddenFiles(show_hidden);
	sortBy(sort_type);
	setViewType(view_type);
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
	{
		QString path(model->fileInfo(idx).canonicalFilePath());
		setCurrentPath(path);
		updateHistEntries(path);

	}
	else
	{
		QFileInfo info(model->fileInfo(idx));
		QString suffix(info.suffix());
		if (suffix.contains(QRegExp("^flam[e3]?$")))
		{
			// Check for a contol modifier
			if (QApplication::keyboardModifiers() & Qt::ControlModifier)
			{
				logInfo("DirectoryViewWidget::selectFileAction : file appended");
				emit flam3FileAppended(info.absoluteFilePath());
			}
			else
			{
				logInfo("DirectoryViewWidget::selectFileAction : file selected");
				emit flam3FileSelected(info.absoluteFilePath());
			}
		}
		else if (suffix.contains(QRegExp("^lua$")))
		{
			logInfo("DirectoryViewWidget::selectFileAction : script selected");
			emit luaScriptSelected(info.absoluteFilePath());
		}
		else
			logWarn(QString("DirectoryViewWidget::openDirectoryAction : unknown filetype '%1'").arg(info.fileName()));
	}
}

DirectoryViewWidget::~DirectoryViewWidget()
{
	delete iconProvider;
	delete model;
	delete comboListModel;
}

void DirectoryViewWidget::setCurrentPath(QString p)
{
	if (p.isEmpty()) return;
	QFileInfo info(p);
	if (info.exists() && info.isDir())
	{
		if (info.isHidden() && ((model->filter() & QDir::Hidden) == 0))
			showHiddenFiles(true);

		QModelIndex i = model->index(p);
		path = p;
		logFine(QString("DirectoryViewWidget::setCurrentPath : dir selected %1").arg(path));
		model->setRootPath(path);
		m_dirListView->setRootIndex(i);
		m_treeView->setRootIndex(i);
		int hist_idx = m_dirComboBox->findText(path);
		if (hist_idx != -1)
		{
			m_dirComboBox->blockSignals(true);
			m_dirComboBox->setCurrentIndex(hist_idx);
			m_dirComboBox->blockSignals(false);
		}
		else
		{
			QStringList list;
			QFileInfo path_info(path);
			foreach (QString s, comboListModel->stringList())
			{
				QFileInfo info(s);
				if (info.exists() && info.isDir() && (path_info != info))
					list.append(info.canonicalFilePath());
			}

			qSort(list.begin(), list.end(), qGreater<QString>());
			list.push_front(path);
			m_dirComboBox->blockSignals(true);
			comboListModel->setStringList(list);
			m_dirComboBox->setCurrentIndex(0);
			m_dirComboBox->blockSignals(false);
		}
	}
}

QString DirectoryViewWidget::currentPath()
{
	return path;
}

void DirectoryViewWidget::saveDetailedViewState() const
{
	QSettings().setValue("directoryview/detailedviewstate", m_treeView->header()->saveState());
}

void DirectoryViewWidget::restoreDetailedViewState()
{
	QByteArray h_state = QSettings().value("directoryview/detailedviewstate", QByteArray()).toByteArray();
	QHeaderView* header = m_treeView->header();
	if (!header->restoreState(h_state))
	{
		logInfo("DirectoryViewWidget::restoreDetailedViewState : setting default state");
		QAbstractItemModel* h_model = header->model();
		int h_type_idx = 2;
		for (int i = 0 ; i < h_model->columnCount() ; i++)
		{
			if (h_model->headerData(i, Qt::Horizontal) == "Type")
			{
				h_type_idx = i;
				break;
			}
		}
		header->resizeSection(0, 300);
		header->hideSection(h_type_idx);
	}
}

void DirectoryViewWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	if (!event->spontaneous())
	{
		if (view_type == DETAILED)
			restoreDetailedViewState();
	}
}

void DirectoryViewWidget::hideEvent(QHideEvent* /*event*/)
{
	if (view_type == DETAILED)
		saveDetailedViewState();
}

void DirectoryViewWidget::closeEvent(QCloseEvent* /*e*/)
{
	logInfo("DirectoryViewWidget::closeEvent : saving settings");
	QSettings s;
	s.beginGroup("directoryview");
	s.setValue("lastdirectory", path);
	s.setValue("iconsize", m_dirListView->iconSize().width());
	s.setValue("dirhistory", comboListModel->stringList());
	s.setValue("sortorder", sort_order);
	s.setValue("sorttype", sort_type);
	s.setValue("viewtype", view_type);
	s.setValue("showhidden", (model->filter() & QDir::Hidden) != 0);
	s.setValue("detailedviewstate", m_treeView->header()->saveState());
	s.endGroup();
}

void DirectoryViewWidget::upButtonClicked()
{
	QDir dir(model->rootDirectory());
	if (dir.cdUp())
	{
		QString path(dir.canonicalPath());
		setCurrentPath(path);
		updateHistEntries(path);
	}
}

void DirectoryViewWidget::forwardButtonClicked()
{
	if (currHistEntry  <  (histEntries.size() - 1))
	{
		setCurrentPath(histEntries.at(++currHistEntry));
		m_backButton->setEnabled(currHistEntry > 0);
		m_forwardButton->setEnabled(currHistEntry < histEntries.size() - 1);
	}
}

void DirectoryViewWidget::backButtonClicked()
{
	if (currHistEntry  > 0)
	{
		setCurrentPath(histEntries.at(--currHistEntry));
		m_backButton->setEnabled(currHistEntry > 0);
		m_forwardButton->setEnabled(currHistEntry < histEntries.size() - 1);
	}
}

void DirectoryViewWidget::historyIndexChanged(const QString& path)
{
	setCurrentPath(path);
	updateHistEntries(path);
}

void DirectoryViewWidget::updateHistEntries(const QString& path)
{
	QFileInfo path_info(path);
	if (path_info.exists() && path_info.isDir()
	&& (path_info != QFileInfo(histEntries.at(currHistEntry))))
	{
		QStringList::iterator begin = histEntries.begin() + currHistEntry + 1;
		QStringList::iterator end   = histEntries.end();
		if (begin < end)
			histEntries.erase(begin, end);
		histEntries.append(path);
		currHistEntry = histEntries.size() - 1;
		m_backButton->setEnabled(currHistEntry > 0);
		m_forwardButton->setEnabled(currHistEntry < histEntries.size() - 1);
	}
}

void DirectoryViewWidget::configButtonClicked()
{
	QMenu* popup = new QMenu(this);
	QMenu* sortmenu = popup->addMenu("Sorting");
	QAction* nameaction = sortmenu->addAction("By Name");
	QAction* dateaction = sortmenu->addAction("By Date");
	QAction* sizeaction = sortmenu->addAction("By Size");
	QAction* typeaction = sortmenu->addAction("By Type");
	nameaction->setCheckable(true);
	dateaction->setCheckable(true);
	sizeaction->setCheckable(true);
	typeaction->setCheckable(true);
	if (sort_type == NAME)
		nameaction->setChecked(true);
	else if (sort_type == DATE)
		dateaction->setChecked(true);
	else if (sort_type == TYPE)
		typeaction->setChecked(true);
	else
		sizeaction->setChecked(true);

	sortmenu->addSeparator();
	QAction* orderaction = sortmenu->addAction("Descending");
	orderaction->setCheckable(true);
	orderaction->setChecked(sort_order == Qt::DescendingOrder);

	QMenu* viewmenu = popup->addMenu("View");
	QAction* shorttype = viewmenu->addAction("Short View");
	QAction* detailtype = viewmenu->addAction("Detailed View");
	shorttype->setCheckable(true);
	detailtype->setCheckable(true);
	if (view_type == SHORT)
		shorttype->setChecked(true);
	else
		detailtype->setChecked(true);

	popup->addSeparator();
	QAction* hidden = popup->addAction("Show Hidden Files");
	hidden->setCheckable(true);
	hidden->setChecked((model->filter() & QDir::Hidden) != 0);

	connect(popup, SIGNAL(triggered(QAction*)), this, SLOT(configMenuTriggered(QAction*)));
	connect(viewmenu, SIGNAL(triggered(QAction*)), this, SLOT(configMenuTriggered(QAction*)));
	connect(sortmenu, SIGNAL(triggered(QAction*)), this, SLOT(configMenuTriggered(QAction*)));

	popup->exec(m_configButton->mapToGlobal(QPoint(0,0)));

	delete popup;
}

void DirectoryViewWidget::configMenuTriggered(QAction* action)
{
	if (action->text() == "Descending")
	{
		Qt::SortOrder order = Qt::AscendingOrder;
		if (action->isChecked())
			order = Qt::DescendingOrder;
		setSortOrder(order);
	}
	else if (action->text() == "Show Hidden Files")
	{
		bool showhidden = action->isChecked();
		showHiddenFiles(showhidden);
	}
	else if (action->text() == "Short View")
		setViewType(SHORT);
	else if (action->text() == "Detailed View")
		setViewType(DETAILED);
	else
	{
		SortType type;
		if (action->text() == "By Date")
			type = DATE;
		else if (action->text() == "By Size")
			type = SIZE;
		else if (action->text() == "By Type")
			type = TYPE;
		else
			type = NAME;
		sortBy(type);
	}
}

void DirectoryViewWidget::showHiddenFiles(bool flag)
{
	if (model)
	{
		if (flag)
			model->setFilter( model->filter() | QDir::Hidden );
		else
			model->setFilter( model->filter() &  ~QDir::Hidden );
	}
}

void DirectoryViewWidget::sortBy(SortType type)
{
	sort_type = type;
	if (view_type == DETAILED)
	{
		QString header;
		if (sort_type == DATE)
			header = "Date Modified";
		else if (sort_type == SIZE)
			header = "Size";
		else if (sort_type == TYPE)
			header = "Type";
		else
			header = "Name";
		QAbstractItemModel* h_model = m_treeView->header()->model();
		for (int i = 0 ; i < h_model->columnCount() ; i++)
		{
			if (h_model->headerData(i, Qt::Horizontal) == header)
			{
				m_treeView->sortByColumn(i, sort_order);
				break;
			}
		}
	}
	else
	{
		if (model)
			model->sort((int)sort_type, sort_order);
	}
}

DirectoryViewWidget::SortType DirectoryViewWidget::sortType() const
{
	return sort_type;
}

Qt::SortOrder DirectoryViewWidget::sortOrder() const
{
	return sort_order;
}

void DirectoryViewWidget::setSortOrder(Qt::SortOrder order)
{
	sort_order = order;
	sortBy(sort_type);
}

void DirectoryViewWidget::setViewType(DirectoryViewWidget::ViewType type)
{
	view_type = type;
	switch (view_type)
	{
	case SHORT:
		saveDetailedViewState();
		m_dirListView->setVisible(true);
		m_dirListView->blockSignals(false);
		m_treeView->setVisible(false);
		m_treeView->blockSignals(true);
		sortBy(sort_type);
		break;
	case DETAILED:
		m_treeView->setVisible(true);
		m_treeView->blockSignals(false);
		m_dirListView->setVisible(false);
		m_dirListView->blockSignals(true);
		restoreDetailedViewState();
		break;
	default:
		;
	}
}

DirectoryViewWidget::ViewType DirectoryViewWidget::viewType() const
{
	return view_type;
}

void DirectoryViewWidget::detailedViewSortTypeChanged(int section, Qt::SortOrder order)
{
	sort_order = order;
	QString type = m_treeView->header()->model()->headerData(section, Qt::Horizontal).toString();
	if (type == "Date Modified")
		sort_type = DATE;
	else if (type == "Size")
		sort_type = SIZE;
	else if (type == "Type")
		sort_type = TYPE;
	else
		sort_type = NAME;
}

void DirectoryViewWidget::fileImageRendered(const QString& /*path*/)
{
	model->setIconProvider( model->iconProvider() );
}

void DirectoryViewWidget::zoomInButtonClicked()
{
	int dx( 8 );
	QAbstractItemView* view = m_dirListView;
	if (view_type == DETAILED)
		view = m_treeView;

	QSize s(view->iconSize());
	if (s.width() < 16)
		s = QSize(16, 16);
	if (s.width() <= 120)
	{
		s += QSize( dx, dx );
		view->setIconSize(s);
	}
}

void DirectoryViewWidget::zoomOutButtonClicked()
{
	int dx( -8 );
	QAbstractItemView* view = m_dirListView;
	if (view_type == DETAILED)
		view = m_treeView;

	QSize s(view->iconSize());
	if (s.width() >= 24)
	{
		s += QSize( dx, dx );
		view->setIconSize(s);
	}
}

