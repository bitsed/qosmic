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
#include <QApplication>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QScrollBar>
#include <QSettings>
#include <cmath>

#include "xfedit.h"
#include "logger.h"

#define genome_ptr (genomes->selectedGenome())

/**
 * The QGraphicsScene that manages the Triangles.
 */
FigureEditor::FigureEditor(GenomeVector* g, QGraphicsView* parent)
	: QGraphicsScene(parent), QosmicWidget(this, "FigureEditor"),
	moving(0), genomes(g), view(parent), selectedTriangle(0), postTriangle(0),
	centered_scaling(None), transform_location(Origin), editMode(Move),
	move_edge_mode(false), has_selection(false), is_selecting(false),
	editing_post(false), menu_visible(false), move_border_size(10)
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
	addAction = new QAction(QIcon(":icons/silk/shape_triangle_add.xpm"),
		tr("Add"),this);
	resAction = new QAction(QIcon(":icons/silk/shape_triangle_go.xpm"),
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
	preview_depth = settings.value("previewdepth", 1).toInt();
	preview_visible = settings.value("previewvisible", false).toBool();
	labels_visible = settings.value("labelsvisible", true).toBool();
	grid_visible = settings.value("gridvisible", true).toBool();
	grid_color = QColor(settings.value("gridcolor", "#999999").toString());
	guide_visible = settings.value("guidevisible", true).toBool();
	guide_color = QColor(settings.value("guidecolor", "#ee0000").toString());
	bg_color   = QColor(settings.value("bgcolor", "#000000").toString());
	centered_scaling = (SceneLocation)settings.value("centeredscaling", false).toInt();
	transform_location = (SceneLocation)settings.value("transformlocation", (int)Origin).toInt();
	editMode = (EditMode)settings.value("editmode", (int)Move).toInt();

	setBackgroundBrush(Qt::black);
	basisTriangle->setGraphicsScene(this);
	addItem(basisTriangle);

	infoItem = new QGraphicsSimpleTextItem();
	infoItem->setFont(QFont("Monospace", 8, QFont::Normal));
	addItem(infoItem);

	selectionItem = new TriangleSelection(this, basisTriangle);
	selectionItem->setPen(QPen(QBrush(Qt::gray), 1, Qt::DashLine));
	selectionItem->setZValue(-1);
	selectionItem->setVisible(false);
	selectionItem->setSelectedType(settings.value("selectiontype", Triangle::Type).toInt());
	addItem(selectionItem);

	// The single GrahicsGuide instance passed around to all TransformableGraphicsItems
	graphicsGuide = new TransformableGraphicsGuide(this);

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
	coordinateMark->setVisible(transform_location == Mark);
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
	delete graphicsGuide;
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
	settings.setValue("previewdepth", preview_depth);
	settings.setValue("previewvisible", preview_visible);
	settings.setValue("labelsvisible", labels_visible);
	settings.setValue("gridvisible", grid_visible);
	settings.setValue("gridcolor", grid_color.name());
	settings.setValue("guidevisible", guide_visible);
	settings.setValue("guidecolor", guide_color.name());
	settings.setValue("bgcolor", bg_color.name());
	settings.setValue("centeredscaling", centered_scaling);
	settings.setValue("transformlocation", transform_location);
	settings.setValue("selectiontype", selectionItem->selectedType());
	settings.setValue("editmode", editMode);
}

void FigureEditor::enableFinalXform(bool enable)
{
	if (enable)
	{
		if (genome_ptr->final_xform_enable == 1)
			selectTriangle(trianglesList.last());
		else
		{
			logFine("FigureEditor::enableFinalXform : enabling final xform");
			flam3_add_xforms(genome_ptr, 1, 0, 1);
			reset();
			selectTriangle(trianglesList.last());
			emit triangleListChangedSignal();
			emit undoStateSignal();
		}
	}
	else
	{
		if (genome_ptr->final_xform_enable == 0)
			logWarn("FigureEditor::enableFinalXform : final xform already disabled");
		else
		{
			logFine("FigureEditor::enableFinalXform : disabling final xform");
			flam3_delete_xform(genome_ptr, genome_ptr->final_xform_index);
			reset();
			selectTriangle(trianglesList.last());
			emit triangleListChangedSignal();
			emit undoStateSignal();
		}
	}
}

void FigureEditor::addTriangleAction()
{
	logFine(QString("FigureEditor::addTriangleAction : adding triangle to slot %1")
		.arg(genome_ptr->num_xforms));
	Util::add_default_xforms(genome_ptr);
	reset();
	logFine(QString("FigureEditor::addTriangleAction : %1 items in triangleList")
		.arg(trianglesList.size()));
	logFine(QString("FigureEditor::addTriangleAction : genome has %1 xforms")
		.arg( genome_ptr->num_xforms ));
	if (genome_ptr->final_xform_enable == 1)
		selectTriangle(trianglesList.size() - 2);
	else
		selectTriangle(trianglesList.last());
	emit triangleListChangedSignal();
	emit undoStateSignal();
}

void FigureEditor::cutTriangleAction()
{
	copyTriangleAction();
	if (has_selection && selectionItem->hasItems())
	{
		TriangleList triangles;
		if (selectionItem->selectedType() == NodeItem::Type)
		{
			foreach (NodeItem* node, selectionItem->nodes())
				triangles << node->triangle();
		}
		else
			triangles = selectionItem->triangles();

		// flam3_delete_xform() moves down the higher indexed xforms after removing an xform,
		// so we remove xforms indexed from highest to lowest
		QList<int> idxs;
		foreach (Triangle* t, triangles)
		{
			if (t->type() == PostTriangle::Type)
			{
				if (!triangles.contains(selectedTriangle))
					t = selectedTriangle;
				else
					continue;
			}
			idxs << trianglesList.indexOf(t, 0);
		}
		qSort(idxs);
		QListIterator<int> i(idxs);
		i.toBack();
		while (i.hasPrevious())
			flam3_delete_xform(genome_ptr, i.previous());
	}
	else
	{
		Triangle* t = getCurrentOrSelected();
		int idx = trianglesList.indexOf(t, 0);
		logFine(QString("FigureEditor::cutTriangleAction : removing triangle %1").arg(idx));
		flam3_delete_xform(genome_ptr, idx);
	}
	reset();
	emit triangleListChangedSignal();
	emit undoStateSignal();
}

void FigureEditor::copyTriangleAction()
{
	if (has_selection && selectionItem->hasItems())
	{
		TriangleList triangles;
		if (selectionItem->selectedType() == NodeItem::Type)
		{
			foreach (NodeItem* node, selectionItem->nodes())
				triangles << node->triangle();
		}
		else
			triangles = selectionItem->triangles();

		xformClip.clear();
		foreach (Triangle* t, triangles)
		{
			if (t->type() == PostTriangle::Type)
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
		if (genome_ptr->final_xform_enable == 1)
			n -= 1; // don't copy over the final xform
		memcpy(genome_ptr->xform + n, xformClip.data(), sizeof(flam3_xform) * size);
		reset();
		selectTriangle(n + size - 1);
		emit triangleListChangedSignal();
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
	emit triangleListChangedSignal();
	emit undoStateSignal();
}

Triangle* FigureEditor::getCurrentOrSelected()
{
	Triangle* t;
	QGraphicsItem* item = itemAt(moving_start, QTransform());
	if (menu_visible && item)
	{
		switch (item->type())
		{
			case Triangle::Type:
				t = dynamic_cast<Triangle*>(item);
				break;
			case NodeItem::Type:
				t = dynamic_cast<NodeItem*>(item)->triangle();
				if (t->type() != PostTriangle::Type)
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
	QGraphicsItem* item = itemAt(moving_start, QTransform());
	if (hasFocus() && menu_visible && item)
	{
		switch (item->type())
		{
			case Triangle::Type:
			case PostTriangle::Type:
				t = dynamic_cast<Triangle*>(item);
				break;
			default:
				;
		}
	}
	else if (postEnabled())
		t = dynamic_cast<Triangle*>(post());

	t->resetPosition();
	triangleModifiedAction(t);
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
		execPopupMenu(e->screenPos());
		moving_start = QPointF(0.0, 0.0);
	}
	else if (e->button() == Qt::MidButton)
	{
		moving = 0;
	}
	else if (e->modifiers() & Qt::ControlModifier && !(e->modifiers() & Qt::ShiftModifier))
	{
		// LeftButton+Ctrl to activate selection and start selecting items
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
			graphicsGuide->update();
			update();
		}
	}
	else // LeftButton, possibly with control+shift modifiers
	{
		QGraphicsItem* item = itemAt(moving_start, QTransform());
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

			switch (item->type())
			{
				case Triangle::Type:
					selectTriangle(dynamic_cast<Triangle*>(item));

				case PostTriangle::Type:
				{
					moving = dynamic_cast<Triangle*>(item);
					break;
				}

				case NodeItem::Type:
				{
					NodeItem* node = dynamic_cast<NodeItem*>(item);
					Triangle* t = node->triangle();
					node->setZValue(t->nextZPos());
					moving = node;
					if (t->type() == Triangle::Type)
						selectTriangle(t);
					break;
				}

				case GraphicsGuideScaleButton::Type:
				{
					GraphicsGuideScaleButton* b = dynamic_cast<GraphicsGuideScaleButton*>(item);
					moving = b;
				}

				case TriangleSelection::Type:
					break;

				default:
					logWarn(QString("FigureEditor::mousePressEvent : unknown type %1")
						.arg(item->type()));
			}

			// holding control+shift will activate and add an item to the selection
			if (e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))
			{
				// select the triangle for a node if selecting triangles
				if (selectionItem->selectedType() == Triangle::Type
					&& item->type() == NodeItem::Type)
					item = dynamic_cast<NodeItem*>(item)->triangle();

				if (!(item == selectionItem || selectionItem->contains(item))
					&& (item->type() == selectionItem->selectedType()
						|| ( item->type() == PostTriangle::Type
							&& selectionItem->selectedType() == Triangle::Type) ) )
				{
					selectionItem->addItem(item);
					QPolygonF poly;
					foreach (QGraphicsItem* item, selectionItem->allItems())
						poly = poly.united(QPolygonF(item->sceneBoundingRect()));
					selectionItem->setPolygon(poly.boundingRect());
					selectionItem->setZValue(selectedTriangle->nextZPos());
					selectionItem->setVisible(true);
					has_selection = true;
					selectionItem->setGraphicsGuide(graphicsGuide);
				}
			}

			// if the selection is active, then only ever select it
			if (hasSelection() && (item->type() != GraphicsGuideScaleButton::Type))
			{
				if (item->type() != TriangleSelection::Type)
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
	}
	QGraphicsScene::mousePressEvent(e);
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
			{
				editMode = Move;
				move_edge_mode = false;
				graphicsGuide->update();
			}
			else if (editMode == Move)
				setTransformLocation(transform_location);
			update();
			if (dz != QPointF(0.0, 0.0) || wheel_moved)
				emit undoStateSignal();
		}
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
			if (editing_post)
				postTriangle->setGraphicsGuide(graphicsGuide);
			else
				selectedTriangle->setGraphicsGuide(graphicsGuide);
			has_selection = false;
		}
		else
		{
			selectionItem->setZValue(selectedTriangle->nextZPos());
			selectionItem->selectCoveredItems();
			has_selection = true;
			selectionItem->setGraphicsGuide(graphicsGuide);
			emit undoStateSignal();
		}
		update();
	}
	view->setCursor(Qt::ArrowCursor);
	moving_start = QPointF(0.0,0.0);
	QGraphicsScene::mouseReleaseEvent(e);
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
	QGraphicsItem* item = itemAt(scenePos, QTransform());

	if ( moving )
	{
		infoItem->hide();
		lastt = 0;

		if ( moving->type() == TriangleSelection::Type )
		{
			if ( editMode == Move )
			{
				QPointF pt( selectedTriangle->mapFromScene( dx, dy ) );
				QPointF p( selectionItem->mapFromItem(selectedTriangle, pt) );
				scenePos = moveAnItem(selectionItem, e, p.x(), p.y());
			}
			else if ( editMode == Rotate )
			{
				if (qAbs(dx) >= qAbs(dy))
				{
					if (e->modifiers() & Qt::ShiftModifier)
						dx /= 10.0;
					else if (e->modifiers() & Qt::ControlModifier)
						dx *= 10.0;
					rotateSelection(dx, selectionTransformPos());
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
					scaleSelection(dx, dx, selectionTransformPos());
				}
			}
		}
		else if ( moving->type() == GraphicsGuideScaleButton::Type )
		{
			QGraphicsItem* parent( moving->parentItem() );
			QGraphicsItem* t = dynamic_cast<QGraphicsItem*>( parent->parentItem() );
			QRectF mrect( moving->mapRectToScene(moving->boundingRect()) );
			QPointF bpos( mrect.center() );
			QPointF tpos;
			if (t->type() == TriangleSelection::Type)
				tpos = selectionTransformPos();
			else
				tpos = triangleTransformPos();
			QPointF cen( t->mapToScene(tpos) );
			qreal dx( qAbs(bpos.x() - cen.x()) );
			qreal dy( qAbs(bpos.y() - cen.y()) );

			if (dx != 0.0 && dy != 0.0)
			{
				qreal sx( qAbs(scenePos.x() - cen.x()) / dx );
				qreal sy( qAbs(scenePos.y() - cen.y()) / dy );

				QTransform trans( parent->transform() );
				trans.scale(sx,sy);
				QRectF trect( t->mapRectToScene(trans.mapRect(parent->boundingRect())) );
				qreal blen = mrect.width();
				if ( (trect.width() < blen*2.0) )
					sx = 1.0;
				if ( (trect.height() < blen*2.0) )
					sy = 1.0;
				if ((sx != 1.0) || (sy != 1.0))
				{
					if (t->type() == TriangleSelection::Type)
						scaleSelection(qMin(sx, 1.15), qMin(sy, 1.15), tpos);
					else
						scaleTriangle(dynamic_cast<Triangle*>(t), qMin(sx, 1.15), qMin(sy, 1.15), tpos);
				}
			}
		}
		else //  moving->type() == Triangle::Type || PostTriangle::Type || NoteItem::Type
		{
			Triangle* t = 0;
			NodeItem* node = 0;
			if (moving->type() == NodeItem::Type)
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
					if (node)
					{
						t->rotateNode(node, dx, triangleTransformPos());
						if (editing_post)
							t = selectedTriangle;
						triangleModifiedAction(t);
					}
					else
						rotateTriangle(t, dx, triangleTransformPos());
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
				}
			}
			else if ( editMode == Flip )
			{
				QPointF dz (e->buttonDownScenePos(Qt::LeftButton) - e->scenePos());
				qreal dx = dz.x();
				qreal dy = dz.y();
				if (qAbs(dx) > qAbs(dy))
					view->setCursor(Qt::SplitHCursor);
				else if (qAbs(dx) < qAbs(dy))
					view->setCursor(Qt::SplitVCursor);
				else
					view->setCursor(Qt::ArrowCursor);
			}
			else // Move mode
				scenePos = moveAnItem(moving, e, dx, dy);
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
			graphicsGuide->update();
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
			QScrollBar* hbar = view->horizontalScrollBar();
			QScrollBar* vbar = view->verticalScrollBar();
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
			graphicsGuide->update();
			update();
		}

		else
		{
			// otherwise scroll the qgraphicsview
			QScrollBar* hbar = view->horizontalScrollBar();
			QScrollBar* vbar = view->verticalScrollBar();
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
			case NodeItem::Type:
				item = dynamic_cast<NodeItem*>(item)->triangle();

			case Triangle::Type:
				if (item->type() != PostTriangle::Type)
				{
					// show the xform's infoItem for this triangle
					Triangle* t = dynamic_cast<Triangle*>(item);
					if (lastt != t)
					{
						infoItem->setText(getInfoLabel(t));
						QPoint pt = QPoint(5,
								view->maximumViewportSize().height()
								+ 10 - (int)infoItem->boundingRect().height());
								// 10 units of fudge
						infoItem->setPos(view->mapToScene(pt));
						infoItem->setBrush(QBrush(t->pen().color()));
						infoItem->setZValue(selectedTriangle->zValue());
						infoItem->show();
						lastt = t;
					}
					break;
				}
			case PostTriangle::Type:
			{
				Triangle* t = selectedTriangle;
				int tidx = trianglesList.indexOf(t);
				infoItem->setText(tr("post transform: %1\n").arg(tidx + 1));
				QPoint pt = QPoint(5,
						view->maximumViewportSize().height()
						+ 10 - (int)infoItem->boundingRect().height());
						// 10 units of fudge
				infoItem->setPos(view->mapToScene(pt));
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
	}
	emit coordinateChangeSignal( scenePos.x(), scenePos.y() );
	QGraphicsScene::mouseMoveEvent(e);
}


QPointF FigureEditor::moveAnItem(QGraphicsItem* item, QGraphicsSceneMouseEvent* e, int dx, int dy)
{
	const QRect vr(view->viewport()->rect().adjusted(
		 move_border_size,
		 move_border_size,
		-move_border_size,
		-move_border_size));

	if (moveScrollMutex.tryLock())
	{
		view->setCursor(Qt::SizeAllCursor);
		m_vrect = view->mapToScene(vr).boundingRect();
		if (!m_vrect.contains(e->scenePos()))
		{
			// otherwise scroll the qgraphicsview
			QScrollBar* hbar = view->horizontalScrollBar();
			QScrollBar* vbar = view->verticalScrollBar();

			m_scenePos = e->scenePos();
			m_lastScenePos = e->lastScenePos();

			int dn = 0, dx, dy;
			if (m_scenePos.x() < m_vrect.left())
				dn = qAbs(m_scenePos.x() - m_vrect.left());

			if (m_scenePos.x() > m_vrect.right())
				dn = qAbs(m_scenePos.x() - m_vrect.right());

			if (m_scenePos.y() < m_vrect.top())
				dn = qAbs(m_scenePos.y() - m_vrect.top());

			if (m_scenePos.y() > m_vrect.bottom())
				dn = qAbs(m_scenePos.y() - m_vrect.bottom());

			while (QApplication::mouseButtons() & Qt::LeftButton)
			{
				if (m_vrect.contains(m_scenePos))
					break;

				double dxx = 0;
				double dyy = 0;
				int dt = 0;
				dx = dy = 0;

				if (QApplication::keyboardModifiers() & Qt::ControlModifier)
					dt = dn;
				else if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
					dt = 1;
				else
					dt = 2;

				if (m_scenePos.x() >= m_vrect.right())
				{
					dx = dt;
					if (m_scenePos.y() < m_vrect.bottom()
					&& m_scenePos.y() > m_vrect.top())
						dyy = m_scenePos.y() - m_lastScenePos.y();
				}
				if (m_scenePos.y() >= m_vrect.bottom())
				{
					dy = dt;
					if (m_scenePos.x() < m_vrect.right()
						&& m_scenePos.x() > m_vrect.left())
						dxx = m_scenePos.x() - m_lastScenePos.x();
				}
				if (m_scenePos.y() <= m_vrect.top())
				{
					dy = -dt;
					if (m_scenePos.x() < m_vrect.right()
						&& m_scenePos.x() > m_vrect.left())
					dxx = m_scenePos.x() - m_lastScenePos.x();
				}
				if (m_scenePos.x() <= m_vrect.left())
				{
					dx = -dt;
					if (m_scenePos.y() < m_vrect.bottom()
						&& m_scenePos.y() > m_vrect.top())
					dyy = m_scenePos.y() - m_lastScenePos.y();
				}

				moveItemBy(item, dx + dxx, dy + dyy);
				hbar->setValue(hbar->value() + dx);
				vbar->setValue(vbar->value() + dy);
				m_scenePos = m_lastScenePos;
				QCoreApplication::processEvents();
			}
		}
		else
		{
			moveItemBy(item, dx, dy);
			m_scenePos = e->scenePos();
		}
		moveScrollMutex.unlock();
	}
	else
	{
		m_scenePos = e->scenePos();
		m_lastScenePos = e->lastScenePos();
		m_vrect = view->mapToScene(vr).boundingRect();
	}
	return m_scenePos;
}

void FigureEditor::moveItemBy(QGraphicsItem* item, int dx, int dy)
{
	switch (item->type())
	{
		case PostTriangle::Type:
		case Triangle::Type:
		{
			dynamic_cast<Triangle*>(item)->moveBy(dx, dy); // need a virtual moveBy()
			triangleModifiedAction(selectedTriangle);
			break;
		}
		case NodeItem::Type:
		{
			dynamic_cast<NodeItem*>(item)->moveBy(dx, dy);
			triangleModifiedAction(selectedTriangle);
			break;
		}
		case TriangleSelection::Type:
		{
			TriangleSelection* selectionItem = dynamic_cast<TriangleSelection*>(item);
			selectionItem->moveBy(dx, dy);
			if (selectionItem->hasItems())
			{
				if (selectionItem->containsAnyOf(selectedTriangle))
					triangleModifiedAction(selectedTriangle);
				else
					emit triangleModifiedSignal(selectionItem->first());
			}
			break;
		}
		default:
			logWarn(QString("FigureEditor::moveItemBy : unknown item type %1").arg(item->type()));
	}
}

void FigureEditor::wheelEvent(QGraphicsSceneWheelEvent* e)
{
	QPointF p = e->scenePos();
	QGraphicsItem* item = itemAt(p, QTransform());
	if (item)
	{
		switch (item->type())
		{
			case Triangle::Type:
			case PostTriangle::Type:
			{
				double rad;
				Triangle* t;
				if (item->type() == Triangle::Type)
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
						rad = (double)e->delta() / (128.*M_PI);
					else
						rad = (double)e->delta() / (16.*M_PI);
					rotateTriangle(t, rad, cpos);
				}
				e->accept();
				break;
			}
			case BasisTriangle::Type:
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
			case TriangleSelection::Type:
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
						rad = (double)e->delta() / (128.*M_PI);
					else
						rad = (double)e->delta() / (16.*M_PI);
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
	if (t->type() == PostTriangle::Type)
		t = selectedTriangle;
	selectTriangle(t);
	updatePreview();
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
	emit undoStateSignal();
}

void FigureEditor::flipTriangleVAction()
{
	if (hasSelection())
		flipSelectionVAction(selectionTransformPos());
	else if (postEnabled())
		flipTriangleVAction(post(), triangleTransformPos());
	else
		flipTriangleVAction(selectedTriangle, triangleTransformPos());
	emit undoStateSignal();
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
	emit undoStateSignal();
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
	emit undoStateSignal();
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
	blockSignals(true);
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
	QColor c(Util::get_xform_color(genome_ptr, selectedTriangle->xform()));
	selectedTriangle->setPen(QPen(c, 0, Qt::SolidLine));
	c.setAlphaF(0.5);
	QBrush brush(c, Qt::SolidPattern);
	brush.setMatrix(basisTriangle->coordTransform());
	selectedTriangle->setBrush(brush);
	selectedTriangle->moveToFront();
	box_center = selectedTriangle->polygon().boundingRect().center();
	// the guide decorator is applied to the selected triangle in editPostTriangle()
	editPostTriangle(post);
	blockSignals(false);
	updatePreview();
	emit triangleSelectedSignal(t);
}

void FigureEditor::createXformPreview()
{
	int xi = selectedTriangleIndex();
	logFiner(QString("FigureEditor::createXformPreview : xi = %1").arg(xi));
	double range = 1.0;
	int numvals = preview_density;
	int depth = preview_depth;
	int rcnt = 2* (2*numvals+1)*(2*numvals+1);
	QVector<double>* result;
	randctx* rc = Util::get_isaac_randctx();
	flam3_genome g = flam3_genome();
	for (int n = 0 ; n < depth ; n++)
	{
		if (n < xformPreview.size())
		{
			result = xformPreview.at(n);
			result->resize(rcnt);
		}
		else
		{
			result = new QVector<double>(rcnt);
			xformPreview.append(result);
		}
		flam3_copy(&g, genomes->selectedGenome());
		flam3_xform_preview(&g, xi, range, numvals, n+1, result->data(), rc);
	}
	clear_cp(&g, flam3_defaults_on);
	while ( xformPreview.size() > depth )
	{
		QVector<double>*result = xformPreview.last();
		xformPreview.pop_back();
		delete result;
	}
}

void FigureEditor::updatePreview()
{
	if (preview_visible)
	{
		createXformPreview();
		update();
	}
}

QAction* FigureEditor::execPopupMenu(const QPoint& p)
{
	// setup the triangle popup menu
	triangleMenu->clear();
	for (int n = 0 ; n < genome_ptr->num_xforms ; n++)
	{
		Triangle* t = trianglesList.at(n);
		QSize pix_size(64, 64); // the icon size
		QPolygonF poly = t->getCoords();
		QRectF prect = poly.boundingRect();
		QPointF pcenter = prect.center();
		qreal min_side = qMax(prect.width(), prect.height());
		qreal scale = pix_size.width() / min_side;
		QTransform ts(QTransform().scale(scale, scale));
		QTransform tf(ts.m11(), ts.m12(), ts.m13(),
		ts.m21(), -ts.m22(), ts.m23(),
		ts.m31(), ts.m32(), ts.m33());
		poly.translate(-pcenter.x(), -pcenter.y());
		poly = tf.map(poly);
		poly.translate(pix_size.width() / 2, pix_size.height() / 2);

		QPixmap pix(pix_size);
		QPainter p(&pix);
		p.fillRect(QRect(QPoint(0, 0), pix_size), bgColor());
		const QPen pen(Util::get_xform_color(genome_ptr, t->xform()), 0, Qt::DashLine);
		p.setPen(pen);
		p.setBrush(QBrush(pen.color(), Qt::SolidPattern));
		p.drawPolygon(poly);

		QString action_name = QString("%1").arg(n+1);
		QAction* a = triangleMenu->addAction(action_name);
		a->setObjectName(action_name);
		a->setCheckable(false);
		a->setIconVisibleInMenu(true);
		a->setIcon(QIcon(pix));
	}
	triangleMenu->setActiveAction(triangleMenu->actions().value(selectedTriangleIndex()));
	menu_visible = true;
	QAction* a = popupMenu->exec(p);
	menu_visible = false;
	return a;
}

void FigureEditor::reset()
{
	logFine("FigureEditor::reset : entered");
	if (genome_ptr->num_xforms < 1)
	{
		logWarn("FigureEditor::reset : no xforms, adding a default");
		Util::add_default_xforms(genome_ptr);
	}

	int selected_idx = 0;
	if (selectedTriangle)
		selected_idx = selectedTriangleIndex();

	bool hasPost = editing_post;
	if (hasPost)
	{
		// hide/disable the post triangle and recreate it after the reset
		box_center = selectedTriangle->polygon().boundingRect().center();
		postTriangle->setVisible(false);
		editing_post = false;
	}

	if (has_selection)
		enableSelection(false);

	// make sure there are as many triangles as xforms
	int num_triangles = trianglesList.size();
	int dn = genome_ptr->num_xforms - num_triangles;
	if (dn > 0)
		for (int n = num_triangles ; n < dn + num_triangles ; n++)
		{
			Triangle* t = new Triangle(this, genome_ptr->xform + n, basisTriangle, n);
			t->setLabelVisible(labels_visible);
			trianglesList << t;
			addItem(t);
		}
	else if (dn < 0)
	{
		// Temporarily re-parent the graphicsGuide in case its parent triangle is deleted
		trianglesList.first()->setGraphicsGuide(graphicsGuide);
		for (int n = 0 ; n > dn ; n--)
		{
			Triangle* t = trianglesList.takeLast();
			removeItem(t);
			delete t;
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
	const QPen pen(grid_color, 0);
	bLabelA->setBrush(b);
	bLabelA->setPos(basisTriangle->mapToScene(basisTriangle->A) + QPointF(-8.0,0.0));
	bLabelB->setBrush(b);
	bLabelB->setPos(basisTriangle->mapToScene(basisTriangle->B));
	bLabelC->setBrush(b);
	bLabelC->setPos(basisTriangle->mapToScene(basisTriangle->C) + QPointF(-8.0,-16.0));
	basisTriangle->setPen(pen);
	coordinateMark->setPen(pen);
	selectionItem->setPen(QPen(b, 0, Qt::DashLine));

	selectedTriangle = 0;
	editing_post = hasPost; // there's a side-effect here that selectTriangle
							// will recreate the postTriangle with this set
	if (selected_idx < trianglesList.size())
		selectTriangle(trianglesList.at(selected_idx));
	else
		selectTriangle(trianglesList.last());

	// recenter the scene on the current center point in the view
	QPointF view_center( view->mapToScene(view->frameRect()).boundingRect().center() );
	transform().inverted().map(view_center.x(), view_center.y(), &scene_start.rx(), &scene_start.ry());
	adjustSceneRect();

	logFiner(QString("FigureEditor::reset : sceneRect %1,%2")
			.arg(sceneRect().width())
			.arg(sceneRect().height()));
	logFiner(QString("FigureEditor::reset : itemsRect %1,%2")
			.arg(itemsBoundingRect().size().width())
			.arg(itemsBoundingRect().size().height()));
	logFiner(QString("FigureEditor::reset : viewport %1,%2")
			.arg(view->maximumViewportSize().width())
			.arg(view->maximumViewportSize().height()));
}

void FigureEditor::findViewCenter()
{
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
		view->centerOn(basisTriangle->mapToScene(pos));
	}
	else if (!moving_start.isNull())
		view->centerOn(moving_start);
	else
		view->centerOn(basisTriangle->mapToScene(scene_start));
}

void FigureEditor::adjustSceneRect()
{
	// Adjust the sceneRect so it covers just more than the items.  This
	// helps reposition the sceneview and adjusts the scrollbars.  It also
	// helps avoid some "jumpiness" that sometimes happens when moving
	// a graphicsitem outside of the sceneRect.
	setSceneRect(itemsSceneBounds().adjusted(-200.,-200.,200.,200.));

	if (infoItem->isVisible())
	{
		// put the infoItem back
		QPoint pt = QPoint(5,
			view->maximumViewportSize().height()
			+ 10 - (int)infoItem->boundingRect().height());
			// 10 units of fudge
			infoItem->setPos(view->mapToScene(pt));
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
		int numvals = preview_density;
		int depth = preview_depth;
		int rcnt = 2* (2*numvals+1)*(2*numvals+1);
		p->setPen(selectedTriangle->pen());
		for (int n = 0 ; n < depth ; n++)
		{
			double* result = xformPreview.at(n)->data();
			for (int i = 0 ; i < rcnt  ; i+=2)
				p->drawPoint(selectedTriangle->mapToScene(result[i], -(result[i+1])));
		}
	}
}

void FigureEditor::colorChangedAction(double /*idx*/)
{
	foreach (Triangle* triangle, trianglesList)
	{
		QColor c( Util::get_xform_color(genome_ptr, triangle->xform()) );
		QPen pen(triangle->pen());
		QBrush brush(triangle->brush());
		pen.setColor(c);
		triangle->setPen(pen);
		foreach (NodeItem* n, triangle->getNodes())
			n->setPen(QPen(grid_color), c);
		c.setAlphaF(0.5);
		brush.setColor(c);
		triangle->setBrush(brush);
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
	graphicsGuide->setVisible(value);
	update();
}

int FigureEditor::previewDensity() const
{
	return preview_density;
}

void FigureEditor::setPreviewDensity(int value)
{
	preview_density = value;
}

int FigureEditor::previewDepth() const
{
	return preview_depth;
}

void FigureEditor::setPreviewDepth(int value)
{
	preview_depth = value;
}

bool FigureEditor::previewVisible() const
{
	return preview_visible;
}

void FigureEditor::setPreviewVisible(bool value)
{
	preview_visible = value;
	updatePreview();
	if(!value) update(); // call update() when !value to hide the preview
}

bool FigureEditor::labelsVisible() const
{
	return labels_visible;
}

void FigureEditor::setLabelsVisible(bool value)
{
	labels_visible = value;
	foreach (Triangle* t, trianglesList)
		t->setLabelVisible(labels_visible);
}

QColor FigureEditor::gridColor() const
{
	return grid_color;
}

void FigureEditor::setGridColor(const QColor& c)
{
	grid_color = c;
	const QPen p(grid_color, 0);
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
	graphicsGuide->update();
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
		view->ensureVisible(t->sceneBoundingRect());
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
	QSize v_size(view->maximumViewportSize());
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
	view->ensureVisible(itemsSceneBounds(), 0, 0);
	adjustSceneRect();
	findViewCenter();
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

	// recenter the scene on the current center point in the view
	QPointF view_center( view->mapToScene(view->frameRect()).boundingRect().center() );
	transform().inverted().map(view_center.x(), view_center.y(), &scene_start.rx(), &scene_start.ry());

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
		QTransform trans(selectionItem->transform());
		trans.scale(dx, dy);
		selectionItem->setPolygon(trans.map(selectionItem->polygon()));
	}

	// put back the mark
	transform().map(mark_start.x(), mark_start.y(), &mark.rx(), &mark.ry());
	coordinateMark->centerOn(mark);

	// put back the moving_start position
	transform().map(cursor_start.x(), cursor_start.y(), &moving_start.rx(), &moving_start.ry());

	// update the guide dimensions
	graphicsGuide->update();

	// adjust scene rect and repaint
	adjustSceneRect();
	findViewCenter();
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
	{
		if (selectionItem->containsAnyOf(selectedTriangle))
			triangleModifiedAction(selectedTriangle);
		else
			emit triangleModifiedSignal(selectionItem->first());
	}
}

void FigureEditor::scaleSelection( double dx, double dy, QPointF pos )
{
	selectionItem->scale(dx, dy, pos);
	logFine(QString("triangle set scaled dx %1 (%2,%3)")
				.arg(dx)
				.arg(pos.x())
				.arg(pos.y()));
	if (selectionItem->hasItems())
	{
		if (selectionItem->containsAnyOf(selectedTriangle))
			triangleModifiedAction(selectedTriangle);
		else
			emit triangleModifiedSignal(selectionItem->first());
	}
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
	if (t->type() == PostTriangle::Type)
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
	{
		if (selectionItem->containsAnyOf(selectedTriangle))
			triangleModifiedAction(selectedTriangle);
		else
			emit triangleModifiedSignal(selectionItem->first());
	}
}


void FigureEditor::editPostTriangle(bool flag)
{
	editing_post = flag;

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
		const QPen pen(grid_color, 0);
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
		if (!has_selection) // don't steal the guide from the selection
			postTriangle->setGraphicsGuide(graphicsGuide);
	}
	else
	{
		postTriangle->setVisible(false);
		box_center = selectedTriangle->polygon().boundingRect().center();
		if (!has_selection)
			selectedTriangle->setGraphicsGuide(graphicsGuide);
	}

	update();
	emit triangleSelectedSignal(selectedTriangle);
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
	graphicsGuide->update();
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
	graphicsGuide->update();
}

void FigureEditor::provideState(UndoState* state)
{
	logFine("FigureEditor::provideState : setting state");
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
		case NodeItem::Type:
			foreach (NodeItem* node, selectionItem->nodes())
			{
				if (node->id() == Triangle::NODE_O)
					if (node->triangle()->type() == PostTriangle::Type)
						state->NodesO.append(-1);
					else
						state->NodesO.append(node->triangle()->index());

				else if (node->id() == Triangle::NODE_X)
					if (node->triangle()->type() == PostTriangle::Type)
						state->NodesX.append(-1);
					else
						state->NodesX.append(node->triangle()->index());

				else if (node->id() == Triangle::NODE_Y)
					if (node->triangle()->type() == PostTriangle::Type)
						state->NodesY.append(-1);
					else
						state->NodesY.append(node->triangle()->index());

				else
					logWarn(QString("FigureEditor::saveUndoState : found node with unknown name %1").arg(node->name()));
			}
			break;

		case Triangle::Type:
			foreach (Triangle* triangle, selectionItem->triangles())
				if (triangle->type() == PostTriangle::Type)
					state->Triangles.append(-1);
				else
					state->Triangles.append(triangle->index());
			break;

		default:
			;
	}
	state->MarkPos = itrans.map(coordinateMark->center());
}

void FigureEditor::restoreState(UndoState* state)
{
	logFine("FigureEditor::restoreState : restoring state");
	reset();
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
			case NodeItem::Type:
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

			case Triangle::Type:
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

GenomeVector* FigureEditor::genomeVector() const
{
	return genomes;
}

