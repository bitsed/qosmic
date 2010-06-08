/***************************************************************************
 *   Copyright (C) 2007, 2010 by David Bitseff                             *
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
#ifndef SCRIPTEDITWIDGET_H
#define SCRIPTEDITWIDGET_H

#include "ui_scripteditwidget.h"
#include "flam3util.h"
#include "lua/luathread.h"

class MainWindow;
class ScriptEditWidget : public QWidget, private Ui::ScriptEditWidget
{
	Q_OBJECT

	public:
		ScriptEditWidget(MainWindow* m, QWidget *parent = 0);
		~ScriptEditWidget();

	public slots:
		void runScriptAction();
		void stopScriptAction();
		void scriptFinishedAction();
		void loadScriptAction();
		void saveScriptAction();

	protected:
		QString script_filename;
		Lua::LuaThread lua_thread;
		MainWindow* mw;
};

#include "mainwindow.h"

#endif
