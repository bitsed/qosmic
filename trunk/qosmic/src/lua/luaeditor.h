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
#ifndef LUALUAEDITOR_H
#define LUALUAEDITOR_H

#include <QTextEdit>
#include "highlighter.h"

namespace Lua {


class LuaEditor : public QTextEdit
{
	Q_OBJECT

	public:
		LuaEditor(QWidget* pa=0);
		~LuaEditor();

	public slots:
		bool save();
		bool loadScript(QString filename=QString());
		bool saveScript(QString filename=QString());
		QString scriptFile() const;

	signals:
		void scriptLoaded();
		void scriptSaved();

	protected:
		void keyPressEvent(QKeyEvent*);
		QString script_filename;
		QWidget* parent;
		Highlighter* highlighter;
};

}

#endif
