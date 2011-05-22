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
#ifndef LUATHREADADAPTER_H
#define LUATHREADADAPTER_H

#include <QObject>
#include <QMutex>

#include "luathread.h"


class BasisTriangle;
class MainWindow;

namespace Lua
{

class LuaThreadAdapter : public QObject
{
	Q_OBJECT

	MainWindow* m_win;
	LuaThread* m_thread;
	QMutex m_mutex;

	public:
		LuaThreadAdapter(MainWindow*, LuaThread*, QObject* =0);
		~LuaThreadAdapter();

		GenomeVector* genomeVector();
		BasisTriangle* basisTriangle();
		LuaThread* thread() const;
		MainWindow* window() const;
		void renderPreview(int =0);
		void update(int =0);
		bool loadFile(const QString&);
		bool saveFile(const QString&);
		bool saveImage(const QString&, int =0);
		bool save();

	public slots:
		void flameRenderedSlot(RenderEvent* e);
		void mainWindowChangedSlot();

	signals:
		void loadFileSignal(const QString&);
		void updateSignal();

	private:
		void waitForEvent();

};

}

#endif
