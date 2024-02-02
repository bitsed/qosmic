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

#ifndef CAMERASETTINGSWIDGET_H
#define CAMERASETTINGSWIDGET_H

#include <QWidget>

#include "ui_camerasettingswidget.h"
#include "genomevector.h"

class CameraSettingsWidget : public QWidget, private Ui::CameraSettingsWidget
{
	Q_OBJECT

	public:
		CameraSettingsWidget(GenomeVector*, QWidget* parent=0);
		void reset();

	signals:
		void dataChanged();
		void undoStateSignal();

	public slots:
		void fieldEditedAction();
		void updateFormData();

	private:
		GenomeVector* genome;
};


#endif
