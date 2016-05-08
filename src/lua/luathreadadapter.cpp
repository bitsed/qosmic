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
#include "luathreadadapter.h"
#include "flam3filestream.h"
#include "mainwindow.h"
#include "logger.h"

const char Lua::LuaThreadAdapter::RegKey = 'k';

Lua::LuaThreadAdapter::LuaThreadAdapter(MainWindow* mw, LuaThread* t, QObject* parent)
 : QObject(parent), m_win(mw), m_thread(t)
{
	logFine("Lua::LuaThreadAdapter::LuaThreadAdapter : const");
	moveToThread(t);
	connect(this, SIGNAL(updateSignal()), m_win->xformEditor(), SLOT(reset()));
}

Lua::LuaThreadAdapter::~LuaThreadAdapter()
{
	logFine("Lua::LuaThreadAdapter::LuaThreadAdapter : dest");
}


void Lua::LuaThreadAdapter::listen(bool listen)
{
	if (listen)
	{
		connect(m_win, SIGNAL(mainWindowChanged()),
			this, SLOT(mainWindowChangedSlot()),
			Qt::QueuedConnection);
		connect(m_win->renderThread(), SIGNAL(flameRendered(RenderEvent*)),
			this, SLOT(flameRenderedSlot(RenderEvent*)),
			Qt::QueuedConnection);
	}
	else
	{
		disconnect(m_win->renderThread(), SIGNAL(flameRendered(RenderEvent*)),
			this, SLOT(flameRenderedSlot(RenderEvent*)));
		disconnect(m_win, SIGNAL(mainWindowChanged()),
			this, SLOT(mainWindowChangedSlot()));
	}
}


GenomeVector* Lua::LuaThreadAdapter::genomeVector()
{
	return m_win->genomeVector();
}

BasisTriangle* Lua::LuaThreadAdapter::basisTriangle()
{
	return m_win->xformEditor()->basis();
}

void Lua::LuaThreadAdapter::renderPreview(int idx)
{
	logFine("Lua::LuaThreadAdapter::renderPreview");
	m_win->renderPreview(idx);
	waitForEvent();
}

void Lua::LuaThreadAdapter::update(int idx)
{
	logFine("Lua::LuaThreadAdapter::update");
	emit updateSignal();
	renderPreview(idx);
}

bool Lua::LuaThreadAdapter::loadFile(const QString& name)
{
	logFine("Lua::LuaThreadAdapter::loadFile");
	QFile file(name);
	Flam3FileStream s(&file);
	int ncps(0);
	flam3_genome* in;
	if (s.read(&in, &ncps))
	{
		GenomeVector* genomes = genomeVector();
		genomes->clear();
		genomes->insert(0, ncps, in);
		return true;
	}
	return false;
}

bool Lua::LuaThreadAdapter::saveFile(const QString& name)
{
	QFile file(name);
	Flam3FileStream s(&file);
	return s.write(genomeVector());
}

bool Lua::LuaThreadAdapter::saveImage(const QString& name, int idx)
{
	logFine("Lua::LuaThreadAdapter::saveImage");
	bool n = m_win->saveImage(name, idx);
	waitForEvent();
	return n;
}

void Lua::LuaThreadAdapter::flameRenderedSlot(RenderEvent* /*e*/)
{
	logFine("Lua::LuaThreadAdapter::flameRenderedSlot : signaling wait event");
	if (m_mutex.tryLock())
		m_mutex.unlock();
	else
		m_mutex.unlock();
}

void Lua::LuaThreadAdapter::mainWindowChangedSlot()
{
	logFine("Lua::LuaThreadAdapter::mainWindowChangedSlot : signaling wait event");
	if (m_mutex.tryLock())
		m_mutex.unlock();
	else
		m_mutex.unlock();
}

void Lua::LuaThreadAdapter::waitForEvent()
{
	m_mutex.lock();
	while(!m_mutex.tryLock())
	{
		logFinest("Lua::LuaThreadAdapter::waitForEvent : waiting");
		QCoreApplication::processEvents();
	}
	m_mutex.unlock();
}



Lua::LuaThread* Lua::LuaThreadAdapter::thread() const
{
	return m_thread;
}


MainWindow* Lua::LuaThreadAdapter::window() const
{
	return m_win;
}

QList<bool>& Lua::LuaThreadAdapter::modifiedList()
{
	return m_modified;
}

void Lua::LuaThreadAdapter::insertModified(int idx, bool flag)
{
	m_modified.insert(idx, flag);
}

void Lua::LuaThreadAdapter::setModified(int idx, bool flag)
{
	m_modified.replace(idx, flag);
}

void Lua::LuaThreadAdapter::removeModified(int idx)
{
	m_modified.removeAt(idx);
}

void Lua::LuaThreadAdapter::resetModified(bool flag)
{
	m_modified.clear();
	for (int i = 0 ; i < genomeVector()->size() ; i++)
		m_modified.append(flag);
}

