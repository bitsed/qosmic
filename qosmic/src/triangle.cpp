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
#include <cmath>

#include "triangle.h"
#include "logger.h"
#include "nodeitem.h"
#include "xfedit.h"
#include "transformablegraphicsguide.h"

uint Triangle::zpos = 0;

Triangle::Triangle( FigureEditor* c, flam3_xform* x, BasisTriangle* b, int idx)
: canvas(c), m_xform(x), basisTriangle(b), m_index(idx), cList(), nList(),
m_edgeLine(0.,0.,0.,0.), m_edgeType(NoEdge),
m_idxLabel(QString::number(idx + 1), this)
{
	logFinest("Triangle::Triangle : enter");
	setTransform(QTransform(basisTriangle->coordTransform()));

	QFont f(m_idxLabel.font());
	f.setBold(true);
	m_idxLabel.setFont(f);
	m_idxLabel.setFlag(QGraphicsItem::ItemStacksBehindParent);

	NodeItem *el;
	el = new NodeItem( canvas, this, NODE_O, "O" );
	addNode(el);
	el = new NodeItem( canvas, this, NODE_X, "X" );
	addNode(el);
	el = new NodeItem( canvas, this, NODE_Y, "Y" );
	addNode(el);
	TriangleCoords tc = basisTriangle->getCoords(m_xform->c);
	setPoints(tc);
	moveToFront();
	setBrush(Qt::NoBrush);
	setPen(QPen(Qt::gray));
	setAcceptHoverEvents(true);
}

void Triangle::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
	hoverMoveEvent(e);
}

void Triangle::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
	if (canvas->mode() == FigureEditor::Move)
	{
		bool postEnabled( canvas->postEnabled() );
		if ((!postEnabled && canvas->getSelectedTriangle() == this)  ||
			( postEnabled && canvas->post() == this) )
			findEdge(e->pos());
	}
}

void Triangle::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*e*/)
{
	clearFoundEdge();
}

Triangle::~Triangle()
{
	NodeItem *node;
	TriangleNodesIterator it( nList );
	while ( it.hasNext() )
	{
		node = it.next();
		canvas->removeItem(node);
		delete node;
	}
	hide();
}

void Triangle::resetPosition()
{
	m_xform->c[0][0] = 1.0;
	m_xform->c[0][1] = 0.0;
	m_xform->c[1][0] = 0.0;
	m_xform->c[1][1] = 1.0;
	m_xform->c[2][0] = 0.0;
	m_xform->c[2][1] = 0.0;

	TriangleCoords tc = basisTriangle->getCoords(m_xform->c);
	setPoints(tc);
}

void Triangle::addNode( NodeItem *node )
{
	node->setTriangle(this);
	node->setParentItem(this);
	canvas->addItem(node);
	nList.append( node );
}

TriangleNodes& Triangle::getNodes()
{
	return nList;
}

TriangleCoords Triangle::getCoords()
{
	return cList;
}

int Triangle::type() const
{
	return RTTI;
}

flam3_xform* Triangle::xform()
{
	return m_xform;
}

void Triangle::basisScaledSlot()
{
	setTransform(QTransform(basisTriangle->coordTransform()));
	TriangleCoords tc = basisTriangle->getCoords(m_xform->c);
	setPoints(tc);

	// only update the brush matrix for selected triangles
	if (this == canvas->getSelectedTriangle())
	{
		QBrush b(brush());
		b.setMatrix(basisTriangle->coordTransform());
		setBrush(b);
	}
	if (m_guide)
		m_guide->update();
}

void Triangle::setPoints(TriangleCoords& points)
{
	logFinest("Triangle::setPoints : enter");
	cList = points;
	if (points.size() != 3)
		return;
	int n = 0;
	NodeItem *node;
	TriangleNodesIterator it( nList );

	while ( it.hasNext() )
	{
		node = it.next();
		QPointF npos = mapToItem(node, cList[n]);
		node->movePoint(npos.x(), npos.y());
		n++;
	}
	QGraphicsPolygonItem::setPolygon(cList);
	adjustSceneRect();
}

void Triangle::moveToFront()
{
	zpos++;
	setZValue(zpos);
	zpos++;
	TriangleNodesIterator it( nList );
	while ( it.hasNext() )
		it.next()->setZValue( zpos );
}

int Triangle::nextZPos()
{
	return ++zpos;
}

void Triangle::show()
{
	QGraphicsPolygonItem::show();
	TriangleNodesIterator it( nList );
	while ( it.hasNext() )
		it.next()->show();
}

void Triangle::setVisible(bool flag)
{
	QGraphicsPolygonItem::setVisible(flag);
	TriangleNodesIterator it( nList );
	while ( it.hasNext() )
		it.next()->setVisible(flag);
}

int Triangle::index() const
{
	return m_index;
}

QRectF Triangle::boundingRect()
{
	return QGraphicsPolygonItem::boundingRect().adjusted(-.4,-.4,.8,.8);
}

/**
 *  This one is called by the currently moving NodeItem
 */
void Triangle::moveEdges()
{
	TriangleNodesIterator it( nList );
	NodeItem *node;
	cList.clear();
	while ( it.hasNext() )
	{
		node = it.next();
		cList <<  mapFromScene(node->pos());
	}
	QGraphicsPolygonItem::setPolygon( cList );
	adjustSceneRect();
	coordsToXForm();
}

void Triangle::scale(double dx, double dy, QPointF cpos)
{
	double tx(cpos.x());
	double ty(cpos.y());
	QTransform trans;
	// scale
	trans.translate(tx, ty);
	trans.scale(dx, dy);
	trans.translate(-tx, -ty);
	QPolygonF pa( mapToScene(trans.map(polygon())) );
	TriangleNodesIterator it( nList );
	int n(0);
	// rebuild triangle + nodes
	while ( it.hasNext() )
		it.next()->setPos( pa.at(n++) );
	moveEdges();
}

void Triangle::rotate(double rad, QPointF cpos)
{
	double tx(cpos.x());
	double ty(cpos.y());
	QTransform trans;
	// rotate
	trans.translate(tx, ty);
	trans.rotate(rad);
	trans.translate(-tx, -ty);
	QPolygonF pa( mapToScene(trans.map(polygon())) );
	TriangleNodesIterator it( nList );
	int n(0);
	// rebuild triangle + nodes
	while ( it.hasNext() )
		it.next()->setPos( pa.at(n++) );
	moveEdges();
}

void Triangle::rotateNode(NodeItem* nodeItem, double rad, QPointF cpos)
{
	double tx(cpos.x());
	double ty(cpos.y());
	QTransform trans;
	trans.translate(tx, ty);
	trans.rotate(rad);
	trans.translate(-tx, -ty);
	QPolygonF pa( mapToScene(trans.map(polygon())) );
	TriangleNodesIterator it( nList );
	NodeItem *node;
	int n(0);
	// rebuild triangle + nodes
	while ( it.hasNext() )
	{
		node = it.next();
		if (node == nodeItem)
			node->setPos( pa.at(n) );
		n++;
	}
	moveEdges();
}

void Triangle::flipHorizontally()
{
	flipHorizontally(polygon().boundingRect().center());
}

void Triangle::flipVertically()
{
	flipVertically(polygon().boundingRect().center());
}

void Triangle::flipHorizontally(QPointF center)
{
	double tx = center.x();
	QPolygonF pa = polygon();
	int n = 0;
	foreach (QPointF p, pa)
	{
		p.rx()  =  p.x() - 2.*(p.x() - tx) ;
		pa[n++] = p;
	}
	TriangleNodesIterator it( nList );
	NodeItem *node;
	n = 0;
	cList.clear();
	while ( it.hasNext() )
	{
		node = it.next();
		QPointF p = mapToScene(pa[n++]);
		node->setPos( p );
		cList << mapFromScene(node->pos());
	}
	QGraphicsPolygonItem::setPolygon(cList);
	adjustSceneRect();
	coordsToXForm();
}

void Triangle::flipVertically(QPointF center)
{
	double ty = center.y();
	QPolygonF pa = polygon();
	int n = 0;
	foreach (QPointF p, pa)
	{
		p.ry()  =  p.y() - 2.*(p.y() - ty) ;
		pa[n++] = p;
	}
	TriangleNodesIterator it( nList );
	NodeItem *node;
	n = 0;
	cList.clear();
	while ( it.hasNext() )
	{
		node = it.next();
		QPointF p = mapToScene(pa[n++]);
		node->setPos( p );
		cList << mapFromScene(node->pos());
	}
	QGraphicsPolygonItem::setPolygon(cList);
	adjustSceneRect();
	coordsToXForm();
}

/**
 *  This one is called by the FigureEditor
 */
void Triangle::moveBy(double dx, double dy)
{
	TriangleNodesIterator it( nList );
	NodeItem *node;
	cList.clear();
	while ( it.hasNext() )
	{
		node = it.next();
		node->movePoint( dx, dy );
		cList << mapFromScene(node->pos());
	}
	QGraphicsPolygonItem::setPolygon(cList);
	adjustSceneRect();
	coordsToXForm();
}

BasisTriangle* Triangle::basis() const
{
	return basisTriangle;
}

const QMatrix& Triangle::getCoordinateTransform()
{
	return basisTriangle->coordTransform();
}

void Triangle::coordsToXForm()
{
	logFinest("Triangle::coordsToXForm : enter");
	basisTriangle->applyTransform(cList, m_xform->c);
}

void Triangle::setNodeColor(const QColor& c, const QColor& s)
{
	TriangleNodesIterator it( nList );
	while ( it.hasNext() )
		it.next()->setPen(QPen(c), QPen(s));
	m_idxLabel.setPen(Qt::NoPen);
	m_idxLabel.setBrush(QBrush(s));
}

FigureEditor* Triangle::editor() const
{
	return canvas;
}

void Triangle::adjustSceneRect()
{
	if (scene())
	{
		QRectF r = mapToScene(boundingRect()).boundingRect();
		QRectF scene_r = scene()->sceneRect();
		if (!scene_r.contains(r))
			scene()->setSceneRect(scene_r.united(r));

		if (m_guide)
			m_guide->update();

		if (m_idxLabel.isVisible())
		{
			QFontMetrics fm(m_idxLabel.font());
			QSizeF fsz(mapFromScene(QRectF(fm.boundingRect(m_idxLabel.text()))).boundingRect().size());
			qreal fw = fsz.width() / 2.0;
			qreal fh = fsz.height() / 2.0;
			QPointF a(polygon().at(0));
			QPointF b(polygon().at(1));
			QPointF c(polygon().at(2));

			// find the intersection of two right triangles
			qreal b_ax = (b.x() - a.x());
			qreal c_ax = (c.x() - a.x());
			qreal dx = mapFromScene(QPointF(0.0001, 0.0)).x();
			if (qAbs(b_ax) < dx) b_ax = dx;
			if (qAbs(c_ax) < dx) c_ax = dx;
			qreal mab = (b.y() - a.y())/b_ax;
			qreal mac = (c.y() - a.y())/c_ax;
			qreal abx = a.x() + b_ax / 2;
			qreal aby = mab*( abx - a.x()) + a.y();
			QPointF d(abx, aby);
			qreal acx = a.x() + c_ax / 2;
			qreal acy = mac* (acx - a.x()) + a.y();
			QPointF e(acx, acy);
			QLineF dc(d, c);
			QLineF eb(e, b);
			QPointF k;
			QLineF::IntersectType t = dc.intersect(eb, &k);
			if (t != QLineF::BoundedIntersection)
				logWarn(QString("Triangle::adjustSceneRect : IntersectType %1").arg(t));
			logFine(QString("Triangle::adjustSceneRect : k = (%1, %2)").arg(k.x()).arg(k.y()));
			m_idxLabel.setTransform(transform().inverted());
			m_idxLabel.setPos(k + QPointF(-fw, fh));
		}
	}
}

NodeItem* Triangle::getNode(int idx) const
{
	return nList[idx];
}

void Triangle::setXform(flam3_xform* xform)
{
	m_xform = xform;
}

// Clear found edges
void Triangle::clearFoundEdge()
{
	m_edgeType = NoEdge;
	m_edgeLine.setLength(0);
	update();
}

// Find the edge closest to the given point.
void Triangle::findEdge(const QPointF& pos)
{
	QPolygonF poly(polygon());
	QList<QLineF> lines;
	lines << QLineF(poly[0], poly[1])
		<< QLineF(poly[0], poly[2])
		<< QLineF(poly[1], poly[2]);
	QPointF scenePos( mapToScene( pos ) );
	int n(0);
	int lastLength = m_edgeLine.length();
	int lastType = m_edgeType;
	m_edgeType = NoEdge;
	m_edgeLine.setLength(0);
	foreach (QLineF line, lines)
	{
		// determine the minimum distance between pos and each of the edges
		QPointF n0( line.p1() );
		QPointF n1( line.p2() );

		qreal denom(n1.x() - n0.x());
		qreal x, y;
		if (denom == 0.0) // the edge is a vertical line
		{
			x = n0.x();
			y = pos.y();
		}
		else
		{
			qreal m( ( n1.y() - n0.y() ) / denom );
			qreal b( n0.y() -  m * n0.x() );
			x = ( pos.x() + m * (pos.y() - b) ) / (m*m + 1 );
			y = m * x + b ;
		}

		QPointF scenePoint( mapToScene( x, y ) );
		qreal dx( scenePoint.x() - scenePos.x() );
		qreal dy( scenePoint.y() - scenePos.y() );
		qreal d( sqrt( dx*dx + dy*dy ) );
		// choose the edge that is <5 pixels from pos
		if (d < 5.0)
		{
			logFiner(QString("Triangle::findEdge : found triangle %1 edge at (%2,%3)")
				.arg(m_index).arg(x).arg(y));
			m_edgeLine.setPoints(n0, n1);
			if (n < 2)
				m_edgeType = RotateEdge;
			else
				m_edgeType = ScaleEdge;
			break;
		}
		n++;
	}
	if (lastLength != m_edgeLine.length() || lastType != m_edgeType)
		update();
}

QPointF Triangle::circumCenter()
{
	QPolygonF pa( polygon() );
	double Ax, Ay, Bx, By, Cx, Cy;
	Ax = pa[0].x(); Ay = pa[0].y();
	Bx = pa[1].x(); By = pa[1].y();
	Cx = pa[2].x(); Cy = pa[2].y();
	double D = 2.0*(Ax*(By-Cy)+Bx*(Cy-Ay)+Cx*(Ay-By));
	return QPointF( ((Ay*Ay + Ax*Ax)*(By-Cy) + (By*By + Bx*Bx)*(Cy-Ay) + (Cy*Cy + Cx*Cx)*(Ay-By)) / D,
					((Ay*Ay + Ax*Ax)*(Cx-Bx) + (By*By + Bx*Bx)*(Ax-Cx) + (Cy*Cy + Cx*Cx)*(Bx-Ax)) / D );
}


void Triangle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QGraphicsPolygonItem::paint(painter, option, widget);
	if (!canvas->hasSelection()
		&& (canvas->getSelectedTriangle() == this || type() == PostTriangle::RTTI))
	{
		if (!m_edgeLine.isNull())
		{
			// highlight the edge closest to the mouse position found in findEdge()
			QRectF r( mapRectFromScene(0., 0., 2., 2.) );
			qreal width( sqrt( r.width()*r.width() + r.height()*r.height()) );
			logFiner(QString("Triangle::paint : painting selected edge width %1").arg(width));
			painter->save();
			QPen pen(painter->pen());
			pen.setWidthF(width);
			pen.setCapStyle(Qt::RoundCap);
			painter->setPen(pen);
			painter->drawLine(m_edgeLine);
			painter->restore();
			m_edgeLine.setLine(0.,0.,0.,0.);
		}
	}
}

Triangle::EdgeType Triangle::getEdgeType()
{
	return m_edgeType;
}

bool Triangle::labelVisible() const
{
	return m_idxLabel.isVisible();
}

void Triangle::setLabelVisible(bool visible)
{
	m_idxLabel.setVisible(visible);
	if (visible)
		adjustSceneRect();
}


