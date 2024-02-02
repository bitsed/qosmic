/***************************************************************************
 *   Copyright (C) 2007-2024 by David Bitseff                              *
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

#include <QDockWidget>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include <QMimeData>
#include <QGraphicsSceneMouseEvent>

#include "mainviewer.h"
#include "viewerpresetsmodel.h"
#include "mainwindow.h"

//
// This has gotten wierd and complicated.  The point of the timer
// is to have the viewerResized() signal emitted after the mouse button
// is resleased.  This is used only when the parent widget is a dock
// widget.
//
// The MainViewer is used as a dock widget, and as a window.  The 'F5' and
// 'F6' keys scale the image.  The 'F8' key shows the image in original size.
// The 'F7' key resizes to the previous 'F8' size.  The 'Esc' key closes the
// viewer.
//
MainViewer::MainViewer(QWidget* parent, const QString& title)
: QWidget(parent), QosmicWidget(this, title)
{
	setupUi(this);

	m_request.setName(tr("viewer"));
	m_request.setType(RenderRequest::Queued);

	fullscreen_action = new QAction(tr("fullscreen"), this);
	fullscreen_action->setCheckable(true);
	fullscreen_action->setChecked(false);

	if (isDockWidget())
	{
		// popupMenu for the dockable viewer has image preset/quality options
		QSettings settings;
		settings.beginGroup("mainviewer");
		selected_preset = settings.value("preset", ViewerPresetsModel::getInstance()->presetNames().first()).toString();
		show_status = settings.value("showstatus", false).toBool();
		settings.endGroup();
		popupMenu = new QMenu(tr("presets"));
		connect(popupMenu, SIGNAL(triggered(QAction*)),
				this, SLOT(popupMenuTriggeredSlot(QAction*)));

		fullscreen_action->setShortcut(Qt::Key_Escape);

		status_action = new QAction(tr("show status"), this);
		status_action->setCheckable(true);
		status_action->setChecked(show_status);
		connect(status_action, SIGNAL(triggered(bool)), this, SLOT(statusAction(bool)));

		nullPresetText = tr("genome quality");
	}
	else
	{
		// popupMenu for standalone viewer has scaling, save, and close options
		popupMenu = new QMenu(tr("file"));
		QAction* a = popupMenu->addAction(tr("close"));
		a->setShortcut( Qt::Key_Escape );
		addAction(a);
		connect(a, SIGNAL(triggered(bool)), this, SLOT(closeWindowAction()));

		a = popupMenu->addAction(tr("scale down"));
		a->setShortcut( Qt::Key_F5 );
		addAction(a);
		connect(a, SIGNAL(triggered(bool)), this, SLOT(scaleDownAction()));

		a = popupMenu->addAction(tr("scale up"));
		a->setShortcut( Qt::Key_F6 );
		addAction(a);
		connect(a, SIGNAL(triggered(bool)), this, SLOT(scaleUpAction()));

		a = popupMenu->addAction(tr("scale last"));
		a->setShortcut( Qt::Key_F7 );
		addAction(a);
		connect(a, SIGNAL(triggered(bool)), this, SLOT(scaleLastAction()));

		a = popupMenu->addAction(tr("scale reset"));
		a->setShortcut( Qt::Key_F8 );
		addAction(a);
		connect(a, SIGNAL(triggered(bool)), this, SLOT(scaleResetAction()));

		a = popupMenu->addAction(tr("save image"));
		a->setShortcut( Qt::Key_F9 );
		addAction(a);
		connect(a, SIGNAL(triggered(bool)), this, SLOT(saveImageAction()));

		fullscreen_action->setShortcut( Qt::Key_F10 );
		popupMenu->addAction(fullscreen_action);
	}
	addAction(fullscreen_action);
	connect(fullscreen_action, SIGNAL(triggered(bool)), this, SLOT(fullScreenAction(bool)));

	QPixmap p(1,1);
	p.fill(Qt::black);
	m_pitem = m_scene.addPixmap(p);
	m_pitem->setAcceptDrops(true);
	m_graphicsView->setScene(&m_scene);
	m_scene.installEventFilter(this);
	m_scene.setBackgroundBrush( Qt::black );

	m_titem = new QGraphicsTextItem("");
	m_titem->setDefaultTextColor(QColor(Qt::white));

	m_ritem = new QGraphicsRectItem();
	m_ritem->setBrush(QBrush(Qt::black, Qt::SolidPattern));
	m_ritem->setPen(Qt::NoPen);
	m_ritem->setOpacity(0.2);
	if (isDockWidget())
	{
		m_scene.addItem(m_ritem);
		m_scene.addItem(m_titem);
		m_ritem->setZValue(m_pitem->zValue() + 2);
		m_titem->setZValue(m_pitem->zValue() + 3);
	}
	m_resizedself = false;
	timer = new QTimer(this);
	timer->setInterval(1000);
	timer->setSingleShot(true);
	connect(timer, SIGNAL(timeout()), this, SLOT(checkResized()));
	connect(RenderThread::getInstance(), SIGNAL(flameRendered(RenderEvent*)), this, SLOT(requestRenderedAction(RenderEvent*)));
}

QSize MainViewer::getViewerSize()
{
	return m_graphicsView->maximumViewportSize();
}

// if parent() is a dockwidget, then emit viewerResized() using the timer,
// otherwise emit a signal if user changes the widget size
void MainViewer::resizeEvent(QResizeEvent* /*e*/)
{
	static QSize lastSize(0,0);
	if (m_resizedself)
		m_resizedself = false;
	else if (lastSize != getViewerSize())
	{
		m_scaled_size = last_M = lastSize = getViewerSize();
		if (isDockWidget() && QApplication::mouseButtons() != Qt::NoButton)
			timer->start();
		else
			emit viewerResized( getViewerSize() );
	}
}

void MainViewer::closeWindowAction()
{
		if (isDockWidget())
			qobject_cast<QDockWidget*>(parent())->close();
		else
			close();
}

void MainViewer::scaleDownAction()
{
	m_scaled_size *= 0.9;
	m_pitem->setPixmap( m_pix.scaled(m_scaled_size,
									Qt::KeepAspectRatio,
									Qt::SmoothTransformation) );
	rescaleViewer();
}

void MainViewer::scaleUpAction()
{
	m_scaled_size *= 1.1;
	m_pitem->setPixmap( m_pix.scaled(m_scaled_size,
									Qt::KeepAspectRatio,
									Qt::SmoothTransformation) );
	rescaleViewer();
}

void MainViewer::scaleLastAction()
{
	if (m_pitem->pixmap().size() == m_pix.size())
	{
		m_pitem->setPixmap( m_pix.scaled(last_M,
									Qt::KeepAspectRatio,
									Qt::SmoothTransformation) );
	}
	else
	{
		last_M = m_pitem->pixmap().size();
		m_pitem->setPixmap( m_pix );
	}
	rescaleViewer();
}

void MainViewer::scaleResetAction()
{
	if (m_pitem->pixmap().size() != m_pix.size())
	{
		m_scaled_size = last_M = m_pitem->pixmap().size();
		m_pitem->setPixmap(m_pix);
		rescaleViewer();
	}
}

void MainViewer::saveImageAction()
{
	QString fileName = QFileDialog::getSaveFileName(this,
			tr("Save an image"), tr("untitled.png"));

	if (!fileName.isEmpty()
			   && QFileInfo(QFileInfo(fileName).path()).isWritable())
		m_pitem->pixmap().save(fileName, "png");
}

void MainViewer::setPixmap(QPixmap& p, bool resized)
{
	setPixmap(QPixmap(p), resized);
}

// the resized boolean tells the viewer to rescale either itself
// or the pixmap.  scale the viewer by default
void MainViewer::setPixmap(const QPixmap& p, bool resized)
{
	if (RenderThread::getInstance()->format() != RenderThread::RGB32)
	{
		QImage img(p.size(), QImage::Format_RGB32);
		QPainter pa(&img);
		pa.fillRect(img.rect(), CheckersBrush(16));
		pa.drawPixmap(0, 0, p);
		m_pix = QPixmap::fromImage(img);
	}
	else
		m_pix = p;

	m_orig_size = m_pix.size();
	m_scaled_size = getViewerSize();
	last_M = m_scaled_size;

	if (!(p.isNull() || getViewerSize() == p.size()))
	{
		if (resized)
		{
			m_pitem->setPixmap( m_pix );
			rescaleViewer();
		}
		else
			rescalePixmap();
	}
	else
	{
		m_titem->setVisible(false);
		m_ritem->setVisible(false);
		m_pitem->setPixmap( m_pix );
		QSize size( m_pix.size() );
		m_graphicsView->setSceneRect(0, 0, size.width(), size.height());
	}
}

void MainViewer::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
	{
		if (m_pix.isNull())
		{
			QPixmap p(getViewerSize());
			p.fill(Qt::black);
			setPixmap(p);
		}
		emit viewerResized( getViewerSize() );
	}
}

QPixmap MainViewer::pixmap()
{
	return m_pix;
}

// used by dockwidget children, look for no buttons, otherwise check later
void MainViewer::checkResized()
{
	if(QApplication::mouseButtons() == Qt::NoButton)
		emit viewerResized( getViewerSize() );
	else
		timer->start();
}

MainViewer::~MainViewer()
{
	disconnect(popupMenu);
	disconnect(timer);
	if (isDockWidget())
	{
		m_scene.removeItem(m_titem);
		if (popupMenu->actions().indexOf(status_action) != -1)
			popupMenu->removeAction(status_action);
		delete status_action;
	}
	delete m_ritem;
	delete m_titem;
	delete popupMenu;
	delete timer;
}

// rescale the pixmap to fit the viewer
void MainViewer::rescalePixmap()
{
	m_pitem->setPixmap( m_pix.scaled(getViewerSize(),
									Qt::KeepAspectRatio,
									Qt::SmoothTransformation) );
	QSize size = m_pitem->pixmap().size();
	m_graphicsView->setSceneRect(0, 0, size.width(), size.height());
}

// rescale the viewer to fit the pixmap
void MainViewer::rescaleViewer()
{
	m_resizedself = true;
	QSize px_size = m_pitem->pixmap().size();
	QSize v_size  = m_graphicsView->maximumViewportSize();
	QSize ds = px_size - v_size;
	const QSize fs = frameSize() + ds;

	if (isDockWidget())
		qobject_cast<QDockWidget*>(parent())->resize(fs);
	else
		resize(fs);

	m_graphicsView->setSceneRect(0, 0, px_size.width(), px_size.height());
}

void MainViewer::hideEvent(QHideEvent* e)
{
	if (!e->spontaneous())
		emit viewerHidden();
}

// build a popup menu containing the current set of presets with the last
// selected preset checked
void MainViewer::buildPopupMenu()
{
	// only build this menu for docked widgets
	if (isDockWidget())
	{
		popupMenu->clear();
		preset_actions.clear();

		QStringList presets = ViewerPresetsModel::getInstance()->presetNames();
		foreach (QString s, presets)
		{
			QAction* a = popupMenu->addAction(s);
			preset_actions.append(a);
			if (selected_preset == s)
			{
				a->setCheckable(true);
				a->setChecked(true);
				popupMenu->setActiveAction(a);
			}
		}
		popupMenu->addSeparator();

		// Add the action to select the same image rendering settings
		// as used by the mainpreviewwidget.
		QAction* a = popupMenu->addAction(nullPresetText);
		preset_actions.append(a);
		if (!isPresetSelected())
		{
			a->setCheckable(true);
			a->setChecked(true);
			popupMenu->setActiveAction(a);
		}

		popupMenu->addAction(status_action);
		popupMenu->addAction(fullscreen_action);
		popupMenu->addSeparator();
		a = popupMenu->addAction(tr("save image"));
		a->setCheckable(false);
		connect(a, SIGNAL(triggered(bool)), this, SLOT(saveImageAction()));
	}
}

// This handles displaying the popup menu when MainViewer is installed
// as an eventFilter for the QGraphicsScene
bool MainViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::GraphicsSceneMousePress)
	{
		QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
		if (mouseEvent->button() == Qt::RightButton)
		{
			buildPopupMenu();
			popupMenu->popup(mouseEvent->screenPos());
		}
		return false;
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}

// Called when an item in the popup is triggered.  This method only
// handles preset actions.
void MainViewer::popupMenuTriggeredSlot(QAction* action)
{
	if (preset_actions.contains(action))
	{
		selected_preset = action->text();
		QSettings().setValue("mainviewer/preset", selected_preset);
		emit viewerResized( getViewerSize() );
	}
}

void MainViewer::fullScreenAction(bool checked)
{
	QWidget* parent = parentWidget();

	if (isDockWidget())
	{
		QPixmap p(1,1);
		p.fill(Qt::black);
		if (checked)
		{
			normal_rect = QRect(parent->pos(), parent->size());
			normal_flags = windowFlags();
			normal_margins = layout()->contentsMargins();
			setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
			layout()->setContentsMargins(0, 0, 0, 0);
			showFullScreen();
			m_pitem->setPixmap(p);
			show();
		}
		else
		{
			QDockWidget* dockParent = qobject_cast<QDockWidget*>(parent);
			setWindowFlags(normal_flags);
			layout()->setContentsMargins(normal_margins);
			dockParent->setWidget(this);
			parent->resize(normal_rect.size());
			parent->move(normal_rect.topLeft());
			m_pitem->setPixmap(p);
			show();
		}
	}
	else
	{
		if (checked)
		{
			showFullScreen();
			normal_margins = layout()->contentsMargins();
			layout()->setContentsMargins(0, 0, 0, 0);
		}
		else
		{
			showNormal();
			layout()->setContentsMargins(normal_margins);
		}
	}
}

void MainViewer::statusAction(bool checked)
{
	show_status = checked;
	QSettings().setValue("mainviewer/showstatus", checked);
}

QString MainViewer::presetName() const
{
	return selected_preset;
}

flam3_genome MainViewer::preset() const
{
	return ViewerPresetsModel::getInstance()->preset(selected_preset);
}

bool MainViewer::isDockWidget()
{
	return parent() && parent()->inherits("QDockWidget");
}

void MainViewer::setRenderStatus(RenderStatus* status)
{
	if (show_status && isVisible())
	{
		if (status->State == status->Busy)
		{
			m_titem->setVisible(true);
			m_ritem->setVisible(true);
			QFontMetrics fm(m_titem->font());
			double y = getViewerSize().height() - fm.height() - 5;
			m_titem->setPos(m_graphicsView->mapToScene(0, y));
			m_ritem->setPos(m_graphicsView->mapToScene(0, y + fm.height()));
			const QString s(status->getMessage());
			m_ritem->setRect(fm.boundingRect(s).adjusted(0,-1,6,1));
			m_titem->setPlainText(s);
		}
		else
		{
			m_titem->setVisible(false);
			m_ritem->setVisible(false);
		}
	}
}

bool MainViewer::isPresetSelected() const
{
	return selected_preset != nullPresetText;
}

void MainViewer::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("application/x-mutationpreviewwidget")
	|| event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
		event->acceptProposedAction();
}

void MainViewer::dropEvent(QDropEvent* event)
{
	bool do_render(false);

	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		int idx = event->mimeData()->data("application/x-qabstractitemmodeldatalist").toInt();
		GenomeVector* genomes = getWidget<MainWindow>()->genomeVector();
		if (idx < genomes->size())
		{
			m_request.setName(tr("genome %1").arg(idx + 1));
			m_request.setGenome(genomes->data() + idx);
			do_render = true;
		}
	}
	else if (event->mimeData()->hasFormat("application/x-mutationpreviewwidget"))
	{
		MutationPreviewWidget* mutation = qobject_cast<MutationPreviewWidget*>(event->source());
		m_request.setName(mutation->toolTip().left(30));
		m_request.setGenome(mutation->genome());
		do_render = true;
	}

	if (do_render)
	{
		m_request.setSize(getViewerSize());
		if (isPresetSelected())
			m_request.setImagePresets(preset());
		else
			m_request.setImagePresets(*(getWidget<MainWindow>()->genomeVector()->selectedGenome()));
		RenderThread::getInstance()->render(&m_request);
	}
}

void MainViewer::requestRenderedAction(RenderEvent* event)
{
	if (event->request() == &m_request)
	{
		setPixmap(QPixmap::fromImage(event->request()->image()));
		event->accept();
	}
}


// ----------------------------------------------------------------------------

MainViewerGraphicsView::MainViewerGraphicsView(QWidget* parent)
: QGraphicsView(parent)
{
}

void MainViewerGraphicsView::dragEnterEvent(QDragEnterEvent* /*event*/)
{
}

void MainViewerGraphicsView::dropEvent(QDropEvent* /*event*/)
{
}
