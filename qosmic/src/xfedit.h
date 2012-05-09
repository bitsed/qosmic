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
#ifndef QOSMIC_XFEDIT_H
#define QOSMIC_XFEDIT_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>

#include "qosmicwidget.h"
#include "genomevector.h"
#include "basistriangle.h"
#include "triangle.h"
#include "nodeitem.h"
#include "coordinatemark.h"
#include "triangleselection.h"
#include "transformablegraphicsguide.h"
#include "posttriangle.h"
#include "undoring.h"


typedef QList<Triangle*> TriangleList;


/**
 * The QGraphicsScene that manages the Triangles.
 */
class FigureEditor : public QGraphicsScene, public QosmicWidget, public UndoStateProvider
{
	Q_OBJECT

	public:
		enum SceneLocation { None, Origin, Mark, Circum, Box, NodeO, NodeX, NodeY, Cursor };
		enum EditMode { Move, Rotate, Scale, Flip };

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
		void setPreviewDensity(int);
		int previewDensity() const;
		void setPreviewDepth(int);
		int previewDepth() const;
		void setPreviewVisible(bool);
		bool previewVisible() const;
		void setGridVisible(bool);
		bool gridVisible() const;
		QColor gridColor() const;
		void setGridColor(const QColor&);
		QColor bgColor() const;
		void setbgColor(QColor);
		void setGuideVisible(bool);
		bool guideVisible() const;
		QColor guideColor() const;
		void setGuideColor(QColor);
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
		SceneLocation centeredScaling();
		void setCenteredScaling(SceneLocation);
		SceneLocation transformLocation();
		void setTransformLocation(SceneLocation);
		void provideState(UndoState*);
		void restoreState(UndoState*);
		EditMode mode() const;
		QPointF triangleTransformPos(Triangle*);
		QPointF triangleTransformPos();
		QPointF selectionTransformPos();
		GenomeVector* genomeVector() const;

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
		QAction* execPopupMenu(const QPoint&);

	signals:
		void coordinateChangeSignal(double, double);
		void triangleSelectedSignal(Triangle*);
		void triangleModifiedSignal(Triangle*);
		void triangleListChangedSignal();
		void undoStateSignal();
		void editModeChangedSignal(FigureEditor::EditMode);

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
		void setMode(FigureEditor::EditMode);
		void updatePreview();

		// graphicsitem horiz and vert flipping
		void flipTriangleHAction();
		void flipTriangleVAction();
		void flipTriangleHPopupAction();
		void flipTriangleVPopupAction();

		// display and enable editing of the selected triangle's post coords
		void editPostTriangle(bool);
		void enableFinalXform(bool);

	protected:
		void mousePressEvent(QGraphicsSceneMouseEvent*);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
		void mouseMoveEvent(QGraphicsSceneMouseEvent*);
		void wheelEvent(QGraphicsSceneWheelEvent* wheelEvent);
		void drawBackground(QPainter*, const QRectF&);
		Triangle* getCurrentOrSelected();
		void adjustSceneRect();
		void createXformPreview();

	private slots:
		void triangleMenuAction(QAction*);

	private:
		QAbstractGraphicsShapeItem* moving;
		QPointF moving_start;
		QPointF box_center;
		QPointF scene_start;
		QGraphicsSimpleTextItem* infoItem;
		TriangleList trianglesList;
		GenomeVector* genomes;
		QGraphicsView* view;
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
		bool guide_visible;
		QColor guide_color;
		TriangleSelection* selectionItem;
		TransformableGraphicsGuide* graphicsGuide;
		PostTriangle* postTriangle;
		SceneLocation centered_scaling;
		SceneLocation transform_location;
		QVector<flam3_xform> xformClip;
		QVector<QVector<double>*> xformPreview;
		EditMode editMode;
		bool move_edge_mode;
		bool has_selection;
		bool is_selecting;
		bool editing_post;
		bool wheel_moved;
		bool menu_visible;
		bool preview_visible;
		int preview_density;
		int preview_depth;
};



#endif

