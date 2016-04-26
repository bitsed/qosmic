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

#include "editmodeselectorwidget.h"
#include "adjustscenewidget.h"
#include "logger.h"

EditModeSelectorWidget::EditModeSelectorWidget(FigureEditor* e, QWidget* parent)
: QWidget(parent), m_xfeditor(e)
{
	setupUi(this);

	m_buttonGroup.addButton(m_moveButton, FigureEditor::Move);
	m_buttonGroup.addButton(m_rotateButton, FigureEditor::Rotate);
	m_buttonGroup.addButton(m_scaleButton, FigureEditor::Scale);
	m_buttonGroup.addButton(m_flipButton, FigureEditor::Flip);

	connect(&m_buttonGroup, SIGNAL(buttonPressed(int)), this, SLOT(groupButtonPressedSlot(int)));

	// restore (Double/Int)ValueEditor settings
	m_multiplierEditor->restoreSettings();
	m_rotateEditor->restoreSettings();
	m_scaleEditor->restoreSettings();

	reset();

	QSettings settings;
	settings.beginGroup("editmodselectorwidget");
	m_multiplierEditor->updateValue(settings.value("movemultiplier", 1.0).toDouble());

	// set the dropdown menu index for the triangle selection type
	int selection_type = m_xfeditor->selection()->selectedType();
	if (selection_type == Triangle::Type)
		m_selectionItemsSelector->setCurrentIndex(0);
	else
		m_selectionItemsSelector->setCurrentIndex(1);

	// restore the edit mode button state
	setSelectedButton(m_xfeditor->mode());

	// set the dropdown menu index for the scene editing origin
	m_sceneAxesSelector->setCurrentIndex((int)m_xfeditor->transformLocation() - 1);

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

	connect(m_moveLeftButton, SIGNAL(pressed()), this, SLOT(moveTriangleLeftAction()));
	connect(m_moveLeftButton, SIGNAL(released()), this, SLOT(buttonReleasedSlot()));

	connect(m_moveRightButton, SIGNAL(pressed()), this, SLOT(moveTriangleRightAction()));
	connect(m_moveRightButton, SIGNAL(released()), this, SLOT(buttonReleasedSlot()));

	connect(m_moveUpButton, SIGNAL(pressed()), this, SLOT(moveTriangleUpAction()));
	connect(m_moveUpButton, SIGNAL(released()), this, SLOT(buttonReleasedSlot()));

	connect(m_moveDownButton, SIGNAL(pressed()), this, SLOT(moveTriangleDownAction()));
	connect(m_moveDownButton, SIGNAL(released()), this, SLOT(buttonReleasedSlot()));

	connect(m_sceneAxesSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(sceneAxesSelected(int)));
	connect(m_selectionItemsSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectionItemsChangedAction(int)));

	connect(m_hFlipButton, SIGNAL(pressed()), m_xfeditor, SLOT(flipTriangleHAction()));
	connect(m_vFlipButton, SIGNAL(pressed()), m_xfeditor, SLOT(flipTriangleVAction()));

	connect(m_postTriangleButton, SIGNAL(clicked(bool)), m_xfeditor, SLOT(editPostTriangle(bool)));
	connect(m_resetTriangleButton, SIGNAL(pressed()), m_xfeditor, SLOT(resetTriangleCoordsAction()));
	connect(m_addTriangleButton, SIGNAL(pressed()), m_xfeditor, SLOT(addTriangleAction()));
	connect(m_delTriangleButton, SIGNAL(pressed()), m_xfeditor, SLOT(removeTriangleAction()));

	connect(m_xfeditor, SIGNAL(editModeChangedSignal(FigureEditor::EditMode)), this, SLOT(setSelectedButton(FigureEditor::EditMode)));
	connect(sceneScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(sceneScaledSlot()));
	connect(sceneCenterSelector, SIGNAL(activated(int)), this, SLOT(sceneCenteredSlot(int)));
	connect(sceneConfigButton, SIGNAL(clicked(bool)), this, SLOT(sceneConfigSlot()));
	connect(autoScaleButton, SIGNAL(clicked(bool)), m_xfeditor, SLOT(autoScale()));
	connect(m_xfeditor, SIGNAL(triangleSelectedSignal(Triangle*)), this, SLOT(triangleSelectedSlot(Triangle*)));

	connect(this, SIGNAL(buttonPressed(FigureEditor::EditMode)), m_xfeditor, SLOT(setMode(FigureEditor::EditMode)));
}

void EditModeSelectorWidget::setSelectedButton(FigureEditor::EditMode id)
{
	if (m_buttonGroup.checkedId() != id)
	{
		blockSignals(true);
		m_buttonGroup.button(id)->setChecked(true);
		m_stackedWidget->setCurrentIndex(id);
		blockSignals(false);
	}
}

int EditModeSelectorWidget::selectedButton()
{
	return m_buttonGroup.checkedId();
}

void EditModeSelectorWidget::groupButtonPressedSlot(int id)
{

	emit buttonPressed((FigureEditor::EditMode)id);
}

void EditModeSelectorWidget::closeEvent(QCloseEvent* /*e*/)
{
	logInfo("EditModeSelectorWidget::closeEvent : saving settings");
	QSettings settings;
	settings.beginGroup("editmodeselectorwidget");
	settings.setValue("movemultiplier", m_multiplierEditor->value());
}

void EditModeSelectorWidget::triangleScaleUpAction()
{
	double scale = (100.0 + m_scaleEditor->value()) / 100.0;
	if (scale == 0.0 || scale == 1.0)
		return;
	if (m_xfeditor->hasSelection())
		m_xfeditor->scaleSelection(scale, scale, m_xfeditor->selectionTransformPos());
	else if (m_xfeditor->postEnabled())
		m_xfeditor->scaleTriangle(m_xfeditor->post(), scale, scale, m_xfeditor->triangleTransformPos());
	else
		m_xfeditor->scaleTriangle(m_xfeditor->getSelectedTriangle(), scale, scale, m_xfeditor->triangleTransformPos());
}

void EditModeSelectorWidget::triangleScaleDownAction()
{
	double scale = (100.0 - m_scaleEditor->value()) / 100.0;
	if (scale == 0.0 || scale == 1.0)
		return;
	if (m_xfeditor->hasSelection())
		m_xfeditor->scaleSelection(scale, scale, m_xfeditor->selectionTransformPos());
	else if (m_xfeditor->postEnabled())
		m_xfeditor->scaleTriangle(m_xfeditor->post(), scale, scale, m_xfeditor->triangleTransformPos());
	else
		m_xfeditor->scaleTriangle(m_xfeditor->getSelectedTriangle(), scale, scale, m_xfeditor->triangleTransformPos());
}

void EditModeSelectorWidget::triangleRotateAction()
{
	double value = m_rotateEditor->value();
	double dx = value - lastRotateValue;

	if (dx == 0.0)
		return;

	if (m_xfeditor->hasSelection())
		m_xfeditor->rotateSelection(dx, m_xfeditor->selectionTransformPos());
	else if (m_xfeditor->postEnabled())
		m_xfeditor->rotateTriangle(m_xfeditor->post(), dx, m_xfeditor->triangleTransformPos());
	else
		m_xfeditor->rotateTriangle(m_xfeditor->getSelectedTriangle(), dx, m_xfeditor->triangleTransformPos());

	lastRotateValue = value;
}

void EditModeSelectorWidget::triangleRotateCCWAction()
{
	double deg = qAbs(m_rotateEditor->value());
	if (deg == 0.0)
		return;
	if (m_xfeditor->hasSelection())
		m_xfeditor->rotateSelection(deg, m_xfeditor->selectionTransformPos());
	else if (m_xfeditor->postEnabled())
		m_xfeditor->rotateTriangle(m_xfeditor->post(), deg, m_xfeditor->triangleTransformPos());
	else
		m_xfeditor->rotateTriangle(m_xfeditor->getSelectedTriangle(), deg, m_xfeditor->triangleTransformPos());

}

void EditModeSelectorWidget::triangleRotateCWAction()
{
	double deg = qAbs(m_rotateEditor->value()) * -1.0;
	if (deg == 0.0)
		return;
	if (m_xfeditor->hasSelection())
		m_xfeditor->rotateSelection(deg, m_xfeditor->selectionTransformPos());
	else if (m_xfeditor->postEnabled())
		m_xfeditor->rotateTriangle(m_xfeditor->post(), deg, m_xfeditor->triangleTransformPos());
	else
		m_xfeditor->rotateTriangle(m_xfeditor->getSelectedTriangle(), deg, m_xfeditor->triangleTransformPos());

}


void EditModeSelectorWidget::triangleSelectedSlot(Triangle* t)
{
	selectedTriangle = t;
	reset();
}


// Call this slot when the move buttons are released(), and emit an
// undo signal when the button is no longer pressed.
void EditModeSelectorWidget::buttonReleasedSlot()
{
	if (QApplication::mouseButtons() == Qt::NoButton)
		emit undoStateSignal();
}


void EditModeSelectorWidget::moveTriangleLeftAction()
{
	if (m_xfeditor->hasSelection())
		m_xfeditor->moveSelectionBy(m_multiplierEditor->value()*(-1.0), 0.0);
	else if (m_xfeditor->postEnabled())
		m_xfeditor->moveTriangleBy(m_xfeditor->post(), m_multiplierEditor->value()*(-1.0), 0.0);
	else
		m_xfeditor->moveTriangleBy(m_xfeditor->getSelectedTriangle(), m_multiplierEditor->value()*(-1.0), 0.0);
}


void EditModeSelectorWidget::moveTriangleRightAction()
{
	if (m_xfeditor->hasSelection())
		m_xfeditor->moveSelectionBy(m_multiplierEditor->value(),0.0);
	else if (m_xfeditor->postEnabled())
		m_xfeditor->moveTriangleBy(m_xfeditor->post(), m_multiplierEditor->value(), 0.0);
	else
		m_xfeditor->moveTriangleBy(m_xfeditor->getSelectedTriangle(), m_multiplierEditor->value(),0.0);
}


void EditModeSelectorWidget::moveTriangleUpAction()
{
	if (m_xfeditor->hasSelection())
		m_xfeditor->moveSelectionBy(0.0, m_multiplierEditor->value());
	else if (m_xfeditor->postEnabled())
		m_xfeditor->moveTriangleBy(m_xfeditor->post(), 0.0, m_multiplierEditor->value());
	else
		m_xfeditor->moveTriangleBy(m_xfeditor->getSelectedTriangle(), 0.0, m_multiplierEditor->value());
}


void EditModeSelectorWidget::moveTriangleDownAction()
{
	if (m_xfeditor->hasSelection())
		m_xfeditor->moveSelectionBy(0.0, m_multiplierEditor->value()*(-1.0));
	else if (m_xfeditor->postEnabled())
		m_xfeditor->moveTriangleBy(m_xfeditor->post(), 0.0, m_multiplierEditor->value()*(-1.0));
	else
		m_xfeditor->moveTriangleBy(m_xfeditor->getSelectedTriangle(), 0.0, m_multiplierEditor->value()*(-1.0));
}


void EditModeSelectorWidget::rotateTextEditedSlot()
{
	double deg = m_rotateEditor->value();
	if (deg == 0.0)
		return;

	if (m_xfeditor->hasSelection())
		m_xfeditor->rotateSelection(deg, m_xfeditor->selectionTransformPos());
	else if (m_xfeditor->postEnabled())
		m_xfeditor->rotateTriangle(m_xfeditor->post(), deg, m_xfeditor->triangleTransformPos());
	else
		m_xfeditor->rotateTriangle(m_xfeditor->getSelectedTriangle(), deg, m_xfeditor->triangleTransformPos());
}


void EditModeSelectorWidget::toggleMarkAction(bool flag)
{
	m_xfeditor->setMarkVisible(flag);
}


void EditModeSelectorWidget::selectionItemsChangedAction(int idx)
{
	TriangleSelection* selection = m_xfeditor->selection();
	if (idx == 0)
		selection->setSelectedType(Triangle::Type);
	else
		selection->setSelectedType(NodeItem::Type);

	if (selection->isVisible())
	{
		selection->clear();
		selection->selectCoveredItems();
	}
}

void EditModeSelectorWidget::reset()
{
	m_rotateEditor->updateValue(0.0);
	lastRotateValue = 0.0;
	m_postTriangleButton->setChecked(m_xfeditor->postEnabled());
}


void EditModeSelectorWidget::sceneAxesSelected(int idx)
{
	idx++; // FigureEditor::SceneLocation enum contains plus one item
	toggleMarkAction(idx == (int)FigureEditor::Mark);
	m_xfeditor->setTransformLocation((FigureEditor::SceneLocation)idx);
	m_xfeditor->update();
	reset();
}

void EditModeSelectorWidget::sceneScaledSlot()
{
	int value = sceneScaleSlider->dx();
	Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
	double scale;
	if (value > 0)
	{
		if (mods & Qt::ShiftModifier)
			scale = 1.01;
		else if (mods & Qt::ControlModifier)
			scale = 1.10;
		else
			scale = 1.05;
	}
	else if (value < 0)
	{
		if (mods & Qt::ShiftModifier)
			scale = 0.99;
		else if (mods & Qt::ControlModifier)
			scale = 0.9090;
		else
			scale = 0.9523;
	}
	else
		return;

	m_xfeditor->scaleBasis(scale, scale);
}

void EditModeSelectorWidget::sceneCenteredSlot(int idx)
{
	m_xfeditor->setCenteredScaling(static_cast<FigureEditor::SceneLocation>(idx));
}

void EditModeSelectorWidget::sceneConfigSlot()
{
	AdjustSceneWidget dialog(m_xfeditor, this);
	dialog.move(QCursor::pos() + QPoint(0, -dialog.height()));
	dialog.exec();
}

