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
#include <QPushButton>

#include "renderprogressdialog.h"

RenderProgressDialog::RenderProgressDialog(QWidget* parent, RenderThread* thread)
: QDialog(parent, Qt::Dialog), m_rthread(thread), m_showMainViewer(false)
{
	setupUi(this);
	setModal(true);

	m_yesButton->setVisible(false);
	m_noButton->setVisible(false);
	m_dialogLabel->setVisible(false);
	m_finishedLabel.setText(tr("View rendered image?"));

	connect(m_yesButton, SIGNAL(pressed()), this, SLOT(yesButtonPressedSlot()));
	connect(m_rthread, SIGNAL(statusUpdated(RenderStatus*)),
			this, SLOT(setRenderStatus(RenderStatus*)));
}


RenderProgressDialog::~RenderProgressDialog()
{
}

void RenderProgressDialog::yesButtonPressedSlot()
{
	m_showMainViewer = true;
	accept();
}

void RenderProgressDialog::setRenderStatus(RenderStatus* status)
{
	if (status->Type == RenderRequest::File)
	{
		if (status->State == RenderStatus::Busy)
			m_progressBar->setValue(status->Percent);
		else
		{
			m_dialogLabel->setVisible(true);
			m_dialogLabel->setText(status->getMessage());
			m_progressBar->setVisible(false);
			m_verticalLayout->removeWidget(m_progressBar);
			m_verticalLayout->insertWidget(1, &m_finishedLabel);
			m_yesButton->setVisible(true);
			m_noButton->setVisible(true);
			m_stopButton->setVisible(false);
			m_verticalLayout->invalidate();
		}
	}
}

bool RenderProgressDialog::showMainViewer()
{
	return m_showMainViewer;
}
