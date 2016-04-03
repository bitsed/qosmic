/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2011 by David Bitseff                 *
 *   bitsed@gmail.com                                                      *
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
#include "triangleselection.h"
#include "xfedit.h"
#include "logger.h"

TriangleSelection::TriangleSelection(FigureEditor* f, BasisTriangle* b)
	: QGraphicsPolygonItem(QPolygonF(QRectF(0.,0.,1.,1.))),
		m_editor(f), m_basis(b), m_selectType(Triangle::RTTI)
{
}

TriangleSelection::~TriangleSelection()
{
}

int TriangleSelection::type() const
{
	return RTTI;
}

void TriangleSelection::moveBy( double dx, double dy )
{
	foreach (Triangle* t, m_triangles)
		t->moveBy(dx,dy);
	foreach(NodeItem* node, m_nodes)
		node->moveBy(dx,dy);
	QPolygonF pa = polygon();
	pa.translate(dx, dy);
	setPolygon(pa);
}


void TriangleSelection::rotate( double rad, QPointF cpos )
{
	foreach (Triangle* t, m_triangles)
		t->rotate(rad, t->mapFromItem(this, cpos));
	QTransform trans = transform();
	trans.translate(cpos.x(), cpos.y());
	trans.rotate(-rad);
	trans.translate(-cpos.x(), -cpos.y());
	setPolygon(trans.map(polygon()));
	foreach (NodeItem* n, m_nodes)
	{
		n->setPos(trans.map(n->pos()));
		n->triangle()->moveEdges();
	}
	if (m_guide)
		m_guide->update();
}

void TriangleSelection::scale( double dx, double dy, QPointF cpos )
{
	foreach (Triangle* t, m_triangles)
		t->scale(dx, dy, t->mapFromItem(this, cpos));
	QTransform trans = transform();
	trans.translate(cpos.x(), cpos.y());
	trans.scale(dx, dy);
	trans.translate(-cpos.x(), -cpos.y());
	setPolygon(trans.map(polygon()));
	foreach (NodeItem* n, m_nodes)
	{
		n->setPos(trans.map(n->pos()));
		n->triangle()->moveEdges();
	}
	if (m_guide)
		m_guide->update();
}

void TriangleSelection::flipHorizontally(QPointF cpos)
{
	double tx = cpos.x();
	foreach (Triangle* t, m_triangles)
		t->flipHorizontally(t->mapFromItem(this, cpos));
	foreach (NodeItem* n, m_nodes)
	{
		QPointF p(n->pos());
		p.rx() = p.x() - 2.*(p.x() - tx) ;
		n->setPos(p);
		n->triangle()->moveEdges();
	}
	QPolygonF pa;
	foreach (QPointF p, polygon())
	{
		p.rx()  =  p.x() - 2.*(p.x() - tx) ;
		pa << p;
	}
	setPolygon(pa);
	if (m_guide)
		m_guide->update();
}

void TriangleSelection::flipVertically(QPointF cpos)
{
	double ty = cpos.y();
	foreach (Triangle* t, m_triangles)
		t->flipVertically(t->mapFromItem(this, cpos));
	foreach (NodeItem* n, m_nodes)
	{
		QPointF p(n->pos());
		p.ry() = p.y() - 2.*(p.y() - ty) ;
		n->setPos(p);
		n->triangle()->moveEdges();
	}
	QPolygonF pa;
	foreach (QPointF p, polygon())
	{
		p.ry()  =  p.y() - 2.*(p.y() - ty) ;
		pa << p;
	}
	setPolygon(pa);
	if (m_guide)
		m_guide->update();
}

void TriangleSelection::selectCoveredItems()
{
	switch (m_selectType)
	{
		case Triangle::RTTI:
		{
			QList<QGraphicsItem*> collisions = collidingItems();
			m_triangles.clear();
			foreach (QGraphicsItem* item, collisions)
			switch (item->type())
			{
				case Triangle::RTTI:
				case PostTriangle::RTTI:
				m_triangles.append(dynamic_cast<Triangle*>(item));
			}
			break;
		}
		case NodeItem::RTTI:
		{
			QList<QGraphicsItem*> collisions = collidingItems();
			m_nodes.clear();
			foreach (QGraphicsItem* item, collisions)
			switch (item->type())
			{
				case NodeItem::RTTI:
				m_nodes.append(dynamic_cast<NodeItem*>(item));
			}
			break;
		}
		default:
			logWarn(QString("TriangleSelection::selectCoveredItems : unsupported type %1")
				.arg(m_selectType));
	}
}

bool TriangleSelection::hasItems() const
{
	return !(m_triangles.isEmpty() && m_nodes.isEmpty());
}

Triangle* TriangleSelection::first()
{
	if (!m_triangles.isEmpty())
		return m_triangles.first();
	else if (!m_nodes.isEmpty())
		return m_nodes.first()->triangle();
	return 0;
}

QList<Triangle*> TriangleSelection::triangles() const
{
	return m_triangles;
}

QList<NodeItem*> TriangleSelection::nodes() const
{
	return m_nodes;
}

void TriangleSelection::clear()
{
	m_triangles.clear();
	m_nodes.clear();
}

int TriangleSelection::selectedType() const
{
	return m_selectType;
}

void TriangleSelection::setSelectedType(int rtti)
{
	switch (rtti)
	{
		case Triangle::RTTI:
		case NodeItem::RTTI:
			m_selectType = rtti;
			break;
		default:
			logWarn(QString("TriangleSelection::setSelectedType : unsupported type %1").arg(rtti));
	}
}

void TriangleSelection::addItem(QGraphicsItem* item)
{
	switch (item->type())
	{
		case NodeItem::RTTI:
			m_nodes.append(dynamic_cast<NodeItem*>(item));
			break;
		case Triangle::RTTI:
		case PostTriangle::RTTI:
			m_triangles.append(dynamic_cast<Triangle*>(item));
			break;
		default:
			logWarn(QString("TriangleSelection::addItem : unsupported type %1")
				.arg(item->type()));
	}
}

bool TriangleSelection::contains(QGraphicsItem* item) const
{
	switch (item->type())
	{
		case NodeItem::RTTI:
			return m_nodes.contains(dynamic_cast<NodeItem*>(item));
		case Triangle::RTTI:
		case PostTriangle::RTTI:
			return m_triangles.contains(dynamic_cast<Triangle*>(item));
		default:
			logWarn(QString("TriangleSelection::contains : unsupported type %1")
			.arg(item->type()));
	}
	return false;
}

// return true if selection contains any part of a Triangle or its PostTriangle
bool TriangleSelection::containsAnyOf(Triangle* t) const
{
	if (contains(t))
		return true;

	if (t == m_editor->getSelectedTriangle() && m_editor->postEnabled())
	{
		Triangle* post = m_editor->post();
		if (contains(post))
			return true;
		foreach (NodeItem* node, post->getNodes())
			if (contains(node))
				return true;
	}

	foreach (NodeItem* node, t->getNodes())
		if (contains(node))
			return true;

	return false;
}

QList<QGraphicsItem*> TriangleSelection::allItems() const
{
	QList<QGraphicsItem*> items;
	foreach (QGraphicsItem* node, m_nodes)
		items.append(node);
	foreach (QGraphicsItem* triangle, m_triangles)
		items.append(triangle);
	return items;
}

BasisTriangle* TriangleSelection::basis() const
{
	return m_basis;
}

void TriangleSelection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QGraphicsPolygonItem::paint(painter, option, widget);
}
