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

#include "transformablegraphicsguide.h"
#include "transformablegraphicsitem.h"
#include "logger.h"


//--------------------------------------------------------------------------

GraphicsGuideScaleButton::GraphicsGuideScaleButton(QGraphicsItem* parent) :
	QGraphicsRectItem(parent)
{
	QPen p(Qt::NoBrush);
	p.setWidth(0);
	setPen(p);
	setAcceptHoverEvents(true);
}

int GraphicsGuideScaleButton::type() const
{
	return Type;
}

void GraphicsGuideScaleButton::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	QPen p(pen());
	std_pen = p;
	p.setColor(Qt::red);
	setPen(p);
}

void GraphicsGuideScaleButton::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	setPen(std_pen);
}


//--------------------------------------------------------------------------

TransformableGraphicsGuide::TransformableGraphicsGuide(FigureEditor* fe, QGraphicsItem* parent) :
	QGraphicsObject(parent),
	editor(fe),
	outerRect(0.0, 0.0, 1.0, 1.0),
	topLeftRect(this),
	topRightRect(this),
	bottomLeftRect(this),
	bottomRightRect(this),
	bottomRect(this),
	topRect(this),
	leftRect(this),
	rightRect(this)
{
	topLeftRect.location     = GraphicsGuideScaleButton::TopLeft;
	topRightRect.location    = GraphicsGuideScaleButton::TopRight;
	bottomLeftRect.location  = GraphicsGuideScaleButton::BottomLeft;
	bottomRightRect.location = GraphicsGuideScaleButton::BottomRight;

	QPen pen(editor->guideColor(), 0);
	topRightRect.setPen(pen);
	topLeftRect.setPen(pen);
	bottomRightRect.setPen(pen);
	bottomLeftRect.setPen(pen);
	setAcceptHoverEvents(true);

	// hide the scale mode guide(s)
	bool scaleMode( editor->mode() == FigureEditor::Scale );
	topRightRect.setVisible(scaleMode);
	topLeftRect.setVisible(scaleMode);
	bottomRightRect.setVisible(scaleMode);
	bottomLeftRect.setVisible(scaleMode);
}

QPainterPath TransformableGraphicsGuide::shape() const
{
	QPainterPath path;
	if (editor->mode() == FigureEditor::Scale)
	{
		path.addRect(topRightRect.rect());
		path.addRect(topLeftRect.rect());
		path.addRect(bottomRightRect.rect());
		path.addRect(bottomLeftRect.rect());
	}
	else
	{
		path.addRect(QRect(0.0, 0.0, 0.0, 0.0));
	}
	return path;
}

QRectF TransformableGraphicsGuide::boundingRect() const
{
	return outerRect;
}

void TransformableGraphicsGuide::setParentItem(QGraphicsItem* parent)
{
	QGraphicsItem* cparent = parentItem();
	if (cparent && (parent != cparent))
		dynamic_cast<TransformableGraphicsItem*>(cparent)->setGraphicsGuide(0);

	QGraphicsItem::setParentItem(parent);
	parent->setAcceptHoverEvents(true);
	update();
}

void TransformableGraphicsGuide::update()
{
	if (isVisible())
	{
		FigureEditor::EditMode mode = editor->mode();

		bool scaleMode( mode == FigureEditor::Scale );
		topRightRect.setVisible(scaleMode);
		topLeftRect.setVisible(scaleMode);
		bottomRightRect.setVisible(scaleMode);
		bottomLeftRect.setVisible(scaleMode);

		if (scaleMode)
		{
			QPointF cen;
			QPolygonF poly;

			if (editor->hasSelection())
			{
				QGraphicsPolygonItem* item = editor->selection();
				if (item == 0)
					return;
				cen  = mapFromScene(editor->selectionTransformPos());
				poly = mapFromScene(item->polygon());
			}
			else
			{
				QGraphicsPolygonItem* item = dynamic_cast<QGraphicsPolygonItem*>(parentItem());
				cen  = editor->triangleTransformPos();
				poly = item->polygon();
			}
			QRectF f( poly.boundingRect() );
			qreal xmax = qMax(qAbs(f.left() - cen.x()), qAbs(f.right()  - cen.x()));
			qreal ymax = qMax(qAbs(f.top()  - cen.y()), qAbs(f.bottom() - cen.y()));
			QPointF pmax(xmax, ymax);
			QRectF r(pmax, -pmax);

			r.moveCenter(cen);
			outerRect = r;
			QRectF l = parentItem()->mapRectFromScene(QRectF(QPointF(0.0, 0.0), QSizeF(10, 10)));
			QPen pen(editor->guideColor(), 0);

			l.moveBottomLeft(r.topRight());
			topRightRect.setPen(pen);
			topRightRect.setRect(l);

			l.moveBottomRight(r.topLeft());
			topLeftRect.setPen(pen);
			topLeftRect.setRect(l);

			l.moveTopLeft(r.bottomRight());
			bottomRightRect.setPen(pen);
			bottomRightRect.setRect(l);

			l.moveTopRight(r.bottomLeft());
			bottomLeftRect.setPen(pen);
			bottomLeftRect.setRect(l);
		}
		else if (mode == FigureEditor::Rotate)
		{
			QGraphicsPolygonItem* item;
			QPointF cen;
			QPolygonF poly;
			if (editor->hasSelection())
			{
				item = editor->selection();
				if (item == 0)
					return;
				cen  = mapFromScene(editor->selectionTransformPos());
				poly = mapFromScene(item->polygon());
			}
			else
			{
				item = dynamic_cast<QGraphicsPolygonItem*>(parentItem());
				poly = item->polygon();
				cen = editor->triangleTransformPos();
			}
			qreal rmax = 0.0;
			foreach (QPointF p, poly)
			{
				QLineF l(p, cen);
				qreal len(l.length());
				if (len > rmax)
					rmax = len;
			}
			qreal height = rmax * 2.0;
			outerRect = QRectF(cen.x() - rmax, cen.y() - rmax, height, height);
		}
		else if (mode == FigureEditor::Flip)
		{
			QPolygonF poly;
			QPointF  cen;
			if (editor->hasSelection())
			{
				QGraphicsPolygonItem* item = editor->selection();
				if (item == 0)
					return;
				cen  = mapFromScene(editor->selectionTransformPos());
				poly = mapFromScene(item->polygon());
			}
			else
			{
				QGraphicsPolygonItem* item = dynamic_cast<QGraphicsPolygonItem*>(parentItem());
				cen  = editor->triangleTransformPos();
				poly = item->polygon();
			}

			QPolygonF pa( poly );
			double tx = cen.x();
			int n = 0;
			foreach (QPointF p, pa)
			{
				p.rx()  = p.x() - 2.0 * (p.x() - tx);
				pa[n++] = p;
			}
			hFlipPoly = pa;
			pa = poly;
			tx = cen.y();
			n = 0;
			foreach (QPointF p, pa)
			{
				p.ry()  = p.y() - 2.0 * (p.y() - tx);
				pa[n++] = p;
			}
			vFlipPoly = pa;
			outerRect = poly.boundingRect().united(hFlipPoly.boundingRect()).united(vFlipPoly.boundingRect());
		}
	}
}

void TransformableGraphicsGuide::setVisible(bool flag)
{
	QGraphicsItem::setVisible(flag);
	update();
}

void TransformableGraphicsGuide::paint(QPainter* p, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
	QPen pen(editor->guideColor(), 0);
	pen.setStyle(Qt::SolidLine);
	p->save();
	p->setPen(pen);
	p->setBrush(QBrush(Qt::transparent, Qt::SolidPattern));

	FigureEditor::EditMode mode = editor->mode();
	if (mode == FigureEditor::Scale)
	{
		p->drawRect(outerRect);
		pen.setStyle(Qt::DotLine);
		p->setPen(pen);
		p->drawLine(topLeftRect.rect().topLeft(),  bottomRightRect.rect().bottomRight());
		p->drawLine(topRightRect.rect().topRight(), bottomLeftRect.rect().bottomLeft());
	}
	else if (mode == FigureEditor::Rotate)
	{
		p->drawEllipse(outerRect);
		p->setBrush(QBrush(pen.color(), Qt::SolidPattern));
		QRectF r(mapRectFromScene(QRectF(-1., -1., 2., 2.)));
		r.moveCenter(outerRect.center());
		p->drawEllipse(r);
	}
	else if (mode == FigureEditor::Flip)
	{
		p->drawPolygon(hFlipPoly);
		p->drawPolygon(vFlipPoly);
	}
	p->restore();
}
