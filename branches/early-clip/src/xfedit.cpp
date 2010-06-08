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
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QScrollBar>
#include <QSettings>
#include <math.h>

#include "xfedit.h"
#include "edittrianglewidget.h"
#include "adjustscenewidget.h"
#include "logger.h"

#define genome_ptr (genomes->selectedGenome())

/**
 * The QGraphicsScene that manages the Triangles.
 */
FigureEditor::FigureEditor(GenomeVector* g, QObject* parent)
	: QGraphicsScene(parent), QosmicWidget(this, "FigureEditor"),
	moving(0), genomes(g), selectedTriangle(0), postTriangle(0),
	centered_scaling(None), transform_location(Origin), editMode(Move),
	move_edge_mode(false), has_selection(false), is_selecting(false),
	editing_post(false), menu_visible(false)
{
	QMatrix b(100.0, 0.0, 0.0, -100.0, 0.0, 0.0);
	basisTriangle = new BasisTriangle(b);

	QSettings settings;
	settings.beginGroup("figureeditor");
	QVariant v(settings.value("basis"));
	if (!v.isNull() && v.convert(QVariant::Matrix))
		setCoordTransform(v.value<QMatrix>());

	popupMenu = new QMenu(tr("Edit Triangle"));
	cutAction = new QAction(QIcon(":icons/silk/cut.xpm"),
		tr("Cut"),this);
	copyAction = new QAction(QIcon(":icons/silk/page_copy.xpm"),
		tr("Copy"),this);
	pasteAction = new QAction(QIcon(":icons/silk/page_paste.xpm"),
		tr("Paste"),this);
	addAction = new QAction(QIcon(":icons/silk/shape_square_add.xpm"),
		tr("Add"),this);
	resAction = new QAction(QIcon(":icons/silk/shape_square_go.xpm"),
		tr("Reset Position"),this);
	flipHAction = new QAction(QIcon(":icons/silk/shape_flip_horizontal.xpm"),
		tr("Flip Horizontal"),this);
	flipVAction = new QAction(QIcon(":icons/silk/shape_flip_vertical.xpm"),
		tr("Flip Vertical"),this);
	rescaleAction = new QAction(QIcon(":icons/silk/shape_group.xpm"),
		tr("Rescale View"),this);

	triangleMenu = popupMenu->addMenu(tr("Triangles"));
	popupMenu->addSeparator();
	popupMenu->addAction(cutAction);
	popupMenu->addAction(copyAction);
	popupMenu->addAction(pasteAction);
	popupMenu->addAction(addAction);
	popupMenu->addSeparator();
	popupMenu->addAction(flipHAction);
	popupMenu->addAction(flipVAction);
	popupMenu->addAction(resAction);
	popupMenu->addSeparator();
	popupMenu->addAction(rescaleAction);

	preview_density = settings.value("previewdensity", 10).toInt();
	preview_visible = settings.value("previewvisible", false).toBool();
	grid_visible = settings.value("gridvisible", true).toBool();
	grid_color = QColor(settings.value("gridcolor", "#999999").toString());
	guide_visible = settings.value("guidevisible", true).toBool();
	guide_color = QColor(settings.value("guidecolor", "#ee0000").toString());
	bg_color   = QColor(settings.value("bgcolor", "#000000").toString());
	centered_scaling = (SceneLocation)settings.value("centeredscaling", false).toInt();

	setBackgroundBrush(Qt::black);
	basisTriangle->setGraphicsScene(this);
	addItem(basisTriangle);

	infoItem = new QGraphicsSimpleTextItem();
	addItem(infoItem);

	selectionItem = new TriangleSelection(this, basisTriangle);
	selectionItem->setPen(QPen(QBrush(Qt::gray), 1, Qt::DashLine));
	selectionItem->setZValue(-1);
	selectionItem->setVisible(false);
	selectionItem->setSelectedType(settings.value("selectiontype", Triangle::RTTI).toInt());
	addItem(selectionItem);

	bLabelA = new QGraphicsSimpleTextItem(QString("O"));
	bLabelA->setPos(basisTriangle->polygon().at(0) + QPointF(-10.0,0.0));
	bLabelA->setBrush(QBrush(Qt::gray));
	bLabelA->setZValue(0);
	addItem(bLabelA);

	bLabelB = new QGraphicsSimpleTextItem(QString("X"));
	bLabelB->setPos(basisTriangle->polygon().at(0) + QPointF(-10.0,0.0));
	bLabelB->setBrush(QBrush(Qt::gray));
	bLabelB->setZValue(0);
	addItem(bLabelB);

	bLabelC = new QGraphicsSimpleTextItem(QString("Y"));
	bLabelC->setPos(basisTriangle->polygon().at(0) + QPointF(-10.0,0.0));
	bLabelC->setBrush(QBrush(Qt::gray));
	bLabelC->setZValue(0);
	addItem(bLabelC);

	coordinateMark = new CoordinateMark();
	coordinateMark->setPen(QPen(Qt::gray));
	addItem(coordinateMark);
	coordinateMark->centerOn(QPointF(0.0,0.0));
	coordinateMark->setVisible(false);
	coordinateMark->setZValue(0);

	connect(triangleMenu, SIGNAL(triggered(QAction*)), this, SLOT(triangleMenuAction(QAction*)));
	connect(addAction, SIGNAL(triggered()), this, SLOT(addTriangleAction()));
	connect(cutAction, SIGNAL(triggered()), this, SLOT(cutTriangleAction()));
	connect(copyAction, SIGNAL(triggered()), this, SLOT(copyTriangleAction()));
	connect(pasteAction, SIGNAL(triggered()), this, SLOT(pasteTriangleAction()));
	connect(resAction, SIGNAL(triggered()), this, SLOT(resetTriangleCoordsAction()));
	connect(flipHAction, SIGNAL(triggered()), this, SLOT(flipTriangleHPopupAction()));
	connect(flipVAction, SIGNAL(triggered()), this, SLOT(flipTriangleVPopupAction()));
	connect(rescaleAction, SIGNAL(triggered()), this, SLOT(autoScale()));
}

FigureEditor::~FigureEditor()
{
	delete selectionItem;
	delete postTriangle;
	delete infoItem;
	delete bLabelA;
	delete bLabelB;
	delete bLabelC;
	delete coordinateMark;
	delete popupMenu;
	delete triangleMenu;
	delete addAction;
	delete resAction;
	delete flipHAction;
	delete flipVAction;
	delete rescaleAction;
	delete basisTriangle;
}

void FigureEditor::writeSettings()
{
	QSettings settings;
	settings.beginGroup("figureeditor");
	settings.setValue("basis", QVariant::fromValue(transform()));
	settings.setValue("previewdensity", preview_density);
	settings.setValue("previewvisible", preview_visible);
	settings.setValue("gridvisible", grid_visible);
	settings.setValue("gridcolor", grid_color.name());
	settings.setValue("guidevisible", guide_visible);
	settings.setValue("guidecolor", guide_color.name());
	settings.setValue("bgcolor", bg_color.name());
	settings.setValue("centeredscaling", centered_scaling);
	settings.setValue("selectiontype", selectionItem->selectedType());
}

void FigureEditor::enableFinalXform(bool enable)
{
	if (enable)
	{
		if (genome_ptr->final_xform_enable == 1)
			selectTriangle(trianglesList.last());
		else
		{
			logInfo("FigureEditor::finalStateChangedSlot : enabling final xform");
			flam3_add_xforms(genome_ptr, 1, 0, 1);
			reset();
			selectTriangle(trianglesList.last());
			emit triangleModifiedSignal(selectedTriangle);
			emit undoStateSignal();
		}
	}
	else
	{
		if (genome_ptr->final_xform_enable == 0)
			logWarn("FigureEditor::finalStateChangedSlot : final xform already disabled");
		else
		{
			logInfo("FigureEditor::finalStateChangedSlot : disabling final xform");
			flam3_delete_xform(genome_ptr, genome_ptr->final_xform_index);
			reset();
			selectTriangle(trianglesList.last());
			emit triangleModifiedSignal(selectedTriangle);
			emit undoStateSignal();
		}
	}
}

void FigureEditor::addTriangleAction()
{
	int n = genome_ptr->num_xforms;
	Util::add_default_xform(genome_ptr);
	logFine(QString("FigureEditor::addTriangleAction : adding triangle to slot %1")
		.arg(n));
	reset();
	logFine(QString("FigureEditor::addTriangleAction : %1 items in triangleList")
		.arg(trianglesList.size()));
	logFine(QString("FigureEditor::addTriangleAction : genome has %1 xforms")
		.arg( genome_ptr->num_xforms ));
	if (genome_ptr->final_xform_enable == 1)
		selectTriangle(trianglesList.size() - 2);
	else
		selectTriangle(trianglesList.last());
	emit triangleModifiedSignal(selectedTriangle);
	emit undoStateSignal();
}

void FigureEditor::cutTriangleAction()
{
	copyTriangleAction();
	if (has_selection && selectionItem->hasItems())
	{
		TriangleList triangles;
		if (selectionItem->selectedType() == NodeItem::RTTI)
		{
			foreach (NodeItem* node, selectionItem->nodes())
				triangles << node->triangle();
		}
		else
			triangles = selectionItem->triangles();

		foreach (Triangle* t, triangles)
		{
			if (t->type() == PostTriangle::RTTI)
			{
				if (!triangles.contains(selectedTriangle))
					t = selectedTriangle;
				else
					continue;
			}
			int idx = trianglesList.indexOf(t, 0);
			logFine(QString("FigureEditor::cutTriangleAction : removing triangle %1").arg(idx));
			flam3_delete_xform(genome_ptr, idx);
		}
	}
	else
	{
		Triangle* t = getCurrentOrSelected();
		int idx = trianglesList.indexOf(t, 0);
		logFine(QString("FigureEditor::cutTriangleAction : removing triangle %1").arg(idx));
		flam3_delete_xform(genome_ptr, idx);
	}
	reset();
	emit triangleModifiedSignal(selectedTriangle);
	emit undoStateSignal();
}

void FigureEditor::copyTriangleAction()
{
	if (has_selection && selectionItem->hasItems())
	{
		TriangleList triangles;
		if (selectionItem->selectedType() == NodeItem::RTTI)
		{
			foreach (NodeItem* node, selectionItem->nodes())
				triangles << node->triangle();
		}
		else
			triangles = selectionItem->triangles();

		xformClip.clear();
		foreach (Triangle* t, triangles)
		{
			if (t->type() == PostTriangle::RTTI)
			{
				if (!triangles.contains(selectedTriangle))
					xformClip << *(selectedTriangle->xform());
			}
			else
				xformClip << *(t->xform());
		}
		logFine(QString("FigureEditor::copyTriangleAction : "
			"adding %1 triangles to clip").arg(triangles.size()));
	}
	else
	{
		Triangle* t = getCurrentOrSelected();
		xformClip.clear();
		xformClip << *(t->xform());
		logFine(QString("FigureEditor::copyTriangleAction : "
			"adding 1 triangle to clip"));
	}
}

void FigureEditor::pasteTriangleAction()
{
	int size = xformClip.size();
	if (size > 0)
	{
		int n = genome_ptr->num_xforms;
		logFine(QString("FigureEditor::pasteTriangleAction : "
			"copying %1 triangles to slot %2").arg(size).arg(n));
		flam3_add_xforms(genome_ptr, size, 0, 0);
		memcpy(genome_ptr->xform + n, xformClip.data(), sizeof(flam3_xform) * size);
		reset();
		selectTriangle(trianglesList.last());
		emit triangleModifiedSignal(selectedTriangle);
		emit undoStateSignal();
	}
}

void FigureEditor::removeTriangleAction()
{
	Triangle* t = getCurrentOrSelected();
	int idx = trianglesList.indexOf(t, 0);
	logFine(QString("FigureEditor::removeTriangleAction : "
		"Removing triangle %1").arg(idx));
	flam3_delete_xform(genome_ptr, idx);
	reset();
	emit triangleModifiedSignal(selectedTriangle);
	emit undoStateSignal();
}

Triangle* FigureEditor::getCurrentOrSelected()
{
	Triangle* t;
	QGraphicsItem* item = itemAt(moving_start);
	if (menu_visible && item)
	{
		switch (item->type())
		{
			case Triangle::RTTI:
				t = dynamic_cast<Triangle*>(item);
				break;
			case NodeItem::RTTI:
				t = dynamic_cast<NodeItem*>(item)->triangle();
				if (t->type() != PostTriangle::RTTI)
					break;
			default:
				t = selectedTriangle;
		}
	}
	else
		t = selectedTriangle;
	return t;
}

void FigureEditor::resetTriangleCoordsAction()
{
	Triangle* t = selectedTriangle;
	QGraphicsItem* item = itemAt(moving_start);
	if (hasFocus() && item)
		switch (item->type())
		{
			case Triangle::RTTI:
			case PostTriangle::RTTI:
				t = dynamic_cast<Triangle*>(item);
				break;
			default:
				;
		}

	t->resetPosition();
	emit triangleModifiedSignal(selectedTriangle);
	emit undoStateSignal();
}

void FigureEditor::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
	moving_start = e->scenePos();

	if (e->button() == Qt::RightButton)
	{
		// menu_visible indicates that the menu is/was showing.  It's used in
		// getCurrentOrSelected to possibly select an item.  The menu_visible,
		// popupMenu, getCurrentOrSelected interaction works because the popup
		// event is synchronous, and the actions/slots are connected with type
		// Qt::DirectConnection.
		moving = 0;
		menu_visible = true;
		popupMenu->exec(e->screenPos());
		menu_visible = false;
		moving_start = QPointF(0.0, 0.0);
	}
	else if (e->button() == Qt::MidButton)
	{
		moving = 0;
	}
	else if (e->modifiers() & Qt::ControlModifier && !(e->modifiers() & Qt::ShiftModifier)) // LeftButton+Ctrl to select
	{
		is_selecting = true;
		has_selection = false;
		selectionItem->setPos(QPointF(0.0,0.0));
		selectionItem->setPolygon(QPolygonF(QRectF(moving_start,QSizeF(0.01,0.01))));
		selectionItem->setVisible(true);
		selectionItem->clear();
		update();
	}
	else if (e->modifiers() & Qt::ShiftModifier && !(e->modifiers() & Qt::ControlModifier))
	{
		// LeftButton+Shift to set mark
		if (coordinateMark->isVisible())
		{
			// left mouse button + shift sets the coordinate mark
			coordinateMark->centerOn(moving_start);
			if (!trianglesList.isEmpty())
				coordinateMark->setZValue(trianglesList.first()->nextZPos());
			update();
		}
	}
	else // LeftButton, no control modifier
	{
		QGraphicsItem* item = itemAt(moving_start);
		if (item == infoItem) // skip the infoItem if possible
		{
			QList<QGraphicsItem*> list = items(moving_start);
			if (list.size() > 1)
				item = list.at(1);
		}

		if (item)
		{
			moving = 0;
			logFine(QString("FigureEditor::mousePressEvent : rtti %1")
					.arg(item->type()));
			logFine(QString("FigureEditor::mousePressEvent : Found item %1")
					.arg((long)item));

			if ( item->type() == Triangle::RTTI )
			{
				Triangle* t = dynamic_cast<Triangle*>(item);
				moving = t;
				selectTriangle(t);
				t->findEdge(t->mapFromScene(e->scenePos()));
			}
			else if ( item->type() == PostTriangle::RTTI )
			{
				PostTriangle* t = dynamic_cast<PostTriangle*>(item);
				moving = t;
				t->findEdge(t->mapFromScene(e->scenePos()));
			}
			else if ( item->type() == NodeItem::RTTI )
			{
				NodeItem* node = dynamic_cast<NodeItem*>(item);
				Triangle* t = node->triangle();
				node->setZValue(t->nextZPos());
				moving = node;
				if (t->type() == Triangle::RTTI)
					selectTriangle(t);
			}

			// holding control+shift will activate and add an item to the selection
			if (e->modifiers() & Qt::ControlModifier && e->modifiers() & Qt::ShiftModifier)
			{
				// select the triangle for a node if selecting triangles
				if (selectionItem->selectedType() == Triangle::RTTI
					&& item->type() == NodeItem::RTTI)
					item = dynamic_cast<NodeItem*>(item)->triangle();

				if (!selectionItem->contains(item)
					&& (item->type() == selectionItem->selectedType()
						|| ( item->type() == PostTriangle::RTTI
							&& selectionItem->selectedType() == Triangle::RTTI) ) )
				{
					selectionItem->addItem(item);
					QPolygonF poly;
					foreach (QGraphicsItem* item, selectionItem->allItems())
						poly = poly.united(QPolygonF(item->sceneBoundingRect()));
					selectionItem->setPolygon(poly.boundingRect());
					selectionItem->setZValue(selectedTriangle->nextZPos());
					selectionItem->setVisible(true);
					has_selection = true;
				}
			}

			// if the selection is active, then only ever select it
			if (hasSelection())
			{
				if (item->type() != TriangleSelection::RTTI)
				{
					if (selectionItem->contains(item))
						item = selectionItem;
					else
						item = 0;
				}
				if (item)
				{
					moving = dynamic_cast<TriangleSelection*>(item);
					if (!trianglesList.isEmpty())
						moving->setZValue(trianglesList.first()->nextZPos());
				}
				else
					moving = 0;
			}

			wheel_moved = false;
		}
		else
			logFine("FigureEditor::mousePressEvent : no item selected");
	}
}

void FigureEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
	if (moving)
	{
		moving = 0;
		QPointF dz (e->buttonDownScenePos(Qt::LeftButton) - e->scenePos());
		if (editMode == Flip)
		{
			if (qAbs(dz.x()) > qAbs(dz.y()))
				flipTriangleHAction();
			else if (qAbs(dz.x()) < qAbs(dz.y()))
				flipTriangleVAction();
		}
		else
		{
			if (move_edge_mode)
				editMode = Move;
			else if (editMode == Move)
				setTransformLocation(transform_location);
			move_edge_mode = false;
			update();
		}
		if (dz != QPointF(0.0, 0.0) || wheel_moved)
			emit undoStateSignal();
	}
	else if (is_selecting)
	{
		is_selecting = false;
		if (e->scenePos() == e->buttonDownScenePos(Qt::LeftButton))
		{
			selectionItem->setPolygon(QPolygonF(QRectF(QPointF(0.0,0.0),QSizeF(0.1,0.1))));
			selectionItem->setPos(QPointF(0.0,0.0));
			selectionItem->setVisible(false);
			selectionItem->setZValue(-1);
			selectionItem->clear();
			has_selection = false;
		}
		else
		{
			selectionItem->setZValue(selectedTriangle->nextZPos());
			selectionItem->selectCoveredItems();
			has_selection = true;
		}
		update();
	}
	views().first()->setCursor(Qt::ArrowCursor);
	moving_start = QPointF(0.0,0.0);
}

void FigureEditor::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
	static Triangle* lastt;
	QPointF scenePos ( e->scenePos() );
	QPointF screenPos( e->screenPos() );
	// I guess we're only interested in scenePos() changes?  There
	// is an undocumented feature where this event handler is called
	// when the sceneRect is changed.
	if (screenPos == e->lastScreenPos())
		return;

	qreal dx( scenePos.x() - moving_start.x() );
	qreal dy( scenePos.y() - moving_start.y() );
	QGraphicsItem* item = itemAt(scenePos);

	if ( moving )
	{
		infoItem->hide();
		lastt = 0;

		if ( moving->type() == TriangleSelection::RTTI )
		{
			if ( editMode == Move )
			{
				selectionItem->moveBy(dx, dy);
				logFiner(QString("FigureEditor::mouseMoveEvent : triangle set moved to %1,%2")
						.arg(selectionItem->scenePos().x())
						.arg(selectionItem->scenePos().y()));
				if (selectionItem->hasItems())
					emit triangleModifiedSignal(selectionItem->first());
				views().first()->setCursor(Qt::SizeAllCursor);
			}
			else if ( editMode == Rotate )
			{
				if (qAbs(dx) >= qAbs(dy))
				{
					if (e->modifiers() & Qt::ShiftModifier)
						dx /= 10.0;
					else if (e->modifiers() & Qt::ControlModifier)
						dx *= 10.0;
					selectionItem->rotate(dx, selectionTransformPos());
					if (selectionItem->hasItems())
						emit triangleModifiedSignal(selectionItem->first());
				}
			}
			else if ( editMode == Scale )
			{
				if (qAbs(dx) >= qAbs(dy))
				{
					if (dx > 0)
					{
						if (e->modifiers() & Qt::ShiftModifier)
							dx = 1.01;
						else if (e->modifiers() & Qt::ControlModifier)
							dx = 1.10;
						else
							dx = 1.05;
					}
					else
					{
						if (e->modifiers() & Qt::ShiftModifier)
							dx = 0.99;
						else if (e->modifiers() & Qt::ControlModifier)
							dx = 0.9090;
						else
							dx = 0.9523;
					}
					selectionItem->scale(dx, dx, selectionTransformPos());
					if (selectionItem->hasItems())
						emit triangleModifiedSignal(selectionItem->first());
				}
			}
		}
		else //  moving->type() == Triangle::RTTI || moving->type() == PostTriangle::RTTI || moving->type() == NoteItem::RTTI
		{
			Triangle* t = 0;
			NodeItem* node = 0;
			if (moving->type() == NodeItem::RTTI)
			{
				node = dynamic_cast<NodeItem*>(moving);
				t = node->triangle();
			}
			else
				t = dynamic_cast<Triangle*>(moving);

			if ( editMode == Rotate
				|| (editMode == Move && t->getEdgeType() == Triangle::RotateEdge && !node) )
			{
				if (qAbs(dx) >= qAbs(dy))
				{
					if (e->modifiers() & Qt::ShiftModifier)
						dx /= 10.0;
					else if (e->modifiers() & Qt::ControlModifier)
						dx *= 10.0;

					if (editMode == Move)
					{
						// Activate rotate mode when selecting edges. This
						// makes the triangles repaint with rotation mode guides.
						editMode = Rotate;
						move_edge_mode = true;
					}
					rotateTriangle(t, dx, triangleTransformPos());
					emit triangleModifiedSignal(selectedTriangle);
				}

			}
			else if ( editMode == Scale
				|| (editMode == Move && t->getEdgeType() == Triangle::ScaleEdge && !node) )
			{
				if (qAbs(dx) >= qAbs(dy))
				{
					if (dx > 0)
					{
						if (e->modifiers() & Qt::ShiftModifier)
							dx = 1.01;
						else if (e->modifiers() & Qt::ControlModifier)
							dx = 1.10;
						else
							dx = 1.05;
					}
					else
					{
						if (e->modifiers() & Qt::ShiftModifier)
							dx = 0.99;
						else if (e->modifiers() & Qt::ControlModifier)
							dx = 0.9090;
						else
							dx = 0.9523;
					}

					if (editMode == Move)
					{
						// Activate scaling mode when selecting edges. This
						// makes the triangles repaint with scaling mode guides.
						editMode = Scale;
						move_edge_mode = true;
					}
					scaleTriangle(t, dx, dx, triangleTransformPos());
					emit triangleModifiedSignal(selectedTriangle);
				}
			}
			else if ( editMode == Flip )
			{
				QPointF dz (e->buttonDownScenePos(Qt::LeftButton) - e->scenePos());
				qreal dx = dz.x();
				qreal dy = dz.y();
				if (qAbs(dx) > qAbs(dy))
					views().first()->setCursor(Qt::SplitHCursor);
				else if (qAbs(dx) < qAbs(dy))
					views().first()->setCursor(Qt::SplitVCursor);
				else
					views().first()->setCursor(Qt::ArrowCursor);
			}
			else // Move mode
			{
				if (node)
					node->moveBy(dx, dy);
				else
					t->moveBy(dx, dy);
				emit triangleModifiedSignal(selectedTriangle);
				views().first()->setCursor(Qt::SizeAllCursor);
			}
		}

		moving_start = scenePos;
		update();
	}
	else if (is_selecting)
	{
		selectionItem->setPolygon(QPolygonF(QRectF(moving_start,scenePos)));
		update();
	}
	else if (e->buttons() & Qt::MidButton)
	{
		// middle mouse button + shift sets the coordinate mark
		if (coordinateMark->isVisible() && e->modifiers() & Qt::ShiftModifier)
		{
			coordinateMark->centerOn(scenePos);
			update();
		}

		// use control to scale the scene
		else if (e->modifiers() & Qt::ControlModifier)
		{
			dx = (screenPos.x() - e->lastScreenPos().x());
			dy = (screenPos.y() - e->lastScreenPos().y());
			if (qAbs(dx) >= qAbs(dy))
			{
				dx = (dx > 0) ? ( 1.05 + log(dx) / 10.0  ) : ( 0.9523 - log(qAbs(dx)) / 10.0 );
				scaleBasis(dx, dx);
			}
		}

		else
		{
			// otherwise scroll the qgraphicsview
			QScrollBar* hbar = views().first()->horizontalScrollBar();
			QScrollBar* vbar = views().first()->verticalScrollBar();
			dx = (screenPos.x() - e->lastScreenPos().x());
			dy = (screenPos.y() - e->lastScreenPos().y());
			hbar->setValue(hbar->value() - dx);
			vbar->setValue(vbar->value() - dy);
			if (infoItem->isVisible())
			{
				infoItem->hide();
				lastt = 0;
			}
			// return early and avoid emitting a coordinateChangeSignal
			return;
		}

	}
	else if (e->buttons() & Qt::LeftButton)
	{
		// left mouse button + shift sets the coordinate mark
		if (coordinateMark->isVisible() && e->modifiers() & Qt::ShiftModifier)
		{
			coordinateMark->centerOn(scenePos);
			update();
		}

		else
		{
			// otherwise scroll the qgraphicsview
			QScrollBar* hbar = views().first()->horizontalScrollBar();
			QScrollBar* vbar = views().first()->verticalScrollBar();
			dx = (screenPos.x() - e->lastScreenPos().x());
			dy = (screenPos.y() - e->lastScreenPos().y());
			hbar->setValue(hbar->value() - dx);
			vbar->setValue(vbar->value() - dy);
			if (infoItem->isVisible())
			{
				infoItem->hide();
				lastt = 0;
			}
			// return early and avoid emitting a coordinateChangeSignal
			return;
		}
	}
	else if (item)
	{
		switch (item->type())
		{
			case NodeItem::RTTI:
				item = dynamic_cast<NodeItem*>(item)->triangle();

			case Triangle::RTTI:
				if (item->type() != PostTriangle::RTTI)
				{
					// show the xform's infoItem for this triangle
					Triangle* t = dynamic_cast<Triangle*>(item);
					if (t == selectedTriangle && editMode == Move)
						t->findEdge(t->mapFromScene(e->scenePos()));
					if (lastt != t)
					{
						infoItem->setText(getInfoLabel(t));
						QPoint pt = QPoint(5,
								views().first()->maximumViewportSize().height()
								+ 10 - (int)infoItem->boundingRect().height());
								// 10 units of fudge
						infoItem->setPos(views().first()->mapToScene(pt));
						infoItem->setBrush(QBrush(t->pen().color()));
						infoItem->setZValue(selectedTriangle->zValue());
						infoItem->show();
						lastt = t;
					}
					break;
				}
			case PostTriangle::RTTI:
			{
				PostTriangle* post = dynamic_cast<PostTriangle*>(item);
				if (editMode == Move)
					post->findEdge(post->mapFromScene(e->scenePos()));
				Triangle* t = selectedTriangle;
				int tidx = trianglesList.indexOf(t);
				infoItem->setText(tr("post transform: %1\n").arg(tidx + 1));
				QPoint pt = QPoint(5,
						views().first()->maximumViewportSize().height()
						+ 10 - (int)infoItem->boundingRect().height());
						// 10 units of fudge
				infoItem->setPos(views().first()->mapToScene(pt));
				infoItem->setBrush(QBrush(t->pen().color()));
				infoItem->setZValue(selectedTriangle->zValue());
				infoItem->show();
				break;
			}
			default:
				;
		}
	}
	else if (item != infoItem && infoItem->isVisible())
	{
		infoItem->hide();
		lastt = 0;
		update();
	}
	emit coordinateChangeSignal( scenePos.x(), scenePos.y() );
}


void FigureEditor::wheelEvent(QGraphicsSceneWheelEvent* e)
{
	QPointF p = e->scenePos();
	QGraphicsItem* item = itemAt(p);
	if (item)
	{
		switch (item->type())
		{
			case Triangle::RTTI:
			case PostTriangle::RTTI:
			{
				double rad;
				Triangle* t;
				if (item->type() == Triangle::RTTI)
					t = dynamic_cast<Triangle*>(item);
				else
					t = dynamic_cast<PostTriangle*>(item);

				QPointF cpos = t->mapFromScene(p);
				if (e->modifiers() & Qt::ControlModifier)
				{
					// scale
					if (e->delta() > 0)
						if (e->modifiers() & Qt::ShiftModifier)
							rad = 1.05;
						else
							rad = 1.1;
					else
						if (e->modifiers() & Qt::ShiftModifier)
							rad = 0.9523;
						else
							rad = 0.9090;
					scaleTriangle(t, rad, rad, cpos);
				}
				else
				{
					// rotate
					if (e->modifiers() & Qt::ShiftModifier)
						rad = (double)e->delta() / (128.*3.141592654);
					else
						rad = (double)e->delta() / (16.*3.141592654);
					rotateTriangle(t, rad, cpos);
				}
				e->accept();
				break;
			}
			case BasisTriangle::RTTI:
			{
				double rad;
				if (e->delta() > 0)
					if (e->modifiers() & Qt::ShiftModifier)
						rad = 1.1;
					else
						rad = 1.2;
				else
					if (e->modifiers() & Qt::ShiftModifier)
						rad = 0.9090;
					else
						rad = 0.8333;
				scaleBasis(rad,rad);
				e->accept();
				break;
			}
			case TriangleSelection::RTTI:
			{
				double rad;
				QPointF cpos = selectionItem->mapFromScene(p);
				if (e->modifiers() & Qt::ControlModifier)
				{
					// scale
					if (e->delta() > 0)
						if (e->modifiers() & Qt::ShiftModifier)
							rad = 1.05;
						else
							rad = 1.1;
					else
						if (e->modifiers() & Qt::ShiftModifier)
							rad = 0.9523;
						else
							rad = 0.9090;
					scaleSelection(rad, rad, cpos);
				}
				else
				{
					// rotate
					if (e->modifiers() & Qt::ShiftModifier)
						rad = (double)e->delta() / (128.*3.141592654);
					else
						rad = (double)e->delta() / (16.*3.141592654);
					rotateSelection(rad, cpos);
				}
				e->accept();
			}
			default:
			;
		}
		wheel_moved = true;
	}
	else
	{
		// scale the scene
		double rad;
		if (e->delta() > 0)
		{
			if (e->modifiers() & Qt::ShiftModifier)
				rad = 1.1;
			else if (e->modifiers() & Qt::ControlModifier)
				rad = 1.5;
			else
				rad = 1.2;
		}
		else
		{
			if (e->modifiers() & Qt::ShiftModifier)
				rad = 0.9090;
			else if (e->modifiers() & Qt::ControlModifier)
				rad = 0.6767;
			else
				rad = 0.8333;
		}
		scaleBasis(rad,rad);
		e->accept();
	}
}

void FigureEditor::scaleInScene()
{
	scaleBasis(1.1, 1.1);
}

void FigureEditor::scaleOutScene()
{
	scaleBasis(.9090, .9090);
}

void FigureEditor::triangleModifiedAction(Triangle* t)
{
	logFinest(QString("FigureEditor::triangleModifiedAction : t=0x%1")
			.arg((long)t, 0, 16));
	if (t->type() == PostTriangle::RTTI)
		t = selectedTriangle;
	selectTriangle(t);
	update();
	emit triangleModifiedSignal(t);
}

void FigureEditor::flipTriangleHAction()
{
	if (hasSelection())
		flipSelectionHAction(selectionTransformPos());
	else if (postEnabled())
		flipTriangleHAction(post(), triangleTransformPos());
	else
		flipTriangleHAction(selectedTriangle, triangleTransformPos());
}

void FigureEditor::flipTriangleVAction()
{
	if (hasSelection())
		flipSelectionVAction(selectionTransformPos());
	else if (postEnabled())
		flipTriangleVAction(post(), triangleTransformPos());
	else
		flipTriangleVAction(selectedTriangle, triangleTransformPos());
}

void FigureEditor::flipTriangleHPopupAction()
{
	if (has_selection)
		flipSelectionHAction(selectionItem->mapFromScene(moving_start));
	else if (editing_post)
		flipTriangleHAction(dynamic_cast<Triangle*>(postTriangle),
			postTriangle->mapFromScene(moving_start));
	else
		flipTriangleHAction(selectedTriangle, selectedTriangle->mapFromScene(moving_start));
}

void FigureEditor::flipTriangleHAction(Triangle* t, QPointF cpos)
{
	t->flipHorizontally(cpos);
	triangleModifiedAction(t);
}

void FigureEditor::flipTriangleVPopupAction()
{
	if (has_selection)
		flipSelectionVAction(selectionItem->mapFromScene(moving_start));
	else if (editing_post)
		flipTriangleVAction(dynamic_cast<Triangle*>(postTriangle),
			postTriangle->mapFromScene(moving_start));
	else
		flipTriangleVAction(selectedTriangle, selectedTriangle->mapFromScene(moving_start));
}

void FigureEditor::flipTriangleVAction(Triangle* t, QPointF cpos)
{
	t->flipVertically(cpos);
	triangleModifiedAction(t);
}

void FigureEditor::flipSelectionVAction(QPointF cpos)
{
	selectionItem->flipVertically(cpos);
	if (selectionItem->hasItems())
		triangleModifiedAction(selectedTriangle);
}

void FigureEditor::flipSelectionHAction(QPointF cpos)
{
	selectionItem->flipHorizontally(cpos);
	if (selectionItem->hasItems())
		triangleModifiedAction(selectedTriangle);
}

BasisTriangle* FigureEditor::basis() const
{
	return basisTriangle;
}

void FigureEditor::setCoordTransform(QMatrix m)
{
	basisTriangle->setCoordTransform(m);
}

const QMatrix& FigureEditor::transform() const
{
	return basisTriangle->coordTransform();
}

void FigureEditor::selectTriangle(Triangle* t)
{
	if (selectedTriangle == t)
		return;

	bool post = editing_post;

	if (selectedTriangle)
	{
		const QColor c(Util::get_xform_color(genome_ptr, selectedTriangle->xform()));
		selectedTriangle->setPen(QPen(c, 0, Qt::DashLine));
		selectedTriangle->setBrush(Qt::NoBrush);
		editPostTriangle(false);
	}

	selectedTriangle = t;
	logFine(QString("FigureEditor::selectTriangle : selecting triangle %1")
		.arg(trianglesList.indexOf(t, 0) + 1));
	const QColor c(Util::get_xform_color(genome_ptr, selectedTriangle->xform()));
	QBrush brush(c, Qt::Dense5Pattern);
	brush.setMatrix(basisTriangle->coordTransform());
	selectedTriangle->setPen(QPen(c, 0, Qt::SolidLine));
	selectedTriangle->setBrush(brush);
	selectedTriangle->moveToFront();
	triangleMenu->setDefaultAction(triangleMenu->actions().value(selectedTriangleIndex()));
	box_center = selectedTriangle->polygon().boundingRect().center();
	editPostTriangle(post);
	update();
	emit triangleSelectedSignal(t);
}

void FigureEditor::reset()
{
	logFine("FigureEditor::reset : entered");
	if (genome_ptr->num_xforms < 1)
	{
		logWarn("FigureEditor::reset : no xforms, adding a default");
		Util::add_default_xform(genome_ptr);
	}

	int selected_idx = 0;
	if (selectedTriangle)
		selected_idx = selectedTriangleIndex();

	bool hasPost = editing_post;
	editPostTriangle(false);

	if (has_selection)
		enableSelection(false);

	// make sure there are as many triangles as xforms
	int num_triangles = trianglesList.size();
	int dn = genome_ptr->num_xforms - num_triangles;
	if (dn > 0)
		for (int n = num_triangles ; n < dn + num_triangles ; n++)
		{
			Triangle* t = new Triangle(this, genome_ptr->xform + n, basisTriangle, n);
			trianglesList << t;
			addItem(t);
			QString action_name = QString("%1").arg(n+1);
			QAction* a = triangleMenu->addAction(action_name);
			a->setObjectName(action_name);
			a->setCheckable(false);
		}
	else if (dn < 0)
	{
		for (int n = 0 ; n > dn ; n--)
		{
			Triangle* t = trianglesList.takeLast();
			removeItem(t);
			delete t;
			triangleMenu->removeAction(triangleMenu->actions().last());
		}
	}

	// realign triangles and xforms
	for (int n = 0 ; n < genome_ptr->num_xforms ; n++)
	{
		Triangle* t = trianglesList.at(n);
		flam3_xform* xform = genome_ptr->xform + n;
		t->setXform(xform);
		TriangleCoords tc(basisTriangle->getCoords(xform->c));
		t->setPoints(tc);
		const QPen pen(Util::get_xform_color(genome_ptr, xform), 0, Qt::DashLine);
		t->setPen(pen);
		t->setNodeColor(grid_color, pen.color());
	}

	// align the basis triangle and labels
	const QBrush b(grid_color);
	const QPen pen(grid_color);
	bLabelA->setBrush(b);
	bLabelA->setPos(basisTriangle->mapToScene(basisTriangle->A) + QPointF(-8.0,0.0));
	bLabelB->setBrush(b);
	bLabelB->setPos(basisTriangle->mapToScene(basisTriangle->B));
	bLabelC->setBrush(b);
	bLabelC->setPos(basisTriangle->mapToScene(basisTriangle->C) + QPointF(-8.0,-16.0));
	basisTriangle->setPen(pen);
	coordinateMark->setPen(pen);
	selectionItem->setPen(QPen(b,1,Qt::DashLine));

	selectedTriangle = 0;
	editing_post = hasPost; // there's a side-effect here that selectTriangle
							// will recreate the postTriangle with this set
	if (selected_idx < trianglesList.size())
		selectTriangle(trianglesList.at(selected_idx));
	else
		selectTriangle(trianglesList.last());

	adjustSceneRect();
	logFiner(QString("FigureEditor::reset : sceneRect %1,%2")
			.arg(sceneRect().width())
			.arg(sceneRect().height()));
	logFiner(QString("FigureEditor::reset : itemsRect %1,%2")
			.arg(itemsBoundingRect().size().width())
			.arg(itemsBoundingRect().size().height()));
	logFiner(QString("FigureEditor::reset : viewport %1,%2")
			.arg(views().first()->maximumViewportSize().width())
			.arg(views().first()->maximumViewportSize().height()));
}

void FigureEditor::adjustSceneRect()
{
	// Adjust the sceneRect so it covers just more than the items.  This
	// helps reposition the sceneview and adjusts the scrollbars.  It also
	// helps avoid some "jumpiness" that sometimes happens when moving
	// a graphicsitem outside of the sceneRect.
	setSceneRect(itemsSceneBounds().adjusted(-200.,-200.,200.,200.));

	// recenter the view if necessary
	if (centered_scaling != None)
	{
		Triangle* t = selectedTriangle;
		QPointF pos(0., 0.);
		if (postEnabled())
			t = post();

		switch (centered_scaling)
		{
			case Circum:
				// get the triangle's center position from the scene
				pos = t->circumCenter();
				break;
			case Box:
				// get the triangle's center position from the scene
				pos = t->polygon().boundingRect().center();
				break;
			case Mark:
				// center on the marker
				pos = t->mapFromScene(mark()->center());
				break;
			case NodeO:
				pos = t->getCoords().at(0);
				break;
			case NodeX:
				pos = t->getCoords().at(1);
				break;
			case NodeY:
				pos = t->getCoords().at(2);
				break;
			default:
				break;
		}
		views().first()->centerOn(basisTriangle->mapToScene(pos));
	}
	else if (!moving_start.isNull())
		views().first()->centerOn(moving_start);

	if (infoItem->isVisible())
	{
		// put the infoItem back
		QPoint pt = QPoint(5,
			views().first()->maximumViewportSize().height()
			+ 10 - (int)infoItem->boundingRect().height());
			// 10 units of fudge
			infoItem->setPos(views().first()->mapToScene(pt));
	}
}

void FigureEditor::drawBackground(QPainter* p, const QRectF& r)
{
	QRectF rect(sceneRect().united(r));
	p->fillRect(r, QBrush(bg_color));
	if (grid_visible)
	{
		// draw a grid on the background
		QPointF A = basisTriangle->mapToScene(basisTriangle->A);
		QPointF B = basisTriangle->mapToScene(basisTriangle->B);
		QPointF C = basisTriangle->mapToScene(basisTriangle->C);
		QPointF dX = B - A;
		while (dX.x() > 400.0) // maximum grid spacing
			dX /= 2.0;
		while (dX.x() < 20.0) // minimum grid spacing
			dX *= 10.0;
		QPen pen(QBrush(grid_color, Qt::Dense6Pattern),Qt::DashLine);
		p->setPen(pen);
		for (qreal n = A.x() ; n <= rect.width() ; n+=dX.x())
			p->drawLine(QPointF(n, rect.y()), QPointF(n, rect.height()));
		for (qreal n = A.x() - dX.x() ; n >= rect.x() ; n-=dX.x())
			p->drawLine(QPointF(n, rect.y()), QPointF(n, rect.height()));
		for (qreal n = C.y() ; n <= rect.height() ; n += dX.x())
			p->drawLine(QPointF(rect.x(), n), QPointF(rect.width(), n));
		for (qreal n = C.y() - dX.x() ; n >= rect.y() ; n -= dX.x())
			p->drawLine(QPointF(rect.x(), n), QPointF(rect.width(), n));
	}

	if (preview_visible)
	{
		int xi = selectedTriangle->index();
		double range = 1.0;
		int numvals = preview_density;
		int depth = 1;
		int rcnt = 2* (2*numvals+1)*(2*numvals+1);
		double* result = new double[rcnt];
		randctx *rc = Util::get_isaac_randctx();
		flam3_genome g;
		Util::init_genome(&g);
		flam3_copy(&g, genomes->selectedGenome());
		flam3_xform_preview(&g, xi, range, numvals, depth, result, rc);
		p->setPen(selectedTriangle->pen());
		for (int i = 0 ; i < rcnt  ; i+=2)
			p->drawPoint(selectedTriangle->mapToScene(result[i], -result[i+1]));
		clear_cp(&g, flam3_defaults_on);
		delete[] result;
	}

	if (guide_visible)
	{
		QPointF  cen;
		QPolygonF poly;
		QPen pen;
		if (has_selection)
		{
			cen  = selectionItem->mapToScene(selectionTransformPos());
			poly = selectionItem->mapToScene(selectionItem->polygon());
			pen  = selectionItem->pen();
		}
		else
		{
			Triangle* t = selectedTriangle;
			if (postEnabled())
				t = postTriangle;
			cen  = t->mapToScene(triangleTransformPos());
			poly = t->mapToScene(t->polygon());
			pen  = t->pen();
		}
		pen.setColor(guide_color);

		if (editMode == FigureEditor::Rotate)
		{
			qreal rmax = 0.0;
			foreach (QPointF p, poly)
			{
				QLineF l(p, cen);
				qreal len(l.length());
				if (len > rmax)
					rmax = len;
			}
			pen.setStyle(Qt::SolidLine);
			p->setPen(pen);
			p->drawEllipse(cen, rmax, rmax);
			p->setBrush(QBrush(pen.color(), Qt::SolidPattern));
			QRectF r(-1., -1., 2., 2.);
			r.moveCenter(cen);
			p->drawEllipse(r);
		}
		else if (editMode == FigureEditor::Flip)
		{
			QPolygonF pa = poly;
			double tx = cen.x();
			int n = 0;
			pen.setStyle(Qt::SolidLine);
			p->setPen(pen);
			p->setBrush(Qt::NoBrush);
			foreach (QPointF p, pa)
			{
				p.rx()  = p.x() - 2.0 * (p.x() - tx);
				pa[n++] = p;
			}
			p->drawPolygon(pa);
			pa = poly;
			tx = cen.y();
			n = 0;
			foreach (QPointF p, pa)
			{
				p.ry()  = p.y() - 2.0 * (p.y() - tx);
				pa[n++] = p;
			}
			p->drawPolygon(pa);
		}
		else if (editMode == FigureEditor::Scale)
		{
			QRectF r( poly.boundingRect() );
			qreal xmax = qMax(qAbs(r.left() - cen.x()), qAbs(r.right()  - cen.x()));
			qreal ymax = qMax(qAbs(r.top()  - cen.y()), qAbs(r.bottom() - cen.y()));
			QPointF pmax(xmax, ymax);
			QRectF f(pmax, -pmax);
			f.moveCenter(cen);
			pen.setStyle(Qt::SolidLine);
			p->setPen(pen);
			p->drawRect(f);
			pen.setStyle(Qt::DotLine);
			p->setPen(pen);
			p->drawLine(f.topLeft(),f.bottomRight());
			p->drawLine(f.topRight(),f.bottomLeft());
		}


	}
}

void FigureEditor::colorChangedAction(double idx)
{
	logFinest(QString("FigureEditor::colorChangedAction : idx=0x%1").arg(idx));
	foreach (Triangle* triangle, trianglesList)
	{
		const QColor c( Util::get_xform_color(genome_ptr, triangle->xform()) );
		QPen pen(triangle->pen());
		QBrush brush(triangle->brush());
		pen.setColor(c);
		brush.setColor(c);
		triangle->setPen(pen);
		triangle->setBrush(brush);
		foreach (NodeItem* n, triangle->getNodes())
			n->setPen(QPen(grid_color), c);
	}
	update();
}

int FigureEditor::selectedTriangleIndex()
{
	return trianglesList.indexOf(selectedTriangle);
}

Triangle* FigureEditor::getSelectedTriangle()
{
	return selectedTriangle;
}

int FigureEditor::getNumberOfTriangles()
{
	return trianglesList.size();
}

bool FigureEditor::gridVisible() const
{
	return grid_visible;
}

void FigureEditor::setGridVisible(bool value)
{
	grid_visible = value;
	update();
}

bool FigureEditor::guideVisible() const
{
	return guide_visible;
}

void FigureEditor::setGuideVisible(bool value)
{
	guide_visible = value;
	update();
}

int FigureEditor::previewDensity() const
{
	return preview_density;
}

void FigureEditor::setPreviewDensity(int value)
{
	preview_density = value;
	update();
}

bool FigureEditor::previewVisible() const
{
	return preview_visible;
}

void FigureEditor::setPreviewVisible(bool value)
{
	preview_visible = value;
	update();
}


QColor FigureEditor::gridColor() const
{
	return grid_color;
}

void FigureEditor::setGridColor(const QColor& c)
{
	grid_color = c;
	const QPen p(grid_color);
	const QBrush b(grid_color);
	basisTriangle->setPen(p);
	coordinateMark->setPen(p);
	bLabelA->setBrush(b);
	bLabelB->setBrush(b);
	bLabelC->setBrush(b);
	selectionItem->setPen(QPen(b,1,Qt::DashLine));
	foreach(Triangle* t, trianglesList)
		t->setNodeColor(c, t->pen().color());
	update();
}

QColor FigureEditor::guideColor() const
{
	return guide_color;
}

void FigureEditor::setGuideColor(QColor c)
{
	guide_color = c;
	update();
}

QColor FigureEditor::bgColor() const
{
	return bg_color;
}

void FigureEditor::setbgColor(QColor c)
{
	bg_color = c;
	update();
}

void FigureEditor::triangleMenuAction(QAction* a)
{
	int idx = a->objectName().toInt() - 1;
	selectTriangle(idx);
}

void FigureEditor::selectTriangle(int idx)
{
	if (idx < trianglesList.size())
	{
		Triangle* t = trianglesList.at(idx);
		selectTriangle(t);
		QList<QGraphicsView*> viewList = views();
		viewList.first()->ensureVisible(t->sceneBoundingRect());
	}
}

CoordinateMark* FigureEditor::mark() const
{
	return coordinateMark;
}

void FigureEditor::setMarkVisible(bool flag)
{
	coordinateMark->setVisible(flag);
	if (flag && !trianglesList.isEmpty())
		coordinateMark->setZValue(trianglesList.first()->nextZPos());
	emit coordinateChangeSignal(0.,0.);
}

// this function avoids wierd caching issues while calculating
// the itemsBoundingRect.
QRectF FigureEditor::itemsSceneBounds()
{
	QRectF bounds(basisTriangle->sceneBoundingRect());
	foreach (Triangle* t, trianglesList)
		bounds = bounds.united(t->sceneBoundingRect());
	if (editing_post)
		bounds = bounds.united(postTriangle->sceneBoundingRect());
	if (selectionItem->isVisible())
		bounds = bounds.united(selectionItem->sceneBoundingRect());
	if (coordinateMark->isVisible())
		bounds = bounds.united(coordinateMark->sceneBoundingRect());
	return bounds;
}

void FigureEditor::autoScale()
{
	QSizeF padding(100.0, 100.0);
	QSizeF s_size(itemsSceneBounds().size() + padding);
	QSize v_size(views().first()->maximumViewportSize());
	double dz = qMin(v_size.width() / s_size.width(),
		v_size.height() / s_size.height());
	while (dz < 0.99 || dz > 1.01)
	{
		logFine(QString("FigureEditor::autoScale : scale %1").arg(dz));
		scaleBasis(dz, dz);
		s_size = itemsSceneBounds().size() + padding;
		dz = qMin(v_size.width() / s_size.width(),
				v_size.height() / s_size.height());
	}
	views().first()->ensureVisible(itemsSceneBounds(), 0, 0);
	adjustSceneRect();
}

void FigureEditor::selectNextTriangle()
{
	int n = selectedTriangleIndex();
	if (n + 1 < trianglesList.size())
		selectTriangle(n + 1);
}

void FigureEditor::selectPreviousTriangle()
{
	int n = selectedTriangleIndex();
	if (n > 0)
		selectTriangle(n - 1);
}

void FigureEditor::scaleBasis(double dx, double dy)
{
	// we need to move the mark too, so save it's current position in the basis
	// triangle coordinate space.  It may be better to make the mark aware of
	// the basis instead.
	QPointF mark = coordinateMark->center();
	QPointF mark_start;
	transform().inverted().map(mark.x(), mark.y(), &mark_start.rx(), &mark_start.ry());

	QPointF cursor_start;
	transform().inverted().map(moving_start.x(), moving_start.y(), &cursor_start.rx(), &cursor_start.ry());

	basisTriangle->scale(dx, dy);
	bLabelA->setPos(basisTriangle->mapToScene(basisTriangle->A) + QPointF(-8.0,0.0));
	bLabelB->setPos(basisTriangle->mapToScene(basisTriangle->B));
	bLabelC->setPos(basisTriangle->mapToScene(basisTriangle->C) + QPointF(-8.0,-16.0));

	foreach (Triangle* t, trianglesList)
		t->basisScaledSlot();

	if (editing_post)
		postTriangle->basisScaledSlot();

	// scale the selection
	if (has_selection)
	{
		QTransform trans = selectionItem->transform();
		QPointF cpos(selectionItem->pos());
		selectionItem->translate(cpos.x(), cpos.y());
		selectionItem->QGraphicsPolygonItem::scale(dx, dy);
		selectionItem->translate(-cpos.x(), -cpos.y());
		QPolygonF pa = selectionItem->mapToScene(selectionItem->polygon());
		selectionItem->setTransform(trans);
		selectionItem->setPolygon(pa);
	}

	// put back the mark
	transform().map(mark_start.x(), mark_start.y(), &mark.rx(), &mark.ry());
	coordinateMark->centerOn(mark);

	// put back the moving_start position
	transform().map(cursor_start.x(), cursor_start.y(), &moving_start.rx(), &moving_start.ry());

	// adjust scene rect and repaint
	adjustSceneRect();
	update();
}

void FigureEditor::rotateTriangle(Triangle* t, double rad, QPointF pos)
{
	t->rotate(rad, pos);
	if (editing_post)
		t = selectedTriangle;
	triangleModifiedAction(t);
}

void FigureEditor::scaleTriangle(Triangle* t, double dx, double dy, QPointF pos)
{
	t->scale(dx, dy, pos);
	if (editing_post)
		t = selectedTriangle;
	triangleModifiedAction(t);
}

void FigureEditor::rotateSelection( double rad, QPointF pos )
{
	selectionItem->rotate(rad, pos);
	logFine(QString("triangle set rotated rad %1 (%2,%3)")
				.arg(rad)
				.arg(pos.x())
				.arg(pos.y()));
	if (selectionItem->hasItems())
		triangleModifiedAction(selectedTriangle);
}

void FigureEditor::scaleSelection( double dx, double dy, QPointF pos )
{
	selectionItem->scale(dx, dy, pos);
	logFine(QString("triangle set scaled dx %1 (%2,%3)")
				.arg(dx)
				.arg(pos.x())
				.arg(pos.y()));
	if (selectionItem->hasItems())
		triangleModifiedAction(selectedTriangle);
}

bool FigureEditor::hasSelection() const
{
	return has_selection;
}


TriangleSelection* FigureEditor::selection() const
{
	return selectionItem;
}


void FigureEditor::moveTriangleBy(Triangle* t, double dx, double dy)
{
	QPointF p(t->mapToScene(dx, dy));
	t->moveBy(p.x(), p.y());
	if (t->type() == PostTriangle::RTTI)
		triangleModifiedAction(selectedTriangle);
	else
		triangleModifiedAction(t);
}


void FigureEditor::moveSelectionBy(double dx, double dy)
{
	QPointF p = selectionItem->mapFromItem(selectedTriangle, QPointF(dx,dy));
	selectionItem->moveBy(p.x(), p.y());
	logFine(QString("triangle set moved %1,%2")
			.arg(selectionItem->scenePos().x())
			.arg(selectionItem->scenePos().y()));
	if (selectionItem->hasItems())
		triangleModifiedAction(selectionItem->first());
}


void FigureEditor::editPostTriangle(bool flag)
{
	if (postTriangle)
	{
		flam3_xform* xf = selectedTriangle->xform();
		if (has_selection && (xf != postTriangle->xform()))
		{
			selectionItem->triangles().removeOne(postTriangle);
			foreach (NodeItem* node, postTriangle->getNodes())
				selectionItem->nodes().removeOne(node);
		}
		postTriangle->setXform(xf);
		postTriangle->basisScaledSlot();
	}
	else if (selectedTriangle)
	{
		postTriangle = new PostTriangle(this, selectedTriangle->xform(), basisTriangle);
		const QPen pen( grid_color );
		QBrush brush(grid_color, Qt::Dense5Pattern);
		brush.setMatrix(basisTriangle->coordTransform());
		postTriangle->setPen( pen );
		postTriangle->setNodeColor(grid_color, pen.color());
		postTriangle->setBrush(brush);
		addItem(postTriangle);
	}
	else
		return;

	if (flag) // enable post triangle
	{
		postTriangle->setVisible(true);
		postTriangle->moveToFront();
		postTriangle->adjustSceneRect();
		box_center = postTriangle->polygon().boundingRect().center();
	}
	else
	{
		box_center = selectedTriangle->polygon().boundingRect().center();
		postTriangle->setVisible(false);
	}

	editing_post = flag;
	update();
}

bool FigureEditor::postEnabled()
{
	return editing_post;
}

PostTriangle* FigureEditor::post() const
{
	return postTriangle;
}

QString FigureEditor::getInfoLabel(Triangle* t)
{
	int tidx = trianglesList.indexOf(t);
	QString label(tr("transform: %1\ndensity: %L2\n"));
	if (genome_ptr->final_xform_enable
			&& genome_ptr->final_xform_index == tidx)
		label = tr("final transform: %1\ndensity: %L2\n");

	QString info(label.arg(tidx + 1).arg(t->xform()->density, 0, 'f', 4, '0'));
	QMap<QString, int> vars = Util::flam3_variations();
	QMap<QString, int>::const_iterator i = vars.begin();
	while (i != vars.end())
	{
		double val = t->xform()->var[i.value()];
		if (val != 0.0)
			info.append(QString("%1: %L2\n").arg(i.key()).arg(val, 0, 'f', 4, '0'));
		i++;
	}
	return info;
}

const TriangleList& FigureEditor::triangles() const
{
	return trianglesList;
}

void FigureEditor::enableSelection(bool flag)
{
	selectionItem->setVisible(flag);
	has_selection = flag;
	if (!flag)
		selectionItem->clear();
	update();
}

FigureEditor::SceneLocation FigureEditor::centeredScaling()
{
	return centered_scaling;
}

void FigureEditor::setCenteredScaling(SceneLocation location)
{
	centered_scaling = location;
}

FigureEditor::SceneLocation FigureEditor::transformLocation()
{
	return transform_location;
}

void FigureEditor::setTransformLocation(SceneLocation location)
{
	transform_location = location;
	if (postEnabled())
		box_center = postTriangle->polygon().boundingRect().center();
	else
		box_center = selectedTriangle->polygon().boundingRect().center();
}

void FigureEditor::saveUndoState(UndoState* state)
{
	QTransform itrans(basisTriangle->coordTransform().inverted());
	if (selectionItem->isVisible())
		state->SelectionRect = itrans.map(selectionItem->polygon());
	else
		state->SelectionRect = QPolygonF();

	state->NodesO.clear();
	state->NodesX.clear();
	state->NodesY.clear();
	state->Triangles.clear();
	state->SelectedType = selectionItem->selectedType();

	switch (selectionItem->selectedType())
	{
		case NodeItem::RTTI:
			foreach (NodeItem* node, selectionItem->nodes())
			{
				if (node->id() == Triangle::NODE_O)
					if (node->triangle()->type() == PostTriangle::RTTI)
						state->NodesO.append(-1);
					else
						state->NodesO.append(node->triangle()->index());

				else if (node->id() == Triangle::NODE_X)
					if (node->triangle()->type() == PostTriangle::RTTI)
						state->NodesX.append(-1);
					else
						state->NodesX.append(node->triangle()->index());

				else if (node->id() == Triangle::NODE_Y)
					if (node->triangle()->type() == PostTriangle::RTTI)
						state->NodesY.append(-1);
					else
						state->NodesY.append(node->triangle()->index());

				else
					logWarn(QString("FigureEditor::saveUndoState : found node with unknown name %1").arg(node->name()));
			}
			break;

		case Triangle::RTTI:
			foreach (Triangle* triangle, selectionItem->triangles())
				if (triangle->type() == PostTriangle::RTTI)
					state->Triangles.append(-1);
				else
					state->Triangles.append(triangle->index());
			break;

		default:
			;
	}
	state->MarkPos = itrans.map(coordinateMark->center());
}

void FigureEditor::restoreUndoState(UndoState* state)
{
	// Rebuild and reset the selection and the mark
	QTransform basis(basisTriangle->coordTransform());

	if (state->SelectionRect.isEmpty())
		enableSelection(false);
	else
	{
		selectionItem->clear();
		selectionItem->setPolygon(basis.map(state->SelectionRect));
		selectionItem->setSelectedType(state->SelectedType);
		switch (state->SelectedType)
		{
			case NodeItem::RTTI:
				foreach (int n, state->NodesO)
					if (n != -1)
						selectionItem->addItem(trianglesList[n]->getNode(Triangle::NODE_O));
					else
						if (postEnabled())
							selectionItem->addItem(post()->getNode(Triangle::NODE_O));

				foreach (int n, state->NodesX)
					if (n != -1)
						selectionItem->addItem(trianglesList[n]->getNode(Triangle::NODE_X));
					else
						if (postEnabled())
							selectionItem->addItem(post()->getNode(Triangle::NODE_X));

				foreach (int n, state->NodesY)
					if (n != -1)
						selectionItem->addItem(trianglesList[n]->getNode(Triangle::NODE_Y));
					else
						if (postEnabled())
							selectionItem->addItem(post()->getNode(Triangle::NODE_Y));
				break;

			case Triangle::RTTI:
				foreach (int n, state->Triangles)
					if (n != -1)
						selectionItem->addItem(trianglesList[n]);
					else
						if (postEnabled())
							selectionItem->addItem(post());
				break;

			default:
				;
		}
		enableSelection(true);
	}
	coordinateMark->centerOn(basis.map(state->MarkPos));
}

FigureEditor::EditMode FigureEditor::mode() const
{
	return editMode;
}

void FigureEditor::setMode(FigureEditor::EditMode m)
{
	editMode = m;
	setTransformLocation(transform_location);
	update();
	emit editModeChangedSignal(editMode);
}

QPointF FigureEditor::triangleTransformPos()
{
	Triangle* t = selectedTriangle;
	if (postEnabled())
		t = post();
	return triangleTransformPos(t);
}

QPointF FigureEditor::triangleTransformPos(Triangle* t)
{
	QPointF pos(0.0, 0.0);

	if (transform_location == Origin)
		pos = t->mapFromScene(pos);
	else if (transform_location == Circum)
		// get the triangle's circumcenter position
		pos = t->circumCenter();
	else if (transform_location == Box)
		pos = box_center;
	else if (transform_location == Mark)
		// center on the marker
		pos = t->mapFromScene(mark()->center());
	else if (transform_location == NodeO)
		pos = t->getCoords().at(0);
	else if (transform_location == NodeX)
		pos = t->getCoords().at(1);
	else if (transform_location == NodeY)
		pos = t->getCoords().at(2);

	return pos;
}

QPointF FigureEditor::selectionTransformPos()
{
	QPointF pos(0.0, 0.0);
	if (transform_location == Origin)
		pos = selection()->mapFromScene(pos);
	else if (transform_location == Mark)
		pos = selection()->mapFromScene(mark()->center());
	else
		pos = selection()->boundingRect().center();

	return pos;
}

