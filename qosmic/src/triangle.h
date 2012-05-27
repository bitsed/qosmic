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
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "basistriangle.h"
#include "flam3util.h"
#include "transformablegraphicsitem.h"

class NodeItem;
class FigureEditor;
class TransformableGraphicsGuide;

typedef QList<NodeItem*>           TriangleNodes;
typedef QListIterator<NodeItem*>   TriangleNodesIterator;

class Triangle : public QGraphicsPolygonItem, public TransformableGraphicsItem
{
	public:
		static const int RTTI = 349495;
		static const int NODE_O = 0;
		static const int NODE_X = 1;
		static const int NODE_Y = 2;

		enum EdgeType { NoEdge, RotateEdge, ScaleEdge };

	protected:
		FigureEditor* canvas;
		flam3_xform* m_xform;
		BasisTriangle* basisTriangle;
		int m_index;
		TriangleCoords cList;
		TriangleNodes  nList;
		QRectF m_boundingRect;
		static uint zpos;
		double m_scaled;
		double m_rotated;
		QLineF m_edgeLine;
		EdgeType m_edgeType;
		QGraphicsSimpleTextItem m_idxLabel;

	public:
		Triangle( FigureEditor*, flam3_xform*, BasisTriangle*, int idx );
		~Triangle();
		flam3_xform* xform();
		void setXform(flam3_xform*);
		void moveEdges();
		void moveBy(double, double);
		void setPoints(TriangleCoords&);
		void moveToFront();
		void show();
		void setVisible(bool);
		virtual int type() const;
		int nextZPos();
		const QMatrix& getCoordinateTransform();
		TriangleNodes& getNodes();
		TriangleCoords getCoords();
		virtual void coordsToXForm();
		QRectF boundingRect();
		void flipHorizontally();
		void flipVertically();
		void flipHorizontally(QPointF);
		void flipVertically(QPointF);
		void scale(double, double, QPointF);
		void rotate(double, QPointF);
		void rotateNode(NodeItem*, double, QPointF);
		BasisTriangle* basis() const;
		void setNodeColor(const QColor&, const QColor&);
		FigureEditor* editor() const;
		void adjustSceneRect();
		virtual void resetPosition();
		void basisScaledSlot();
		int index() const;
		NodeItem* getNode(int) const;
		EdgeType getEdgeType();
		bool labelVisible() const;
		void setLabelVisible(bool);
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget =0);
		QPointF circumCenter();

	protected:
		void findEdge(const QPointF&);
		void clearFoundEdge();
		void hoverEnterEvent(QGraphicsSceneHoverEvent*);
		void hoverMoveEvent(QGraphicsSceneHoverEvent*);
		void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
};


#endif
