/***************************************************************************
 *   Copyright (C) 2009 by David Bitseff                                   *
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
#include <QPainter>
#include <QMouseEvent>
#include <QStyleOptionFrame>

#include "gradientstopseditor.h"
#include "colordialog.h"
#include "logger.h"

GradientStopsEditor::GradientStopsEditor(QWidget* parent)
: QWidget(parent)
{
	moving_idx   = -1;
	selected_idx << 0;
	arrow_type = Qt::UpArrow;
	menu_enabled = true;

	popupMenu = new QMenu(tr("Edit Stops"));
	addAction = new QAction(tr("Add Stop"),this);
	delAction = new QAction(tr("Remove Stop"),this);
	colorAction = new QAction(tr("Change Color"),this);
	popupMenu->addAction(addAction);
	popupMenu->addAction(delAction);
	popupMenu->addAction(colorAction);

	connect(addAction, SIGNAL(triggered()), this, SLOT(addStop()));
	connect(delAction, SIGNAL(triggered()), this, SLOT(removeStop()));
	connect(colorAction, SIGNAL(triggered()), this, SLOT(selectStopColor()));

	resetStops();
}


void GradientStopsEditor::selectStopColor()
{
	if (selected_idx.size() < 1)
		return;
	ColorDialog d(this);
	QColor lastcolor( stops.at(selected_idx.first()).second );
	d.setSelectedColor(lastcolor);
	d.move(moving_global);
	connect(&d, SIGNAL(colorSelected(QColor)), this, SLOT(stopColorSelected(QColor)));
	if (d.exec() == QDialog::Accepted)
	{
		QColor c( d.getSelectedColor() );
		if (c.isValid() && stops[selected_idx.first()].second != c)
			stopColorSelected(c);
	}
	else
		if (stops[selected_idx.first()].second != lastcolor)
			stopColorSelected(lastcolor);
	disconnect(&d);
}

void GradientStopsEditor::stopColorSelected(QColor c)
{
	QGradientStop* data = stops.data();
	data[selected_idx.first()].second = c;
	emit stopsChanged();
	update();
}

void GradientStopsEditor::addStop()
{
	int h( size().height() );
	int w( size().width() );
	qreal pos( (qreal)moving_start.x() / w );

	QImage palette(w, h, QImage::Format_RGB32);
	QPainter painter(&palette);
	QLinearGradient grad(0, h, w, h);
	grad.setStops(stops);
	painter.fillRect(rect(), QBrush(grad));
	QColor color( palette.pixel(moving_start.x(), 1) );
	stops << QGradientStop(pos, color);
	selected_idx.clear();
	selected_idx << stops.size() - 1;
	update();
	emit stopsChanged();
	logFine(QString("GradientStopsEditor::addStop : found stop %1 at %2 mouse %3")
		.arg(stops.last().first).arg(pos).arg(moving_start.x()));
}

void GradientStopsEditor::removeStop()
{
	if (stops.size() < 3)
		return;

	if (selected_idx.size() > 0)
	{
		logFine(QString("GradientStopsEditor::removeStop : removing stop %1")
			.arg(selected_idx.first()));
		stops.remove(selected_idx.first());
		selected_idx.pop_front();
		update();
		emit stopsChanged();
	}
}

void GradientStopsEditor::mousePressEvent(QMouseEvent* event)
{
	switch (event->button())
	{
		case Qt::RightButton:
		{
			if (menu_enabled)
			{
				moving_start = event->pos();
				moving_global = event->globalPos();
				popupMenu->popup(moving_global);
			}
			break;
		}
		case Qt::LeftButton:
		{
			qreal pos( (qreal)event->x() / size().width() );
			foreach (QGradientStop stop, stops)
			{
				if (qAbs( pos - stop.first ) < 0.01)
				{
					logFine(QString("GradientStopsEditor::mousePressEvent : found stop %1 at %2").arg(stop.first).arg(pos));
					moving_start = event->pos();
					moving_idx = stops.indexOf(stop);
					if (! (event->modifiers() & Qt::ShiftModifier) )
						selected_idx.clear();
					if (! selected_idx.contains(moving_idx))
						selected_idx << moving_idx;
					update();
					break;
				}
			}
			break;
		}
		default:
			;
	}
}

void GradientStopsEditor::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		if (moving_idx != -1)
		{
			QGradientStop* data = stops.data();
			if (selected_idx.size() > 1)
			{
				bool changed(false);
				foreach (int n, selected_idx)
				{
					qreal cur( data[n].first );
					qreal dx( (qreal)(event->x() - moving_start.x()) / size().width() );
					qreal val( qBound(0.0, data[n].first + dx, 1.0) );
					if (val != cur)
					{
						data[n].first = val;
						changed = true;
					}
				}
				moving_start = event->pos();
				if (changed)
				{
					emit stopsChanged();
					update();
				}
			}
			else
			{
				qreal cur( data[moving_idx].first );
				qreal val( qBound(0.0, (qreal)event->x() / size().width(), 1.0) );
				if (val != cur)
				{
					data[moving_idx].first = val;
					emit stopsChanged();
					update();
				}
			}
		}
	}
}

void GradientStopsEditor::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		moving_idx = -1;
}

void GradientStopsEditor::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	int w( size().width() );
	int h( size().height() );

	QStyleOptionFrame option;
	option.initFrom(this);
	option.lineWidth = 0;
	option.midLineWidth = 0;
	style()->drawPrimitive(QStyle::PE_Frame, &option, &painter, this);

	for (int n = 0 ; n < stops.size() ; n++)
	{
		QGradientStop stop( stops.at(n) );
		painter.setBrush(QBrush(stop.second));
		qreal x( qBound(0.0, stop.first * w, w - 1.0) );
		if (selected_idx.contains(n))
		{
			QColor c( QColor::fromRgb(
				255 - stop.second.red(),
				255 - stop.second.green(),
				255 - stop.second.blue()) );
			if (stop.second.value() < 128)
				painter.setPen(c.lighter());
			else
				painter.setPen(c.darker());
		}
		else
			painter.setPen(stop.second);

		switch (arrow_type)
		{
			case Qt::DownArrow:
			{
				const QPoint points[3] = {
					QPoint(x, h),
					QPoint(x + 4, 0),
					QPoint(x - 4, 0),
				};
				painter.drawPolygon(points, 3);
				break;
			}
			default:
			{
				const QPoint points[3] = {
					QPoint(x, 0),
					QPoint(x + 4, h),
					QPoint(x - 4, h),
				};
				painter.drawPolygon(points, 3);
			}
		}

		if (menu_enabled)
		{
			bool selected( selected_idx.size() > 0 );
			delAction->setEnabled(selected);
			colorAction->setEnabled(selected);
		}
	}
}

void GradientStopsEditor::setStops(const QGradientStops& s)
{
	if (s.size() < 2)
	{
		logError(QString("GradientStopsEditor::setStops : number of gradient stops < 2"));
		return;
	}
	stops = s;
	emit stopsChanged();
	update();
}

QGradientStops& GradientStopsEditor::getStops()
{
	return stops;
}

void GradientStopsEditor::setArrowDirection(Qt::ArrowType type)
{
	arrow_type = type;
	update();
}

Qt::ArrowType GradientStopsEditor::arrowDirection() const
{
	return arrow_type;
}

void GradientStopsEditor::setMenuEnabled(bool flag)
{
	menu_enabled = flag;
}

bool GradientStopsEditor::menuEnabled() const
{
	return menu_enabled;
}

void GradientStopsEditor::resetStops()
{
	selected_idx.erase(selected_idx.begin()++, selected_idx.end());
	stops.clear();
	stops << QGradientStop(0.0, Qt::black) << QGradientStop(1.0, Qt::white);
	update();
}
