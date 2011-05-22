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
#include <QDockWidget>
#include <QSettings>

#include "mainpreviewwidget.h"
#include "viewerpresetswidget.h"
#include "logger.h"

MainPreviewWidget::MainPreviewWidget(GenomeVector* g, QWidget* p)
	: QWidget(p), QosmicWidget(this, "MainPreviewWidget"), genome(g)
{
	setupUi(this);
	popupMenu = new QMenu(tr("presets"));
	QSize size(QSettings().value("mainpreview/imagesize", QSize(160, 130)).toSize());
	setPreviewMaximumSize(size);

	connect(popupMenu, SIGNAL(triggered(QAction*)),
		this, SLOT(popupMenuTriggeredSlot(QAction*)));
}

void MainPreviewWidget::closeEvent(QCloseEvent* e)
{
	QSettings().setValue("mainpreview/imagesize", m_previewLabel->maximumSize());
	e->accept();
}

void MainPreviewWidget::setPixmap(const QPixmap& p)
{
	m_previewLabel->setPixmap(p);
}

void MainPreviewWidget::resizeEvent(QResizeEvent* e)
{
	logFine(QString("MainPreviewWidget::resizeEvent : spontaneous %1").arg(e->spontaneous()));
	logFine(QString("MainPreviewWidget::resizeEvent : oldSize %1, %2").arg(e->oldSize().width()).arg(e->oldSize().height()));
	logFine(QString("MainPreviewWidget::resizeEvent : size %1, %2").arg(e->size().width()).arg(e->size().height()));
	if (last_size == m_previewLabel->size())
		return;
	last_size = m_previewLabel->size();
	if (parent()->inherits("QDockWidget")
		&& qobject_cast<QDockWidget*>(parent())->isFloating())
		m_previewLabel->setMaximumSize(e->size());
	emit previewResized(last_size);
}

QSize MainPreviewWidget::getPreviewSize()
{
	return m_previewLabel->size();
}

void MainPreviewWidget::showEvent(QShowEvent* e)
{
	logFine(QString("MainPreviewWidget::showEvent : spontaneous %1").arg(e->spontaneous()));
	if (!e->spontaneous())
		emit previewResized(m_previewLabel->size());
}

void MainPreviewWidget::setPreviewMaximumSize(QSize s)
{
	m_previewLabel->setMaximumSize(s);
}


void MainPreviewWidget::popupMenuTriggeredSlot(QAction* a)
{
	ViewerPresetsWidget* w
		= dynamic_cast<ViewerPresetsWidget*>(getWidget("ViewerPresetsWidget"));
	w->selectPreset(a->text());
}

void MainPreviewWidget::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::RightButton)
	{
		popupMenu->clear();
		ViewerPresetsWidget* w
			= dynamic_cast<ViewerPresetsWidget*>(getWidget("ViewerPresetsWidget"));
		// danger!
		QStringList presets = w->presetNames();
		QString current = w->current();
		foreach (QString s, presets)
		{
			QAction* a = popupMenu->addAction(s);
			if (current == s)
			{
				a->setCheckable(true);
				a->setChecked(true);
				popupMenu->setActiveAction(a);
			}
		}
		popupMenu->popup(e->globalPos());
	}
	else
	{
		last_pos = start_pos = e->pos();
		wheel_moved = false;
	}
}

void MainPreviewWidget::mouseReleaseEvent(QMouseEvent* e)
{
	if ((e->button() == Qt::LeftButton)
		&& (start_pos != e->pos() || wheel_moved))
			emit undoStateSignal();
}

void MainPreviewWidget::mouseMoveEvent(QMouseEvent* e)
{
	if (m_previewLabel->underMouse())
	{
		double dx = last_pos.x() - e->pos().x();
		double dy = last_pos.y() - e->pos().y();
		last_pos = e->pos();
		flam3_genome* g = genome->selectedGenome();

		if (e->modifiers() & Qt::ControlModifier)
			g->pixels_per_unit += -1*dx + dy;
		else
		{
			dx *= ((double)g->width / m_previewLabel->size().width()
										/ qMax(1.0,g->pixels_per_unit));
			dy *= ((double)g->height / m_previewLabel->size().height()
										/ qMax(1.0,g->pixels_per_unit));
			g->center[0] += dx ;
			g->center[1] += dy ;
		}
		emit previewMoved();
	}
}

void MainPreviewWidget::wheelEvent(QWheelEvent* e)
{
	if (m_previewLabel->underMouse())
	{
		int n = 10;
		if (QApplication::keyboardModifiers() & Qt::ControlModifier) n = 100;
		else if (QApplication::keyboardModifiers() & Qt::ShiftModifier) n = 1;
		if (e->delta() < 0) n *= -1;
		genome->selectedGenome()->pixels_per_unit += n;
		wheel_moved = true;
		emit previewMoved();
	}
}
