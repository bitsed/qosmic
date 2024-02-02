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
#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H

#include "ui_renderdialog.h"



class RenderDialog : public QDialog, private Ui::RenderDialog
{
	Q_OBJECT

	public:
		RenderDialog(QWidget*, QString name, QString path,
				QSize size, QStringList list);
		~RenderDialog();
		QString absoluteFilePath();
		QString selectedPreset();
		bool presetSelected();
		QSize selectedSize();
		bool sizeSelected();


	public slots:
		void filePathButtonSlot();
		void accept();
		void addSizeButtonSlot();
		void delSizeButtonSlot();

	private:
		QString fileName;
		QString lastDir;
		QSize imgSize;
		QString sizeText;
		QStringList presets;
		QString size;
		QString preset;
		QString nopreset;

		QRegExpValidator sizeValidator;
};



#endif

