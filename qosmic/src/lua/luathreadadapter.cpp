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
#include "luathreadadapter.h"
#include "mainwindow.h"
#include "logger.h"

const char Lua::LuaThreadAdapter::RegKey = 'k';

Lua::LuaThreadAdapter::LuaThreadAdapter(MainWindow* mw, LuaThread* t, QObject* parent)
 : QObject(parent), m_win(mw), m_thread(t)
{
	logFine("Lua::LuaThreadAdapter::LuaThreadAdapter : const");
	moveToThread(t);
	connect(m_win, SIGNAL(mainWindowChanged()),
			this, SLOT(mainWindowChangedSlot()), Qt::QueuedConnection);
	connect(m_win->renderThread(), SIGNAL(flameRendered(RenderEvent*)),
			this, SLOT(flameRenderedSlot(RenderEvent*)), Qt::QueuedConnection);
	connect(this, SIGNAL(loadFileSignal(const QString&)),
			m_win, SLOT(loadFile(const QString&)), Qt::QueuedConnection);
	connect(m_thread, SIGNAL(scriptStopped()),
			this, SLOT(mainWindowChangedSlot()), Qt::QueuedConnection);
	connect(this, SIGNAL(updateSignal()), m_win->xformEditor(),
			SLOT(reset()),  Qt::QueuedConnection);
}


Lua::LuaThreadAdapter::~LuaThreadAdapter()
{
	logFine("Lua::LuaThreadAdapter::LuaThreadAdapter : dest");
	disconnect(this, SIGNAL(loadFileSignal(const QString&)),
			   m_win, SLOT(loadFile(const QString&)));
	disconnect(m_win->renderThread(), SIGNAL(flameRendered(RenderEvent*)),
			   this, SLOT(flameRenderedSlot(RenderEvent*)));
	disconnect(m_win, SIGNAL(mainWindowChanged()),
			   this, SLOT(mainWindowChangedSlot()));
	disconnect(m_thread, SIGNAL(scriptStopped()),
			   this, SLOT(mainWindowChangedSlot()));
	disconnect(this, SIGNAL(updateSignal()),
			   m_win->xformEditor(), SLOT(reset()));
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
	emit loadFileSignal(name);
	waitForEvent();
	return true;
}

bool Lua::LuaThreadAdapter::saveFile(const QString& name)
{
	return m_win->saveFile(name);
}

bool Lua::LuaThreadAdapter::saveImage(const QString& name, int idx)
{
	logFine("Lua::LuaThreadAdapter::saveImage");
	bool n = m_win->saveImage(name, idx);
	waitForEvent();
	return n;
}

bool Lua::LuaThreadAdapter::save()
{
	return m_win->save();
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
		m_thread->msleep(10);
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

