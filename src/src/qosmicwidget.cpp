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


QosmicWidget* QosmicWidget::getWidget(const QString& name)
{
	QosmicWidget* w =  widgets[name];
	if (!w)
		logError(QString("QosmicWidget::getWidget : invalid name '%1'").arg(name));
	return w;
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

