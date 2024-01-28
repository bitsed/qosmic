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

	LuaThreadAdapter* thread_adapter;
	QString lua_text;
	QString lua_errstr;
	QString lua_paths;
	bool lua_stopluathread_script;
	randctx ctx;
	bool input_response;
	bool input_response_ok;
	QString input_response_text;

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
		void emitScriptInputRequest(const QString&, const QString&);
		static int lua_stopluathread(lua_State*);
		static int lua_irand(lua_State*);
		static int lua_msleep(lua_State*);
		static int lua_print(lua_State*);
		static int lua_dialog(lua_State*);
		void scriptInputResponse(bool, QString&);

	signals:
		void scriptFinished();
		void scriptStopped();
		void scriptHasOutput(const QString&);
		void scriptInputRequest(const QString&, const QString&);

	protected:
		void lua_load_environment(lua_State*);

};
}


#endif
