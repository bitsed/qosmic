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
#include <QDockWidget>

#include "statuswidget.h"

StatusWidget::StatusWidget(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
}

StatusWidget::~StatusWidget()
{
}

void StatusWidget::setRenderStatus(RenderStatus* state)
{
	if (isVisible())
		m_statusLabel->setText(state->getMessage());
}

/*!
    \fn StatusWidget::resizeEvent(QResizeEvent* e)
 */
void StatusWidget::resizeEvent(QResizeEvent* e)
{
	if (parent()->inherits("QDockWidget")
		   && qobject_cast<QDockWidget*>(parent())->isFloating())
		m_statusLabel->setMaximumSize(e->size());
}

