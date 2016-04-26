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
#ifndef IMGSETTINGSWIDGET_H
#define IMGSETTINGSWIDGET_H

#include <QTimer>

#include "ui_imgsettingswidget.h"
#include "viewerpresetswidget.h"
#include "genomevector.h"

class ImageSettingsWidget : public QWidget, private Ui::ImageSettingsWidget
{
	Q_OBJECT

	public:
		ImageSettingsWidget(GenomeVector*, QWidget* parent=0);
		void updateFormData();
		void reset();

	signals:
		void dataChanged();
		void symmetryAdded();
		void presetSelected();

	protected slots:
		void fieldEditedAction();
		void applySymmetryAction();
		void moveStackLeftAction();
		void moveStackRightAction();
		void showPresetsDialog();
		void selectPresetAction(int);
		void presetsDataChangedAction();
		void resetPresetsComboBoxAction();

	protected:
		void hideEvent(QHideEvent*);

	private:
		GenomeVector* genome;
		ViewerPresetsWidget* presets;
		QTimer* timer;
};


#endif
