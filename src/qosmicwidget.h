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

#ifndef QOSMICWIDGET_H
#define QOSMICWIDGET_H

#include <QMap>
#include <QString>

/**
 * This here is some magik to help subclassed objects find each other.  It's
 * main use is to allow each xxxxWidget object instance access to the others.
 * Each subclassed instance needs to have a unique name or it will be clobbered.
 */
class QosmicWidget
{
	static QMap<QString,QosmicWidget*> widgets;
	QString myName;
	QosmicWidget();

	public:
		QosmicWidget(QosmicWidget*, QString);
		virtual ~QosmicWidget();
		template <typename T> T* getWidget() const;
		void setWidget(const QString&, QosmicWidget*);
		QString qosmicWidgetName();
};


#endif
