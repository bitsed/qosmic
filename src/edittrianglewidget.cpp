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
#include <QSettings>

#include "edittrianglewidget.h"
#include "logger.h"

#define genome_ptr (genome->selectedGenome())

EditTriangleWidget::EditTriangleWidget(GenomeVector* g, FigureEditor* e, QWidget* parent)
: QWidget(parent), QosmicWidget(this, "EditTriangleWidget"), genome(g), editor(e)
{
	setupUi(this);

	m_moveLeftButton->setAutoRepeat(false);
	m_moveRightButton->setAutoRepeat(false);
	m_moveUpButton->setAutoRepeat(false);
	m_moveDownButton->setAutoRepeat(false);

	// restore (Double/Int)ValueEditor settings
	m_multiplierEditor->restoreSettings();
	m_rotateEditor->restoreSettings();
	m_scaleEditor->restoreSettings();

	reset();

	QSettings settings;
	settings.beginGroup("edittrianglewidget");
	m_multiplierEditor->updateValue(settings.value("movemultiplier", 1.0).toDouble());

	connect(m_rotateEditor, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_rotateEditor, SIGNAL(valueUpdated()), this, SLOT(triangleRotateAction()));

	connect(m_scaleUpButton, SIGNAL(released()), this, SIGNAL(undoStateSignal()));
	connect(m_scaleUpButton, SIGNAL(pressed()), this, SLOT(triangleScaleUpAction()));

	connect(m_scaleDownButton, SIGNAL(released()), this, SIGNAL(undoStateSignal()));
	connect(m_scaleDownButton, SIGNAL(pressed()), this, SLOT(triangleScaleDownAction()));

	connect(m_rotateLeftButton, SIGNAL(released()), this, SIGNAL(undoStateSignal()));
	connect(m_rotateLeftButton, SIGNAL(pressed()), this, SLOT(triangleRotateCCWAction()));

	connect(m_rotateRightButton, SIGNAL(released()), this, SIGNAL(undoStateSignal()));
	connect(m_rotateRightButton, SIGNAL(pressed()), this, SLOT(triangleRotateCWAction()));

	connect(m_hFlipButton, SIGNAL(released()), this, SIGNAL(undoStateSignal()));
	connect(m_hFlipButton, SIGNAL(pressed()), editor, SLOT(flipTriangleHAction()));

	connect(m_vFlipButton, SIGNAL(released()), this, SIGNAL(undoStateSignal()));
	connect(m_vFlipButton, SIGNAL(pressed()), editor, SLOT(flipTriangleVAction()));

	connect(m_moveLeftButton, SIGNAL(pressed()), this, SLOT(moveTriangleLeftAction()));
	connect(m_moveRightButton, SIGNAL(pressed()), this, SLOT(moveTriangleRightAction()));
	connect(m_moveUpButton, SIGNAL(pressed()), this, SLOT(moveTriangleUpAction()));
	connect(m_moveDownButton, SIGNAL(pressed()), this, SLOT(moveTriangleDownAction()));

	connect(m_sceneAxesSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(sceneAxesSelected(int)));
	connect(m_selectionItemsSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectionItemsChangedAction(int)));

	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(selectToolAction(int)));
	connect(editor, SIGNAL(editModeChangedSignal(FigureEditor::EditMode)), this, SLOT(selectToolAction(FigureEditor::EditMode)));
}

void EditTriangleWidget::closeEvent(QCloseEvent* /*e*/)
{
	logInfo("EditTriangleWidget::closeEvent : saving settings");
	QSettings settings;
	settings.beginGroup("edittrianglewidget");
	settings.setValue("movemultiplier", m_multiplierEditor->value());
}

void EditTriangleWidget::triangleScaleUpAction()
{
	double scale = (100.0 + m_scaleEditor->value()) / 100.0;
	if (scale == 0.0 || scale == 1.0)
		return;
	if (editor->hasSelection())
		editor->scaleSelection(scale, scale, editor->selectionTransformPos());
	else if (editor->postEnabled())
		editor->scaleTriangle(editor->post(), scale, scale, editor->triangleTransformPos());
	else
		editor->scaleTriangle(selectedTriangle, scale, scale, editor->triangleTransformPos());
}

void EditTriangleWidget::triangleScaleDownAction()
{
	double scale = (100.0 - m_scaleEditor->value()) / 100.0;
	if (scale == 0.0 || scale == 1.0)
		return;
	if (editor->hasSelection())
		editor->scaleSelection(scale, scale, editor->selectionTransformPos());
	else if (editor->postEnabled())
		editor->scaleTriangle(editor->post(), scale, scale, editor->triangleTransformPos());
	else
		editor->scaleTriangle(selectedTriangle, scale, scale, editor->triangleTransformPos());
}

void EditTriangleWidget::triangleRotateAction()
{
	double value = m_rotateEditor->value();
	double dx = value - lastRotateValue;

	if (dx == 0.0)
		return;

	if (editor->hasSelection())
		editor->rotateSelection(dx, editor->selectionTransformPos());
	else if (editor->postEnabled())
		editor->rotateTriangle(editor->post(), dx, editor->triangleTransformPos());
	else
		editor->rotateTriangle(selectedTriangle, dx, editor->triangleTransformPos());

	lastRotateValue = value;
}

void EditTriangleWidget::triangleRotateCCWAction()
{
	double deg = qAbs(m_rotateEditor->value());
	if (deg == 0.0)
		return;
	if (editor->hasSelection())
		editor->rotateSelection(deg, editor->selectionTransformPos());
	else if (editor->postEnabled())
		editor->rotateTriangle(editor->post(), deg, editor->triangleTransformPos());
	else
		editor->rotateTriangle(selectedTriangle, deg, editor->triangleTransformPos());

}

void EditTriangleWidget::triangleRotateCWAction()
{
	double deg = qAbs(m_rotateEditor->value()) * -1.0;
	if (deg == 0.0)
		return;
	if (editor->hasSelection())
		editor->rotateSelection(deg, editor->selectionTransformPos());
	else if (editor->postEnabled())
		editor->rotateTriangle(editor->post(), deg, editor->triangleTransformPos());
	else
		editor->rotateTriangle(selectedTriangle, deg, editor->triangleTransformPos());

}


void EditTriangleWidget::triangleSelectedSlot(Triangle* t)
{
	selectedTriangle = t;
	reset();
}


void EditTriangleWidget::moveTriangleLeftAction()
{
	// The move buttons have their own "auto-repeat" loops
	// because setting autoRepeat true for the buttons toggles the
	// isDown state programatically.
	while (m_moveLeftButton->isDown())
	{
		if (editor->hasSelection())
			editor->moveSelectionBy(m_multiplierEditor->value()*(-1.0), 0.0);
		else if (editor->postEnabled())
			editor->moveTriangleBy(editor->post(), m_multiplierEditor->value()*(-1.0), 0.0);
		else
			editor->moveTriangleBy(selectedTriangle, m_multiplierEditor->value()*(-1.0), 0.0);
		QCoreApplication::processEvents();
		usleep(100000);
		QCoreApplication::processEvents();
		usleep(100000);
		QCoreApplication::processEvents();
	}
	emit undoStateSignal();
}


void EditTriangleWidget::moveTriangleRightAction()
{
	while (m_moveRightButton->isDown())
	{
		if (editor->hasSelection())
			editor->moveSelectionBy(m_multiplierEditor->value(),0.0);
		else if (editor->postEnabled())
			editor->moveTriangleBy(editor->post(), m_multiplierEditor->value(), 0.0);
		else
			editor->moveTriangleBy(selectedTriangle, m_multiplierEditor->value(),0.0);
		QCoreApplication::processEvents();
		usleep(100000);
		QCoreApplication::processEvents();
		usleep(100000);
		QCoreApplication::processEvents();
	}
	emit undoStateSignal();
}



void EditTriangleWidget::moveTriangleUpAction()
{
	while (m_moveUpButton->isDown())
	{
		if (editor->hasSelection())
			editor->moveSelectionBy(0.0, m_multiplierEditor->value());
		else if (editor->postEnabled())
			editor->moveTriangleBy(editor->post(), 0.0, m_multiplierEditor->value());
		else
			editor->moveTriangleBy(selectedTriangle, 0.0, m_multiplierEditor->value());
		QCoreApplication::processEvents();
		usleep(100000);
		QCoreApplication::processEvents();
		usleep(100000);
		QCoreApplication::processEvents();
	}
	emit undoStateSignal();
}



void EditTriangleWidget::moveTriangleDownAction()
{
	while (m_moveDownButton->isDown())
	{
		if (editor->hasSelection())
			editor->moveSelectionBy(0.0, m_multiplierEditor->value()*(-1.0));
		else if (editor->postEnabled())
			editor->moveTriangleBy(editor->post(), 0.0, m_multiplierEditor->value()*(-1.0));
		else
			editor->moveTriangleBy(selectedTriangle, 0.0, m_multiplierEditor->value()*(-1.0));
		QCoreApplication::processEvents();
		usleep(100000);
		QCoreApplication::processEvents();
		usleep(100000);
		QCoreApplication::processEvents();
	}
	emit undoStateSignal();
}


void EditTriangleWidget::rotateTextEditedSlot()
{
	double deg = m_rotateEditor->value();
	if (deg == 0.0)
		return;

	if (editor->hasSelection())
		editor->rotateSelection(deg, editor->selectionTransformPos());
	else if (editor->postEnabled())
		editor->rotateTriangle(editor->post(), deg, editor->triangleTransformPos());
	else
		editor->rotateTriangle(selectedTriangle, deg, editor->triangleTransformPos());
}


void EditTriangleWidget::toggleMarkAction(bool flag)
{
	editor->setMarkVisible(flag);
}


void EditTriangleWidget::selectionItemsChangedAction(int idx)
{
	TriangleSelection* selection = editor->selection();
	if (idx == 0)
		selection->setSelectedType(Triangle::RTTI);
	else
		selection->setSelectedType(NodeItem::RTTI);

	if (selection->isVisible())
	{
		selection->clear();
		selection->selectCoveredItems();
	}
}

void EditTriangleWidget::reset()
{
	m_rotateEditor->updateValue(0.0);
	lastRotateValue = 0.0;
}

void EditTriangleWidget::selectToolAction(int idx)
{
	selectToolAction((FigureEditor::EditMode)idx);
}

void EditTriangleWidget::selectToolAction(FigureEditor::EditMode idx)
{
	if (idx != editor->mode())
	{
		editor->setMode(idx);
		reset();
	}
	else
	{
		tabWidget->blockSignals(true);
		tabWidget->setCurrentIndex((int)idx);
		tabWidget->blockSignals(false);
	}
}

void EditTriangleWidget::sceneAxesSelected(int idx)
{
	idx++; // FigureEditor::SceneLocation enum contains plus one item
	toggleMarkAction(idx == (int)FigureEditor::Mark);
	editor->setTransformLocation((FigureEditor::SceneLocation)idx);
	editor->update();
	reset();
}
