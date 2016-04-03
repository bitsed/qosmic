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
#include <QSettings>

#include "scripteditwidget.h"
#include "mainwindow.h"
#include "logger.h"

ScriptEditWidget::ScriptEditWidget(MainWindow* m, QWidget* parent)
	: QWidget(parent), lua_thread(m)
{
	setupUi ( this );
	mw = m;

	QSettings s;
	s.beginGroup("scripteditwidget");
	m_scriptEdit->setPlainText(s.value("editortext", "print('hello world.')").toString());
	m_scriptEdit->setCurrentFont(s.value("editorfont", m_scriptEdit->currentFont()).value<QFont>());
	lua_thread.setLuaPaths(s.value("luapaths", lua_thread.luaPaths()).toString());

	m_printOutputEdit->setMinimumHeight(10);
	QList<int> sizes;
	sizes << 30 << 40;
	m_splitter->setSizes(sizes);
	m_splitter->restoreState(s.value("editorsplitters").toByteArray());

	connect(m_runButton, SIGNAL(pressed()), this, SLOT(runButtonPressedAction()));
	connect(m_openButton, SIGNAL(pressed()), m_scriptEdit, SLOT(load()));
	connect(m_saveButton, SIGNAL(pressed()), m_scriptEdit, SLOT(save()));
	connect(m_saveAsButton, SIGNAL(pressed()), m_scriptEdit, SLOT(saveAs()));
	connect(m_configButton, SIGNAL(pressed()), this, SLOT(configPressedAction()));
	connect(m_scriptEdit, SIGNAL(scriptLoaded()), this, SLOT(scriptLoadedAction()));
	connect(m_scriptEdit, SIGNAL(scriptSaved()), this, SLOT(scriptSavedAction()));
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

void ScriptEditWidget::runButtonPressedAction()
{
	if (lua_thread.isRunning())
		stopScript();
	else
		startScript();
}

void ScriptEditWidget::startScript()
{
	QSettings s;
	s.beginGroup("scripteditwidget");
	QString text(m_scriptEdit->toPlainText());
	lua_thread.setLuaText(text);
	s.setValue("editortext", text);
	logInfo("ScriptEditWidget::runScriptAction : running script");
	m_filenameEdit->setText(tr("running script..."));
	m_printOutputEdit->clear();
	lua_thread.start();
	m_runButton->setIcon(QIcon(":icons/silk/stop.xpm"));
}

void ScriptEditWidget::stopScript()
{
	logInfo("ScriptEditWidget::stopScript : signaling lua thread");
	lua_thread.stopScript();
	m_filenameEdit->setText(tr("script stopped"));
	m_runButton->setIcon(QIcon(":icons/silk/bullet_go.xpm"));
}

void ScriptEditWidget::scriptFinishedAction()
{
	logInfo("ScriptEditWidget::scriptFinishedAction : finished script");
	m_filenameEdit->setText(m_scriptEdit->scriptFile());
	mw->scriptFinishedSlot();
	m_filenameEdit->setText(QString(lua_thread.getMessage()));
	m_runButton->setIcon(QIcon(":icons/silk/bullet_go.xpm"));
}

void ScriptEditWidget::scriptLoadedAction()
{
	m_filenameEdit->setText(m_scriptEdit->scriptFile());
}

void ScriptEditWidget::scriptSavedAction()
{
	m_filenameEdit->setText(tr("saved: %1")
			.arg(m_scriptEdit->scriptFile()));
}

void ScriptEditWidget::closeEvent(QCloseEvent* /*event*/)
{
	logInfo("ScriptEditWidget::closeEvent : saving settings");
	QSettings s;
	s.beginGroup("scripteditwidget");
	s.setValue("editorsplitters", m_splitter->saveState());
}

ScriptEditWidget::~ScriptEditWidget()
{
}

void ScriptEditWidget::loadScript(const QString& path)
{
	m_scriptEdit->load(path);
}

void ScriptEditWidget::configPressedAction()
{
	ScriptEditConfigDialog d(this);
	d.setFont(m_scriptEdit->currentFont());
	d.setLuaEnvText(lua_thread.luaPaths());
	d.move(QCursor::pos());
	if (d.exec() == QDialog::Accepted)
	{
		QFont f(d.getFont());
		QString paths(d.getLuaEnvText());
		QSettings s;
		s.beginGroup("scripteditwidget");
		s.setValue("editorfont", f);
		s.setValue("luapaths", paths);
		m_scriptEdit->setCurrentFont(f);
		lua_thread.setLuaPaths(paths);
	}
}

ScriptEditConfigDialog::ScriptEditConfigDialog(QWidget* parent)
: QDialog(parent)
{
	setupUi(this);
	connect(fontSelectBox, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(fontSelectBoxChanged(const QFont&)));
}

void ScriptEditConfigDialog::changeEvent(QEvent* e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		retranslateUi(this);
		break;
	default:
		break;
	}
}

void ScriptEditConfigDialog::setFont(const QFont& f)
{
	cur_size = QString::number(f.pointSize());
	fontSelectBox->setCurrentFont(f);
}

QFont ScriptEditConfigDialog::getFont() const
{
	QFont f(fontSelectBox->currentFont());
	f.setPointSize(fontSizeBox->currentText().toInt());
	return f;
}

void ScriptEditConfigDialog::setLuaEnvText(const QString& text)
{
	plainTextEdit->setPlainText(text);
}

QString ScriptEditConfigDialog::getLuaEnvText() const
{
	return plainTextEdit->toPlainText();
}

void ScriptEditConfigDialog::fontSelectBoxChanged(const QFont& f)
{
	QString size = fontSizeBox->currentText();
	if (!size.isEmpty())
		cur_size = size;

	QList<int> sizes = fonts.smoothSizes(f.family(), "Normal");
	fontSizeBox->clear();
	foreach (int i, sizes)
		fontSizeBox->addItem(QString::number(i));
	fontSizeBox->setCurrentIndex(qMax(0, fontSizeBox->findText(cur_size)));
}

bool ScriptEditWidget::isScriptRunning() const
{
	return lua_thread.isRunning();
}
