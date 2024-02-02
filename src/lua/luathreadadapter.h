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
	QList<bool> m_modified;
	QMutex m_mutex;

	public:
		static const char RegKey;

		LuaThreadAdapter(MainWindow*, LuaThread*, QObject* =0);
		~LuaThreadAdapter();

		GenomeVector* genomeVector();
		BasisTriangle* basisTriangle();
		LuaThread* thread() const;
		MainWindow* window() const;
		QList<bool>& modifiedList();
		void setModified(int, bool =true);
		void insertModified(int, bool =true);
		void removeModified(int);
		void resetModified(bool =false);
		void renderPreview(int =0);
		void update(int =0);
		bool loadFile(const QString&);
		bool saveFile(const QString&);
		bool saveImage(const QString&, int =0);
		void listen(bool);

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
