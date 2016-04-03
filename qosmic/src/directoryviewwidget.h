/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
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
#ifndef DIRECTORYVIEWWIDGET_H
#define DIRECTORYVIEWWIDGET_H

#include <QWidget>
#include <QStringListModel>
#include <QFileSystemModel>

#include "ui_directoryviewwidget.h"
#include "flamfileiconprovider.h"


class DirectoryViewWidget : public QWidget, private Ui::DirectoryViewWidget
{
	Q_OBJECT

	public:
		// these enum values are found by looking it the QFileSystemModel source
		enum SortType { NAME, SIZE, TYPE, DATE };
		enum ViewType { SHORT, DETAILED };


	public:
		DirectoryViewWidget(QWidget* parent=0);
		~DirectoryViewWidget();
		void setCurrentPath(QString);
		QString currentPath();
		SortType sortType() const;
		Qt::SortOrder sortOrder() const;
		void setSortOrder(Qt::SortOrder);
		void setViewType(ViewType);
		ViewType viewType() const;
		void fileImageRendered(const QString&);

	signals:
		void flam3FileSelected(const QString&);
		void flam3FileAppended(const QString&);
		void luaScriptSelected(const QString&);

	protected:
		void closeEvent(QCloseEvent*);
		void showEvent(QShowEvent*);
		void hideEvent(QHideEvent*);
		void updateHistEntries(const QString &path);
		void saveDetailedViewState() const;
		void restoreDetailedViewState();

	protected slots:
		void openDirectoryAction(bool);
		void selectFileAction(const QModelIndex&);
		void upButtonClicked();
		void forwardButtonClicked();
		void backButtonClicked();
		void historyIndexChanged(const QString&);
		void configButtonClicked();
		void configMenuTriggered(QAction*);
		void showHiddenFiles(bool);
		void sortBy(SortType);
		void detailedViewSortTypeChanged(int, Qt::SortOrder);
		void zoomInButtonClicked();
		void zoomOutButtonClicked();

	private:
		QFileSystemModel* model;
		QStringListModel* comboListModel;
		FlamFileIconProvider* iconProvider;
		int currHistEntry;
		QStringList histEntries;
		QString path;
		ViewType view_type;
		SortType sort_type;
		Qt::SortOrder sort_order;
};

#endif
