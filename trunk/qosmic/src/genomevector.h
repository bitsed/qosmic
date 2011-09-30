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
#ifndef GENOMEVECTOR_H
#define GENOMEVECTOR_H

#include <QVector>
#include <QAbstractListModel>

#include "flam3util.h"
#include "undoring.h"
#include "renderthread.h"

class GenomeVector : public QAbstractListModel, public QVector<flam3_genome>
{
	Q_OBJECT

	public:
		enum AutoSave { NeverSave = 0, SaveOnExit = 1, AlwaysSave = 2 };

	protected:
		RenderThread* r_thread;
		int selected_index;
		int use_previews;
		bool enable_previews;
		QSize preview_size;
		QString preview_preset;
		QList<UndoRing> undoRings;
		QList<UndoStateProvider*> providerList;
		QList<QVariant> previews;
		QList<RenderRequest*> r_requests;
		QPixmap clock_preview;
		AutoSave auto_save;

	public:
		GenomeVector();
		void setSelected(int value);
		int selected() const;
		QModelIndex selectedIndex() const;
		flam3_genome* selectedGenome();
		UndoRing* undoRing(int idx=-1);
		QList<UndoStateProvider*>* undoProviders();
		void restoreUndoState(int idx, UndoState* state);
		void addUndoState(int idx=-1);
		bool undo(int idx=-1);
		bool redo(int idx=-1);
		void append(const flam3_genome& genome);
		void insert(int i, int count, flam3_genome* genomes);
		void insert(int i, const flam3_genome& genome);
		bool remove(int i, int count=1);
		void removeAll();
		void clear();
		flam3_genome* data();  // QVector interface
		int size() const;
		AutoSave autoSave() const;
		void setAutoSave(AutoSave);

		// the QAbstractListModel interface
		bool appendRow();
		bool appendRow(const flam3_genome& genome);
		bool removeRow(int row);
		bool removeRows(int row, int count);
		bool insertRow(int row);
		bool insertRow(int row, const flam3_genome& genome);
		bool insertRows(int row, int count, flam3_genome* genomes);
		bool moveRow(int from, int to);
		int rowCount(const QModelIndex& parent=QModelIndex()) const;
		QVariant data(const QModelIndex& idx, int role=Qt::DisplayRole) const;
		QMap<int, QVariant> itemData(const QModelIndex& index) const;
		bool setData(const QModelIndex& idx, const QVariant& value, int role=Qt::EditRole);
		bool setData(flam3_genome* genomes, int ncps=1);
		Qt::ItemFlags flags(const QModelIndex&) const;
		bool hasIndex(int row, int column, const QModelIndex& parent=QModelIndex()) const;
		QVariant headerData(int section, Qt::Orientation orientation,
							int role=Qt::DisplayRole) const;
		Qt::DropActions supportedDropActions() const;


		void usingPreviews(bool);
		void enablePreviews(bool);
		void setPreviewSize(const QSize& size);
		QSize previewSize() const;
		void setPreviewPreset(const QString &s);
		QString previewPreset() const;
		void dataModified(const QList<bool>&);

	public slots:
		void flameRenderedAction(RenderEvent*);
		void updateSelectedPreview();
		void updatePreviews();
		void updatePreview(int);
		void clearPreviews();
		void clearPreview(int);

	private:
		void setCapacity(int entries);
		void createClockPreview();
};


#endif
