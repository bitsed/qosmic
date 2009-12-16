/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009 by David Bitseff                       *
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
#include "xfedit.h"
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

	m_transformPosGroup = new QButtonGroup(this);
	m_transformPosGroup->addButton(m_originButton, Origin);
	m_transformPosGroup->addButton(m_triangleButton, Center);
	m_transformPosGroup->addButton(m_markButton,  Mark);
	m_transformPosGroup->addButton(m_nodeXButton, NodeX);
	m_transformPosGroup->addButton(m_nodeYButton, NodeY);
	m_transformPosGroup->addButton(m_nodeOButton, NodeO);

	m_selectionTypeGroup = new QButtonGroup(this);
	m_selectionTypeGroup->addButton(m_selectTrianglesButton, Triangle::RTTI);
	m_selectionTypeGroup->addButton(m_selectNodesButton, NodeItem::RTTI);

	reset();

	connect(m_rotateSlider, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_scaleSlider, SIGNAL(undoStateSignal()), this, SIGNAL(undoStateSignal()));
	connect(m_rotateSlider, SIGNAL(valueChanged(int)), this, SLOT(triangleRotatedAction()));
	connect(m_scaleSlider, SIGNAL(valueChanged(int)), this, SLOT(triangleScaledAction()));

	connect(m_hFlipButton, SIGNAL(released()), this, SIGNAL(undoStateSignal()));
	connect(m_hFlipButton, SIGNAL(pressed()), this, SLOT(triangleHFlippedAction()));

	connect(m_vFlipButton, SIGNAL(released()), this, SIGNAL(undoStateSignal()));
	connect(m_vFlipButton, SIGNAL(pressed()), this, SLOT(triangleVFlippedAction()));

	connect(m_moveLeftButton, SIGNAL(pressed()), this, SLOT(moveTriangleLeftAction()));
	connect(m_moveRightButton, SIGNAL(pressed()), this, SLOT(moveTriangleRightAction()));
	connect(m_moveUpButton, SIGNAL(pressed()), this, SLOT(moveTriangleUpAction()));
	connect(m_moveDownButton, SIGNAL(pressed()), this, SLOT(moveTriangleDownAction()));

	connect(m_rotateLineEdit, SIGNAL(returnPressed()), this, SLOT(rotateTextEditedSlot()));
	connect(m_scaleLineEdit, SIGNAL(returnPressed()), this, SLOT(scaleTextEditedSlot()));
	connect(m_markButton, SIGNAL(toggled(bool)), this, SLOT(toggleMarkAction(bool)));

	connect(m_selectTrianglesButton, SIGNAL(released()), this, SLOT(selectedItemsChangedAction()));
	connect(m_selectNodesButton, SIGNAL(released()), this, SLOT(selectedItemsChangedAction()));

	QSettings settings;
	settings.beginGroup("edittrianglewidget");
	int id = settings.value("transformposition", Origin).toInt();
	m_transformPosGroup->button(id)->setChecked(true);
}

void EditTriangleWidget::closeEvent(QCloseEvent* /*e*/)
{
	logInfo("EditTriangleWidget::closeEvent : saving settings");
	QSettings settings;
	settings.beginGroup("edittrianglewidget");
	settings.setValue("transformposition", m_transformPosGroup->checkedId());
}

void EditTriangleWidget::triangleScaledAction()
{
	int value = m_scaleSlider->dx();
	Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
	double scale;
	if (value > 0)
	{
		if (mods & Qt::ShiftModifier)
		{
			scale = 1.01;
			scale_dx *= 1.01;
		}
		else if (mods & Qt::ControlModifier)
		{
			scale = 1.10;
			scale_dx *= 1.10;
		}
		else
		{
			scale = 1.05;
			scale_dx *= 1.05;
		}
	}
	else if (value < 0)
	{
		if (mods & Qt::ShiftModifier)
		{
			scale = 0.99;
			scale_dx *= 0.99;
		}
		else if (mods & Qt::ControlModifier)
		{
			scale = 0.9090;
			scale_dx *= 0.9090;
		}
		else
		{
			scale = 0.9523;
			scale_dx *= 0.9523;
		}
	}
	else
		return;

	if (editor->hasSelection())
		editor->scaleSelection(scale, scale, selectionTransformPos());
	else if (editor->postEnabled())
		editor->scaleTriangle(editor->post(), scale, scale, triangleTransformPos());
	else
		editor->scaleTriangle(selectedTriangle, scale, scale, triangleTransformPos());

	m_scaleLineEdit->setText(QLocale().toString(scale_dx, 'f', 3));
}


void EditTriangleWidget::triangleRotatedAction()
{
	int value = m_rotateSlider->dx();
	if (value == 0)
		return;

	double div = 16.0;
	Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
	if (mods & Qt::ShiftModifier)
		div = 64.0;
	else if (mods & Qt::ControlModifier)
		div = 4.0;

	double deg = -1.0 * value / div;
	rotate_dx += deg;
	while (rotate_dx > 360.0)
		rotate_dx -= 360.0;
	while (rotate_dx < -360.0)
		rotate_dx += 360.0;
	m_rotateLineEdit->setText(QLocale().toString(-1.0 * rotate_dx, 'f', 1));

	if (editor->hasSelection())
		editor->rotateSelection(deg, selectionTransformPos());
	else if (editor->postEnabled())
		editor->rotateTriangle(editor->post(), deg, triangleTransformPos());
	else
		editor->rotateTriangle(selectedTriangle, deg, triangleTransformPos());
}

void EditTriangleWidget::triangleHFlippedAction()
{
	if (editor->hasSelection())
		editor->flipSelectionHAction(selectionTransformPos());
	else if (editor->postEnabled())
		editor->flipTriangleHAction(editor->post(), triangleTransformPos());
	else
		editor->flipTriangleHAction(selectedTriangle, triangleTransformPos());
}

void EditTriangleWidget::triangleVFlippedAction()
{
	if (editor->hasSelection())
		editor->flipSelectionVAction(selectionTransformPos());
	else if (editor->postEnabled())
		editor->flipTriangleVAction(editor->post(), triangleTransformPos());
	else
		editor->flipTriangleVAction(selectedTriangle, triangleTransformPos());
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
	double deg = m_rotateLineEdit->text().toDouble();
	if (deg == 0.0)
		return;

	if (editor->hasSelection())
		editor->rotateSelection(deg, selectionTransformPos());
	else if (editor->postEnabled())
		editor->rotateTriangle(editor->post(), deg, triangleTransformPos());
	else
		editor->rotateTriangle(selectedTriangle, deg, triangleTransformPos());

	rotate_dx = 0.0;
}


void EditTriangleWidget::scaleTextEditedSlot()
{
	double scale = m_scaleLineEdit->text().toDouble();
	if (scale == 1.0)
		return;

	if (editor->hasSelection())
		editor->scaleSelection(scale, scale, selectionTransformPos());
	else if (editor->postEnabled())
		editor->scaleTriangle(editor->post(), scale, scale, triangleTransformPos());
	else
		editor->scaleTriangle(selectedTriangle, scale, scale, triangleTransformPos());

	scale_dx = 0.0;
}


void EditTriangleWidget::toggleMarkAction(bool flag)
{
	editor->setMarkVisible(flag);
}


QPointF EditTriangleWidget::triangleTransformPos()
{
	Triangle* t = selectedTriangle;
	QPointF pos;

	if (editor->postEnabled())
		t = editor->post();

	if (m_triangleButton->isChecked())
	// get the triangle's center position from the scene
		pos = t->mapFromScene(t->sceneBoundingRect().center());
	else if (m_markButton->isChecked())
		// center on the marker
		pos = t->mapFromScene(editor->mark()->center());
	else if (m_nodeOButton->isChecked())
		pos = t->getCoords().at(0);
	else if (m_nodeXButton->isChecked())
		pos = t->getCoords().at(1);
	else if (m_nodeYButton->isChecked())
		pos = t->getCoords().at(2);

	return pos;
}


QPointF EditTriangleWidget::selectionTransformPos()
{
	QPointF pos(0.0, 0.0);
	if (m_triangleButton->isChecked())
		pos = editor->selection()->mapFromScene(
				editor->selection()->sceneBoundingRect().center());
	else if (m_markButton->isChecked())
		pos = editor->selection()->mapFromScene(editor->mark()->center());
	else
		pos = editor->selection()->mapFromScene(pos);

	return pos;
}


void EditTriangleWidget::selectedItemsChangedAction()
{
	TriangleSelection* selection = editor->selection();
	selection->setSelectedType(m_selectionTypeGroup->checkedId());

	if (selection->isVisible())
	{
		selection->clear();
		selection->selectCoveredItems();
	}
}


void EditTriangleWidget::reset()
{
	rotate_dx = 0.0;
	scale_dx  = 1.0;
	QLocale l;
	m_rotateLineEdit->setText(l.toString(0.0, 'f', 1));
	m_scaleLineEdit->setText(l.toString(1.0, 'f', 3));
	m_selectionTypeGroup->button(editor->selection()->selectedType())->setChecked(true);
	m_rotateSlider->setToSnap();
	m_scaleSlider->setToSnap();
}

