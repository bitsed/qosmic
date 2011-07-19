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
#ifndef LUATHREAD_H
#define LUATHREAD_H

#include <QThread>

#include "flam3util.h"
#include "lunar.h"
#include "frame.h"
#include "renderthread.h"

class MainWindow;
class LuaThreadAdapter;
namespace Lua
{

class LuaThread : public QThread
{
	Q_OBJECT

	MainWindow* mw;
	LuaThreadAdapter* thread_adapter;
	QString lua_text;
	QString lua_error;
	QString lua_paths;
	bool lua_stopluathread_script;
	randctx ctx;

	public:
		LuaThread(MainWindow* m, QObject* parent=0);
		~LuaThread();
		void msleep(unsigned long msecs);
		virtual void run();
		void setLuaText(QString);
		const QString& luaText();
		void setLuaPaths(const QString&);
		QString luaPaths() const;
		void stopScript();
		bool stopping() const;
		QString getMessage();
		void emitScriptOutput(const QString&);
		static int lua_stopluathread(lua_State*);
		static int lua_irand(lua_State*);
		static int lua_msleep(lua_State*);
		static int lua_print(lua_State*);

	signals:
		void scriptFinished();
		void scriptStopped();
		void scriptHasOutput(const QString&);

	protected:
		void lua_load_environment(lua_State*);

};
}


#endif
