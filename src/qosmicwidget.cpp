/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
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

#include "qosmicwidget.h"
#include "logger.h"

QMap<QString,QosmicWidget*> QosmicWidget::widgets;

QosmicWidget::QosmicWidget()
{
}

QosmicWidget::~QosmicWidget()
{
}

QosmicWidget::QosmicWidget(QosmicWidget* parent, QString name)
	: myName(name)
{
	logFine(QString("QosmicWidget::QosmicWidget : adding instance %1").arg(name));
	setWidget(name, parent);
}


void QosmicWidget::setWidget(const QString& name, QosmicWidget* widget)
{
	if (widgets.contains(name))
		logWarn(QString("QosmicWidget::setWidget : clobbering widget '%1'").arg(name));
	widgets[name] = widget;
}

QString QosmicWidget::qosmicWidgetName()
{
	return myName;
}

