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
		void lineWidgetPaintEvent(QPaintEvent*);
	    int lineWidgetWidth();

	public slots:
		bool save();
		bool load(QString filename=QString());
		bool saveAs(QString filename=QString());
		QString scriptFile() const;
		void setCurrentFont(const QFont&);

	signals:
		void scriptLoaded();
		void scriptSaved();

	protected:
		void resizeEvent(QResizeEvent*);
		void keyPressEvent(QKeyEvent*);
		void scrollContentsBy(int dx, int dy);
		QTextBlock getFirstVisibleBlock();
		QString script_filename;
		QWidget* parent;
		QWidget* lineWidget;
		Highlighter* highlighter;

	private slots:
		void highlightCurrentLine();
		void updateLineWidgetWidth(int);
};


class LineWidget : public QWidget
{
	Q_OBJECT

	public:
		LineWidget(LuaEditor* editor);
		QSize sizeHint() const;

	protected:
		void paintEvent(QPaintEvent *event);

	private:
		LuaEditor* luaEditor;
};



}

#endif
