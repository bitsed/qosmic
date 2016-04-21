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
#include <QImage>
#include <QPainter>
#include <QSettings>

#include "genomevector.h"
#include "viewerpresetsmodel.h"
#include "logger.h"

GenomeVector::GenomeVector()
{
	QSettings s;
	s.beginGroup("genomevector");
	preview_size   = s.value("previewsize", QSize(72, 56)).toSize();
	preview_preset = s.value("previewpreset", ViewerPresetsModel::getInstance()->presetNames().first()).toString();
	auto_save = (AutoSave)s.value("autosave", SaveOnExit).toInt();
	enable_previews = true;
	use_previews = 0;
	createClockPreview();
	r_thread = RenderThread::getInstance();
	connect(r_thread, SIGNAL(flameRendered(RenderEvent*)), this, SLOT(flameRenderedAction(RenderEvent*)));
}

int GenomeVector::selected() const
{
	return selected_index;
}

void GenomeVector::setSelected(int value)
{
	if (0 <= value && value < size())
		selected_index = value;
}

flam3_genome* GenomeVector::selectedGenome()
{
	return data() + selected_index;
}

QModelIndex GenomeVector::selectedIndex() const
{
	return index(selected_index);
}

flam3_genome* GenomeVector::data()
{
	return QVector<flam3_genome>::data();
}

QList<UndoStateProvider*>* GenomeVector::undoProviders()
{
	return &providerList;
}

UndoRing* GenomeVector::undoRing(int idx)
{
	if (idx == -1)
		idx = selected_index;
	return &undoRings[idx];
}

void GenomeVector::setCapacity(int entries)
{
	// must reserve enough space for the genomes, or "bad things" may happen
	// if the genome vector is reallocated before the whole list is loaded.
	if (capacity() <= entries)
	{
		int count = entries * 2;
		logInfo("GenomeVector::setCapacity : reserving %d entries", count);
		r_thread->running_mutex.lock();
		reserve(count);
		// requeue existing requests
		for (int n = 0 ; n < r_requests.size() ; n++)
		{
			RenderRequest* req = r_requests.at(n);
			if (req->name() == "selector" && !req->finished())
				updatePreview(n);
		}
		r_thread->running_mutex.unlock();
	}
}

void GenomeVector::append(const flam3_genome& genome)
{
	insert(size(), genome);
}

bool GenomeVector::appendRow()
{
	return insertRow(size());
}

bool GenomeVector::appendRow(const flam3_genome& genome)
{
	return insertRow(size(), genome);
}

void GenomeVector::insert(int i, int count, flam3_genome* genomes)
{
	int first = i;
	int last  = first + count;
	logFine("GenomeVector::insert : appending %d genomes at idx %d", count, first);
	setCapacity(last);
	flam3_genome preset = ViewerPresetsModel::getInstance()->preset(preview_preset);
	flam3_genome* g = genomes;
	// copy the genomes into the current list.
	for (int n = first ; n < last ; n++, g++)
	{
		QVector<flam3_genome>::insert(n, *g);
		undoRings.insert(n, UndoRing());
		UndoState* state = undoRings[n].advance();
		flam3_copy(&(state->Genome), data() + n);
		if (previews.size() <= n)
			previews.insert(n, QVariant());
		if (r_requests.size() <= n)
		{
			RenderRequest* req = new RenderRequest(0, preview_size, "selector", RenderRequest::Queued);
			req->setImagePresets(preset);
			r_requests.insert(n, req);
		}
	}
}

void GenomeVector::insert(int i, const flam3_genome& g)
{
	logFine("GenomeVector::insert : inserting %d", i);
	setCapacity(i);
	QVector<flam3_genome>::insert(i, g);
	undoRings.insert(i, UndoRing());
	UndoState* state = undoRings[i].advance();
	flam3_copy(&(state->Genome), data() + i);
	if (previews.size() <= i)
		previews.insert(i, QVariant());
	if (r_requests.size() <= i)
	{
		RenderRequest* req = new RenderRequest(0, preview_size, "selector", RenderRequest::Queued);
		req->setImagePresets(ViewerPresetsModel::getInstance()->preset(preview_preset));
		r_requests.insert(i, req);
	}
}

bool GenomeVector::insertRow(int row)
{
	if (0 <= row && row <= size())
	{
		beginInsertRows(QModelIndex(), row, row);
		flam3_genome* current = selectedGenome();
		flam3_genome gen = flam3_genome();
		Util::init_genome(&gen);
		flam3_copy(&gen, current);

		// preserve values not copied in flam3_apply_template()
		gen.pixels_per_unit = current->pixels_per_unit;
		gen.contrast   = current->contrast;
		gen.gamma      = current->gamma;
		gen.brightness = current->brightness;
		gen.vibrancy   = current->vibrancy;
		insert(row, gen);
		endInsertRows();
		clearPreview(row);
		updatePreview(row);
		return true;
	}
	return false;
}

bool GenomeVector::insertRow(int row, const flam3_genome& genome)
{
	if (0 <= row && row <= size())
	{
		beginInsertRows(QModelIndex(), row, row);
		insert(row, genome);
		endInsertRows();
		clearPreview(row);
		updatePreview(row);
		return true;
	}
	return false;
}

bool GenomeVector::insertRows(int row, int count, flam3_genome* genomes)
{
	if (0 <= row && row <= size() && count > 0)
	{
		int last = row + count;
		beginInsertRows(QModelIndex(), row, last - 1);
		insert(row, count, genomes);
		endInsertRows();
		for (int n = row ; n < last ; n++)
		{
			clearPreview(n);
			updatePreview(n);
		}
		return true;
	}
	return false;
}

bool GenomeVector::remove(int i, int count)
{
	int last = i + count - 1;
	if (0 <= i && last < size())
	{
		logFine("GenomeVector::remove : removing rows %d to %d", i, last);
		r_thread->running_mutex.lock();
		for (int n = last ; n >= i ; n--)
		{
			logFine("GenomeVector::remove : removing row %d", n);
			r_thread->cancel(r_requests[n]);
			flam3_genome* g = data() + n;
			// free the heap mem used by xforms in existing genomes
			clear_cp(g, flam3_defaults_on);
			QVector<flam3_genome>::remove(n);
			undoRings.removeAt(n);
		}
		r_thread->running_mutex.unlock();
		// change the selected_index
		if (selected_index >= size())
			selected_index = qMax(0, size() - 1);
		return true;
	}
	return false;
}

bool GenomeVector::removeRow(int row)
{
	beginRemoveRows(QModelIndex(), row, row);
	bool rv = remove(row);
	previews.removeAt(row);
	endRemoveRows();
	return rv;
}

bool GenomeVector::removeRows(int row, int count)
{
	int last_row = row + count - 1;
	if (0 <= row && last_row < size())
	{
		logFine("GenomeVector::removeRows : clearing genomes [%d, %d]", row, last_row);
		beginRemoveRows(QModelIndex(), row, last_row);
		bool rv = remove(row, count);
		previews.removeAt(row);
		endRemoveRows();
		return rv;
	}
	return false;
}

bool GenomeVector::moveRow(int from, int to)
{
	if (to == from) // nothing to move
		return false;

	if ((to - from) == 1)         // this will cause Qt to throw an exception
		return moveRow(to, from); // from beginMoveRows(), so swap them

	if (0 <= from && from < size() && 0 <= to && to < size())
	{
		logFine("GenomeVector::moveRow : from %d, to %d", from, to);
		QModelIndex parent(index(0, 0).parent());
		beginMoveRows(parent, from, from, parent, to);

		QVector<double> times;
		int dist = qAbs(to - from);
		times.reserve(dist);
		int start  = qMin(from, to);
		int finish = qMax(from, to);
		for (int n = start ; n <= finish ; n++)
			times.append((data() + n)->time);

		flam3_genome g_tmp = *(data() + from);
		QVariant v_tmp(previews[from]);
		UndoRing r_tmp(undoRings[from]);
		RenderRequest* req(r_requests[from]);

		QVector<flam3_genome>::remove(from);
		undoRings.removeAt(from);
		previews.removeAt(from);
		r_requests.removeAt(from);

		QVector<flam3_genome>::insert(to, g_tmp);
		undoRings.insert(to, r_tmp);
		previews.insert(to, v_tmp);
		r_requests.insert(to, req);

		for (int n = 0 ; n <= dist ; n++)
			(data() + start + n)->time = times[n];

		endMoveRows();
		return true;
	}
	return false;
}

void GenomeVector::removeAll()
{
	logFine("GenomeVector::removeAll : removing %d genomes", size());
	removeRows(0, qMax(1, size()));
}

void GenomeVector::clear()
{
	logFine("GenomeVector::clear : removing %d genomes", size());
	remove(0, size());
}

int GenomeVector::size() const
{
	return QVector<flam3_genome>::size();
}

bool GenomeVector::undo(int idx)
{
	if (idx == -1)
		idx = selected_index;
	UndoRing* ring = undoRing(idx);
	if (ring->atTail())
		return false;
	else
		restoreUndoState(idx, ring->prev());
	return true;
}

bool GenomeVector::redo(int idx)
{
	if (idx == -1)
		idx = selected_index;
	UndoRing* ring = undoRing(idx);
	if (ring->atHead())
		return false;
	else
		restoreUndoState(idx, ring->next());
	return true;
}

void GenomeVector::restoreUndoState(int idx, UndoState* state)
{
	logFine("GenomeVector::restoreUndoState : restoring genome[%d]", idx);
	flam3_genome* old = &(state->Genome);
	flam3_genome* g = data() + idx;
	flam3_copy(g, old);
	foreach (UndoStateProvider* provider, providerList)
		provider->restoreState(state);
}

void GenomeVector::addUndoState(int idx)
{
	logFine("GenomeVector::addUndoState : adding state to genome[%d]", idx);
	if (idx == -1)
		idx = selected_index;
	UndoState* state = undoRing(idx)->advance();
	flam3_copy(&(state->Genome), data() + idx);
	foreach (UndoStateProvider* provider, providerList)
		provider->provideState(state);
}

int GenomeVector::rowCount(const QModelIndex& /*parent*/) const
{
	return GenomeVector::size();
}

QVariant GenomeVector::data(const QModelIndex& idx, int role) const
{
	if (!idx.isValid())
		return QVariant();

	int row = idx.row();
	if (row >= size() || row < 0)
	{
		logWarn(QString("GenomeVector::data : genome doesn't exist %1").arg(row));
		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		flam3_genome* g = const_cast<flam3_genome*>(QVector<flam3_genome>::data()) + row;
		return QString("%1 xforms\ntime: %2").arg(g->num_xforms).arg(g->time);
	}

	if (role == Qt::DecorationRole && row < previews.size())
		return previews.at(row);

	return QVariant();
}

QMap<int, QVariant> GenomeVector::itemData(const QModelIndex& index) const
{
	QMap<int, QVariant> map(QAbstractItemModel::itemData(index));
	if (!index.isValid())
		return map;
	int row = index.row();
	if (row >= size() || row < 0)
	{
		logWarn(QString("GenomeVector::itemData : genome doesn't exist %1").arg(row));
		return map;
	}
	flam3_genome* g = const_cast<flam3_genome*>(QVector<flam3_genome>::data()) + row;
	QString s( QString("%1 xforms\ntime: %2").arg(g->num_xforms).arg(g->time) );
	map.insert(Qt::DisplayRole, s);
	map.insert(Qt::EditRole, s);
	map.insert(Qt::DecorationRole, previews.at(row));
	map.insert(Qt::ToolTipRole, QString("genome %1").arg(row + 1));

	return map;
}

bool GenomeVector::setData(const QModelIndex& idx, const QVariant& value, int role)
{
	if (!idx.isValid())
		return false;

	int row = idx.row();
	if (row > size() || row < 0)
	{
		logWarn(QString("GenomeVector::setData : genome doesn't exist %1").arg(row));
		return false;
	}

	if (role == Qt::DecorationRole)
	{
		if (previews.size() > row)
			previews.replace(row, value);
		else
		{
			logWarn("GenomeVector::setData : setting preview for non-existent genome %d", row);
			previews.insert(row, value);
		}
		emit dataChanged(idx, idx);
	}

	return true;
}

bool GenomeVector::setData(flam3_genome* genomes, int ncps)
{
	logFine("GenomeVector::setData : setting %d genomes", ncps);
	beginResetModel();
	r_thread->running_mutex.lock();
	for (int n = size() - 1 ; n >= 0 ; n--)
	{
		logFine("GenomeVector::setData : removing row %d", n);
		r_thread->cancel(r_requests[n]);
		flam3_genome* g = data() + n;
		// free the heap mem used by xforms in existing genomes
		clear_cp(g, flam3_defaults_on);
		QVector<flam3_genome>::remove(n);
		undoRings.removeAt(n);
	}
	r_thread->running_mutex.unlock();
	insert(0, ncps, genomes);
	setSelected(0);
	endResetModel();
	updatePreviews();
	return true;
}

Qt::ItemFlags GenomeVector::flags(const QModelIndex& idx) const
{
	if (!idx.isValid())
		return 0;

	if (idx.row() > size() || idx.row() < 0)
		return 0;

	return Qt::ItemIsSelectable | Qt::ItemIsEnabled |
		Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable ;
}

Qt::DropActions GenomeVector::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

bool GenomeVector::hasIndex(int row, int column, const QModelIndex& /*parent*/) const
{
	if (column == 0 && row < size())
		return true;
	return false;

}

QVariant GenomeVector::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	int row = section;
	return QString("Genome %1").arg(row);
}

void GenomeVector::updatePreviews()
{
	if (enable_previews && use_previews > 0)
	{
		clearPreviews();
		logFine("GenomeVector::updatePreview : rendering previews");
		for (int idx = 0 ; idx < size() ; idx++)
		{
			RenderRequest* req = r_requests.at(idx);
			flam3_genome* g = data() + idx;
			req->setGenome(g);
			r_thread->render(req);
		}
	}
}

void GenomeVector::updatePreview(int idx)
{
	if (enable_previews && use_previews > 0 && idx >= 0 && idx < size())
	{
		logFine("GenomeVector::updatePreview : rendering request %d", idx);
		RenderRequest* req = r_requests[idx];
		flam3_genome* g = data() + idx;
		req->setGenome(g);
		r_thread->render(req);
	}
}

void GenomeVector::updateSelectedPreview()
{
	if (enable_previews && use_previews > 0)
	{
		int idx = selected();
		logFine("GenomeVector::updateSelectedPreview : rendering request %d", idx);
		RenderRequest* req = r_requests[idx];
		flam3_genome* g = data() + idx;
		req->setGenome(g);
		r_thread->render(req);
	}
}

void GenomeVector::flameRenderedAction(RenderEvent* e)
{
	RenderRequest* req = e->request();
	if (req->type() == RenderRequest::Queued && req->name() == "selector")
	{
		int idx = r_requests.indexOf(req, 0);

		if (idx < size())
		{
			logFine(QString("GenomeVector::flameRenderedAction : setting genome data %1,g=0x%2,req=0x%3")
					.arg(idx).arg((long)req->genome(),0,16).arg((long)req,0,16));
			setData(index(idx), QPixmap::fromImage(req->image()), Qt::DecorationRole);
		}
		else
		{
			logFine(QString("GenomeVector::flameRenderedAction : skipping genome data %1,g=0x%2,req=0x%3")
					.arg(idx).arg((long)req->genome(),0,16).arg((long)req,0,16));
		}
		e->accept();
	}
}

void GenomeVector::createClockPreview()
{
	logFine("GenomeVector::createClockPreview : enter");
	QImage img(preview_size, QImage::Format_RGB32);
	QPainter p(&img);
	QRect bounds(QPoint(0,0), preview_size);
	p.fillRect(bounds, Qt::white);
	int lx = preview_size.width()  / 2 - 8;
	int ly = preview_size.height() / 2 - 8;
	p.drawPixmap(lx, ly, 16, 16, QPixmap(":icons/silk/clock.xpm"));
	p.setPen(Qt::gray);
	p.setBrush(QBrush(Qt::gray, Qt::NoBrush));
	p.drawRect(bounds.adjusted(0, 0, -1, -1));
	clock_preview = QPixmap::fromImage(img);
}

void GenomeVector::clearPreviews()
{
	for (int n = 0 ; n < rowCount() ; n++)
		clearPreview(n);
}

void GenomeVector::clearPreview(int n)
{
	QModelIndex model_idx = index(n);
	if (model_idx.isValid())
		setData(model_idx, clock_preview, Qt::DecorationRole);
}

void GenomeVector::setPreviewSize(const QSize& size)
{
	preview_size = size;
	QSettings().setValue("genomevector/previewsize", size);
	createClockPreview();
	foreach (RenderRequest* r, r_requests)
		r->setSize(size);
}

QSize GenomeVector::previewSize() const
{
	return preview_size;
}

void GenomeVector::setPreviewPreset(const QString& s)
{
	preview_preset = s;
	QSettings().setValue("genomevector/previewpreset", s);
	flam3_genome preset = ViewerPresetsModel::getInstance()->preset(s);
	foreach (RenderRequest* r, r_requests)
		r->setImagePresets(preset);
}

QString GenomeVector::previewPreset() const
{
	return preview_preset;
}

GenomeVector::AutoSave GenomeVector::autoSave() const
{
	return auto_save;
}

void GenomeVector::setAutoSave(GenomeVector::AutoSave as)
{
	auto_save = as;
	QSettings().setValue("genomevector/autosave", (int)as);
}

void GenomeVector::usingPreviews(bool flag)
{
	if (flag)
	{
		use_previews += 1;
		if (use_previews == 1)
			updatePreviews();
	}
	else
		use_previews = qMax(0, use_previews - 1);
}

void GenomeVector::enablePreviews(bool flag)
{
	enable_previews = flag;
}

void GenomeVector::dataModified(const QList<bool>& modified)
{
	beginResetModel();
	if (modified.size() != size())
		logWarn("GenomeVector::dataModified : argument list size != data size");
	if (selected_index >= size())
		selected_index = size() - 1;
	for (int n = 0 ; n < size() ; n++)
		if (modified.at(n))
		{
			addUndoState(n);
			updatePreview(n);
		}
	endResetModel();
}
