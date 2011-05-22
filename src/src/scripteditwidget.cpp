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
#include <QSettings>

#include "scripteditwidget.h"
#include "mainwindow.h"
#include "logger.h"

ScriptEditWidget::ScriptEditWidget(MainWindow* m, QWidget* parent)
	: QWidget(parent), lua_thread(m)
{
	setupUi ( this );
	mw = m;

	QSettings settings;
	QString s = settings.value("script_editor_text", "").toString();
	m_scriptEdit->setPlainText(s);
	m_printOutputEdit->setMinimumHeight(10);
	QList<int> sizes;
	sizes << 30 << 4;
	m_splitter->setSizes(sizes);
	m_splitter->restoreState(settings.value("script_editor_splitters").toByteArray());

	connect(m_runButton, SIGNAL(pressed()), this, SLOT(runScriptAction()));
	connect(m_stopButton, SIGNAL(pressed()), this, SLOT(stopScriptAction()));
	connect(m_openButton, SIGNAL(pressed()), m_scriptEdit, SLOT(loadScript()));
	connect(m_saveButton, SIGNAL(pressed()), m_scriptEdit, SLOT(saveScript()));
	connect(m_scriptEdit, SIGNAL(scriptLoaded()), this, SLOT(loadScriptAction()));
	connect(m_scriptEdit, SIGNAL(scriptSaved()), this, SLOT(saveScriptAction()));
	connect(m_scriptEdit, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursorLabel()));
	connect(&lua_thread,  SIGNAL(scriptFinished()), this, SLOT(scriptFinishedAction()));
	connect(&lua_thread,  SIGNAL(scriptHasOutput(const QString&)), this, SLOT(appendScriptOutput(const QString&)));
}

void ScriptEditWidget::updateCursorLabel()
{
	QTextCursor c(m_scriptEdit->textCursor());
	m_filenameEdit->setText(tr("row: %1, col: %2")
	.arg(c.blockNumber() + 1)
	.arg(c.columnNumber() + 1));
}

void ScriptEditWidget::appendScriptOutput(const QString& out)
{
	m_printOutputEdit->insertPlainText(out);
	m_printOutputEdit->ensureCursorVisible();
}

void ScriptEditWidget::runScriptAction()
{
	if (lua_thread.isRunning())
		return;
	QSettings settings;
	lua_thread.setLuaText(m_scriptEdit->toPlainText());
	settings.setValue("script_editor_text", lua_thread.luaText());
	logInfo("ScriptEditWidget::runScriptAction : running script");
	m_filenameEdit->setText(tr("running script..."));
	m_printOutputEdit->clear();
	lua_thread.start();
}


void ScriptEditWidget::stopScriptAction()
{
	logInfo("ScriptEditWidget::stopScriptAction : signaling lua thread");
	lua_thread.stopScript();
	m_filenameEdit->setText(tr("script stopped"));
}


void ScriptEditWidget::scriptFinishedAction()
{
	logInfo("ScriptEditWidget::scriptFinishedAction : finished script");
	m_filenameEdit->setText(m_scriptEdit->scriptFile());
	mw->scriptFinishedSlot();
	m_filenameEdit->setText(QString(lua_thread.getMessage()));
}


void ScriptEditWidget::loadScriptAction()
{
	m_filenameEdit->setText(m_scriptEdit->scriptFile());
}


void ScriptEditWidget::saveScriptAction()
{
	m_filenameEdit->setText(tr("saved: %1")
			.arg(m_scriptEdit->scriptFile()));
}

void ScriptEditWidget::closeEvent(QCloseEvent* /*event*/)
{
	logInfo("ScriptEditWidget::closeEvent : saving settings");
	QSettings settings;
	settings.setValue("script_editor_splitters", m_splitter->saveState());
}

ScriptEditWidget::~ScriptEditWidget()
{
}
