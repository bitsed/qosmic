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
#ifndef TRIANGLESELECTION_H
#define TRIANGLESELECTION_H

#include <QGraphicsPolygonItem>

class Triangle;
class FigureEditor;
class BasisTriangle;
class NodeItem;

class TriangleSelection : public QGraphicsPolygonItem
{
	private:
		FigureEditor* m_editor;
		BasisTriangle* m_basis;
		QList<Triangle*> m_triangles;
		QList<NodeItem*> m_nodes;
		int m_selectType;

	public:
		TriangleSelection(FigureEditor*, BasisTriangle*);
		~TriangleSelection();
		int type() const;
		BasisTriangle* basis() const;
		FigureEditor* editor() const;
		void moveBy(double, double);
		void flipHorizontally(QPointF);
		void flipVertically(QPointF);
		void scale(double, double, QPointF);
		void rotate(double, QPointF);
		int selectedType() const;
		void setSelectedType(int);
		void selectCoveredItems();
		bool hasItems() const;
		void clear();
		QList<Triangle*> triangles() const;
		QList<NodeItem*> nodes() const;
		Triangle* first();
		void addItem(QGraphicsItem*);
		QList<QGraphicsItem*> allItems() const;
		bool contains(QGraphicsItem* item) const;
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget =0);

		static const int RTTI = 2020202;
};

#include "xfedit.h"

#endif
