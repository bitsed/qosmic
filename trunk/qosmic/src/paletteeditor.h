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


#ifndef PALETTEEDITOR_H
#define PALETTEEDITOR_H

#include <QWidget>
#include <QShowEvent>
#include <QCloseEvent>
#include <QFileInfoList>
#include <QGradientStops>

#include "ui_paletteeditor.h"
#include "gradientlistmodel.h"
#include "checkersbrush.h"
#include "flam3util.h"


class PaletteEditor : public QWidget, private Ui::PaletteEditor
{
	Q_OBJECT

	private:
		// this class wraps the palette type so they can be automagically
		// allocated, deallocated, and stored in a QList
		class flam3_palette_t
		{
			public:
				flam3_palette_t(flam3_palette p)
				{
					memcpy(pa, p, sizeof(flam3_palette));
				}
				flam3_palette pa;
		};


		static const int PaletteCount          = 701;
		static const int GradientBufferSize    = 1024;

		flam3_palette p;
		GradientStops p_stops;
		GradientListModel m_flamPalettes;
		GradientListModel m_browsePalettes;
		QFileInfoList m_browseFileList;
		QString m_lastBrowseDir;
		QList<flam3_palette_t> ugrList;
		QButtonGroup* m_gradientSpreadGroup;
		CheckersBrush checkers;
		QRgb m_gradient[GradientBufferSize];
		bool hasUGR;

	public:
		PaletteEditor(QWidget* parent=0);
		void getPalette(flam3_palette);
		void setPalette(flam3_palette);
		bool loadGIMPGradient(QFileInfo&, flam3_palette);
		bool loadCPTGradient(QFileInfo&, flam3_palette);
		bool loadUGRGradients(QFileInfo&);
		void setLastBrowseDir (const QString&);
		QString lastBrowseDir() const;

	signals:
		void paletteChanged();
		void undoStateSignal();

	public slots:
		void paletteIndexChangedAction(const QModelIndex&);
		void paletteRotatedAction(int);
		void openGradientAction(bool);
		void selectGradientAction(const QModelIndex&);
		void loadPalette(int);
		void stopsChangedAction();
		void saveGradientAction();
		void resetGradientAction();
		void browsePathChangedAction();
		void createRandomGradientAction();

	protected:
		void showEvent(QShowEvent*);
		void buildPaletteSelector();
		void setPaletteView();
		void closeEvent(QCloseEvent*);
};

#endif
