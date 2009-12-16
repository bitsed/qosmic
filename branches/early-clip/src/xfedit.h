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
#ifndef QOSMIC_XFEDIT_H
#define QOSMIC_XFEDIT_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>

#include "qosmicwidget.h"
#include "flam3util.h"

class BasisTriangle;
class Triangle;
class NodeItem;
class CoordinateMark;
class TriangleSelection;
class PostTriangle;
class GenomeVector;
class UndoState;

typedef QList<Triangle*> TriangleList;

/**
 * The QGraphicsScene that manages the Triangles.
 */
class FigureEditor : public QGraphicsScene, public QosmicWidget
{
	Q_OBJECT

	public:
		void clear();
		QString& getStatus();
		FigureEditor(GenomeVector*, QObject* parent=0);
		~FigureEditor();
		BasisTriangle* basis() const;
		const QMatrix& transform() const;
		CoordinateMark* mark() const;
		void setMarkVisible(bool);
		void setCoordTransform(QMatrix);
		void selectTriangle(Triangle*);
		void selectTriangle(int);
		void triangleModifiedAction(Triangle*);
		QRectF itemsSceneBounds();
		int selectedTriangleIndex();
		Triangle* getSelectedTriangle();
		int getNumberOfTriangles();
		void setGridVisible(bool);
		bool gridVisible() const;
		QColor gridColor() const;
		void setGridColor(const QColor&);
		QColor bgColor() const;
		void setbgColor(QColor);
		void scaleBasis(double dx, double dy);
		bool hasSelection() const;
		TriangleSelection* selection() const;
		PostTriangle* post() const;
		void moveTriangleBy(Triangle* t, double dx, double dy);
		void moveSelectionBy(double dx, double dy);
		bool postEnabled();
		const TriangleList& triangles() const;
		QString getInfoLabel(Triangle*);
		void enableSelection(bool);
		void writeSettings();
		bool centeredScaling();
		void setCenteredScaling(bool);
		void saveUndoState(UndoState*);
		void restoreUndoState(UndoState*);
		void enableFinalXform(bool);

		// graphicsitem horiz and vert flipping
		void flipTriangleHAction(Triangle*, QPointF);
		void flipTriangleVAction(Triangle*, QPointF);
		void flipSelectionHAction(QPointF);
		void flipSelectionVAction(QPointF);
		// graphicsitem rotate and scale methods
		void rotateTriangle(Triangle*, double, QPointF);
		void rotateSelection(double, QPointF);
		void scaleTriangle(Triangle*, double, double, QPointF);
		void scaleSelection(double, double, QPointF);

	signals:
		void coordinateChangeSignal(double, double);
		void triangleSelectedSignal(Triangle*);
		void triangleModifiedSignal(Triangle*);
		void undoStateSignal();

	public slots:
		void reset();
		void addTriangleAction();
		void cutTriangleAction();
		void copyTriangleAction();
		void pasteTriangleAction();
		void removeTriangleAction();
		void resetTriangleCoordsAction();
		void colorChangedAction(double);
		void selectNextTriangle();
		void selectPreviousTriangle();
		void scaleInScene();
		void scaleOutScene();
		void autoScale();

		// graphicsitem horiz and vert flipping
		void flipTriangleHAction();
		void flipTriangleVAction();

		// display and enable editing of the selected triangle's post coords
		void editPostTriangle(bool);

	protected:
		void mousePressEvent(QGraphicsSceneMouseEvent*);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
		void mouseMoveEvent(QGraphicsSceneMouseEvent*);
		void wheelEvent(QGraphicsSceneWheelEvent* wheelEvent);
		void drawBackground(QPainter*, const QRectF&);
		Triangle* getCurrentOrSelected();
		void adjustSceneRect();

	private slots:
		void triangleMenuAction(QAction*);

	private:
		QAbstractGraphicsShapeItem* moving;
		QPointF moving_start;
		QPointF mark_start;
		QGraphicsSimpleTextItem* infoItem;
		TriangleList trianglesList;
		GenomeVector* genomes;
		BasisTriangle* basisTriangle;
		QGraphicsSimpleTextItem* bLabelA;
		QGraphicsSimpleTextItem* bLabelB;
		QGraphicsSimpleTextItem* bLabelC;
		Triangle* selectedTriangle;
		CoordinateMark* coordinateMark;
		QMenu* popupMenu;
		QMenu* triangleMenu;
		QAction* addAction;
		QAction* cutAction;
		QAction* copyAction;
		QAction* pasteAction;
		QAction* resAction;
		QAction* flipHAction;
		QAction* flipVAction;
		QAction* rescaleAction;
		bool grid_visible;
		QColor grid_color;
		QColor bg_color;
		TriangleSelection* selectionItem;
		PostTriangle* postTriangle;
		QVector<flam3_xform> xformClip;
		bool has_selection;
		bool is_selecting;
		bool editing_post;
		bool wheel_moved;
		bool centered_scaling;
		bool menu_visible;
};

#include "genomevector.h"
#include "basistriangle.h"
#include "triangle.h"
#include "nodeitem.h"
#include "coordinatemark.h"
#include "triangleselection.h"
#include "posttriangle.h"
#include "undoring.h"

#endif

