/***************************************************************************
 *   Copyright (C) 2007 - 2011 by David Bitseff                            *
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
#include <QDesktopWidget>
#include <QDockWidget>
#include <QToolBar>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>

#include "qosmic.h"
#include "mainwindow.h"
#include "renderdialog.h"
#include "renderprogressdialog.h"
#include "flam3filestream.h"

MainWindow::MainWindow()
: QMainWindow(), QosmicWidget(this, "MainWindow")
{
	setupUi(this);

	m_rthread = 0;
	lastSelected = 0;
	m_fileViewer = 0;
	m_dialogsEnabled = true;
	genomes.setSelected(0);
	genomes.undoProviders()->append(this);

	// These are the request instances sent to the renderthread
	m_preview_request.setGenome(genomes.data());
	m_preview_request.setName(tr("preview"));
	m_preview_request.setType(RenderRequest::Preview);

	m_viewer_request.setGenome(genomes.data());
	m_viewer_request.setName(tr("viewer"));
	m_viewer_request.setType(RenderRequest::Image);

	m_file_request.setGenome(genomes.data());
	m_file_request.setName(tr("file.png"));
	m_file_request.setType(RenderRequest::File);

	// the render thread
	m_rthread = RenderThread::getInstance();
	connect(m_rthread, SIGNAL(flameRendered(RenderEvent*)), this, SLOT(flameRenderedSlot(RenderEvent*)));
	logInfo("MainWindow::MainWindow : starting RenderThread");
	m_rthread->start();

	// status widget
	logInfo("MainWindow::MainWindow : creating StatusWidget");
	m_statusWidget = new StatusWidget(this);
	connect(m_rthread, SIGNAL(statusUpdated(RenderStatus*)), m_statusWidget, SLOT(setRenderStatus(RenderStatus*)));

	// figure editor
	logInfo("MainWindow::MainWindow : creating FigureEditor");
	m_xfeditor = new FigureEditor(&genomes, this);
	m_xfeditview->setScene(m_xfeditor);
	// force the containing qgraphicsview to track mouseMoveEvents
	m_xfeditview->viewport()->setMouseTracking(true);
	genomes.undoProviders()->append(m_xfeditor);

	// the figureeditor's editmodeselector
	logInfo("MainWindow::MainWindow : creating EditModeSelectorWidget");
	m_modeSelectorWidget = new EditModeSelectorWidget(m_xfeditor, this);
	centralWidget()->layout()->addWidget(m_modeSelectorWidget);
	connect(m_xfeditor, SIGNAL(triangleModifiedSignal(Triangle*)), this, SLOT(render()));
	connect(m_xfeditor, SIGNAL(triangleListChangedSignal()), this, SLOT(render()));
	connect(m_xfeditor, SIGNAL(triangleSelectedSignal(Triangle*)), this, SLOT(triangleSelectedSlot(Triangle*)));
	connect(m_xfeditor, SIGNAL(coordinateChangeSignal(double,double)), this, SLOT(updateStatus(double,double)));
	connect(m_xfeditor, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));
	connect(m_modeSelectorWidget, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));

	QDockWidget *dock;
	QDockWidget *lastDock;
	QList<QAction*> dockActions;
	setCorner(Qt::BottomLeftCorner,  Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::TopLeftCorner,  Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

	// main preview widget
	logInfo("MainWindow::MainWindow : creating PreviewWidget");
	dock = new QDockWidget(tr("Preview"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_previewWidget = new MainPreviewWidget(&genomes, dock);
	dock->setWidget(m_previewWidget);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_previewWidget, SIGNAL(previewMoved()), this, SLOT(render()));
	connect(m_previewWidget, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));
	lastDock = dock;

	// select a triangle widget
	logInfo("MainWindow::MainWindow : creating SelectTriangleWidget");
	dock = new QDockWidget(tr("Triangles"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_selectTriangleWidget = new SelectTriangleWidget(&genomes, dock);
	dock->setWidget(m_selectTriangleWidget);
	splitDockWidget(lastDock, dock, Qt::Vertical);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_xfeditor, SIGNAL(triangleSelectedSignal(Triangle*)),
			m_selectTriangleWidget, SLOT(triangleSelectedSlot(Triangle*)));
	connect(m_selectTriangleWidget, SIGNAL(dataChanged()), this, SLOT(render()));
	connect(m_selectTriangleWidget, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));
	lastDock = dock;

	// triangle density widget
	logInfo("MainWindow::MainWindow : creating TriangleDensityWidget");
	dock = new QDockWidget(tr("Densities"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_triangleDensityWidget = new TriangleDensityWidget(&genomes, m_xfeditor, dock);
	dock->setWidget(m_triangleDensityWidget);
	splitDockWidget(lastDock, dock, Qt::Vertical);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_xfeditor, SIGNAL(triangleSelectedSignal(Triangle*)),
		m_triangleDensityWidget, SLOT(triangleSelectedSlot(Triangle*)));
	connect(m_triangleDensityWidget, SIGNAL(dataChanged()),
		m_selectTriangleWidget, SLOT(reset()));
	connect(m_selectTriangleWidget, SIGNAL(dataChanged()),
		m_triangleDensityWidget, SLOT(reset()));
	connect(m_triangleDensityWidget, SIGNAL(dataChanged()),
		this, SLOT(render()));
	connect(m_triangleDensityWidget, SIGNAL(genomeMerged()),
		this, SLOT(scriptFinishedSlot()));
	connect(m_triangleDensityWidget, SIGNAL(undoStateSignal()),
		this, SLOT(addUndoState()));
	lastDock = dock;

	// main viewer
	logInfo("MainWindow::MainWindow : creating ViewerWidget");
	dock = new QDockWidget(tr("Viewer"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_viewer = new MainViewer(dock);
	dock->setWidget(m_viewer);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_rthread, SIGNAL(statusUpdated(RenderStatus*)), m_viewer, SLOT(setRenderStatus(RenderStatus*)));

	// image settings widget
	logInfo("MainWindow::MainWindow : creating ImageSettingsWidget");
	dock = new QDockWidget(tr("Image"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_imageSettingsWidget = new ImageSettingsWidget(&genomes, dock);
	dock->setWidget(m_imageSettingsWidget);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_imageSettingsWidget, SIGNAL(dataChanged()), this, SLOT(render()));
	connect(m_imageSettingsWidget, SIGNAL(symmetryAdded()), this, SLOT(scriptFinishedSlot()));
	connect(m_imageSettingsWidget, SIGNAL(presetSelected()), this, SLOT(presetSelectedSlot()));
	lastDock = dock;

	// camera settings widget
	logInfo("MainWindow::MainWindow : creating CameraSettingsWidget");
	dock = new QDockWidget(tr("Camera"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_cameraSettingsWidget = new CameraSettingsWidget(&genomes, dock);
	dock->setWidget(m_cameraSettingsWidget);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_cameraSettingsWidget, SIGNAL(dataChanged()), this, SLOT(render()));
	connect(m_cameraSettingsWidget, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));
	connect(m_previewWidget, SIGNAL(previewMoved()), m_cameraSettingsWidget, SLOT(updateFormData()));
	lastDock = dock;

	// triangle coords widget
	logInfo("MainWindow::MainWindow : creating TriangleCoordsWidget");
	dock = new QDockWidget(tr("Coordinates"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_coordsWidget = new TriangleCoordsWidget(&genomes, dock);
	dock->setWidget(m_coordsWidget);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_xfeditor, SIGNAL(triangleSelectedSignal(Triangle*)),
			 m_coordsWidget, SLOT(triangleSelectedSlot(Triangle*)));
	connect(m_xfeditor, SIGNAL(triangleModifiedSignal(Triangle*)),
			m_coordsWidget, SLOT(triangleModifiedSlot(Triangle*)));
	connect(m_coordsWidget, SIGNAL(dataChanged()), this, SLOT(render()));
	connect(m_coordsWidget, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));
	lastDock = dock;

	// color settings widget
	logInfo("MainWindow::MainWindow : creating ColorSettingsWidget");
	dock = new QDockWidget(tr("Colors"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_colorSettingsWidget = new ColorSettingsWidget(&genomes, dock);
	dock->setWidget(m_colorSettingsWidget);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_xfeditor, SIGNAL(triangleSelectedSignal(Triangle*)),
			m_colorSettingsWidget, SLOT(triangleSelectedSlot(Triangle*)));
	connect(m_colorSettingsWidget, SIGNAL(dataChanged()), this, SLOT(render()));
	connect(m_colorSettingsWidget, SIGNAL(colorSelected(double)),
			m_xfeditor, SLOT(colorChangedAction(double)));
	connect(m_colorSettingsWidget, SIGNAL(undoStateSignal()),
			this, SLOT(addUndoState()));
	lastDock = dock;

	// color balance widget
	logInfo("MainWindow::MainWindow : creating ColorBalanceWidget");
	dock = new QDockWidget(tr("Color Balance"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_colorBalanceWidget = new ColorBalanceWidget(&genomes, dock);
	dock->setWidget(m_colorBalanceWidget);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_colorBalanceWidget, SIGNAL(dataChanged()), this, SLOT(render()));
	connect(m_colorBalanceWidget, SIGNAL(paletteChanged()), this, SLOT(paletteHueChangedAction()));
	lastDock = dock;

	// variations widget
	logInfo("MainWindow::MainWindow : creating VariationsWidget");
	dock = new QDockWidget(tr("Variations"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_variationsWidget = new VariationsWidget(&genomes, dock);
	dock->setWidget(m_variationsWidget);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_xfeditor, SIGNAL(triangleSelectedSignal(Triangle*)),
			m_variationsWidget, SLOT(triangleSelectedSlot(Triangle*)));
	connect(m_variationsWidget, SIGNAL(dataChanged()), m_xfeditor, SLOT(updatePreview()));
	connect(m_variationsWidget, SIGNAL(dataChanged()), this, SLOT(render()));
	connect(m_variationsWidget, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));
	lastDock = dock;

	// chaos widget
	logInfo("MainWindow::MainWindow : creating ChaosWidget");
	dock = new QDockWidget(tr("Chaos"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_chaosWidget = new ChaosWidget(&genomes, dock);
	dock->setWidget(m_chaosWidget);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_xfeditor, SIGNAL(triangleSelectedSignal(Triangle*)),
			m_chaosWidget, SLOT(triangleSelectedSlot(Triangle*)));
	connect(m_chaosWidget, SIGNAL(dataChanged()), this, SLOT(render()));
	connect(m_chaosWidget, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));
	connect(m_selectTriangleWidget, SIGNAL(dataChanged()), m_chaosWidget, SLOT(updateFormData()));
	lastDock = dock;

	// select a genome widget
	logInfo("MainWindow::MainWindow : creating SelectGenomeWidget");
	dock = new QDockWidget(tr("Genomes"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_genomeSelectWidget = new SelectGenomeWidget(&genomes, dock);
	dock->setWidget(m_genomeSelectWidget);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_genomeSelectWidget, SIGNAL(genomeSelected(int)), this, SLOT(genomeSelectedSlot(int)));
	connect(m_genomeSelectWidget, SIGNAL(genomesModified()), this, SLOT(genomesModifiedSlot()));
	lastDock = dock;

	// palettes
	logInfo("MainWindow::MainWindow : creating PalettesWidget");
	dock = new QDockWidget(tr("Palettes"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_paletteEditor = new PaletteEditor(dock);
	dock->setWidget(m_paletteEditor);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_paletteEditor, SIGNAL(paletteChanged()), this, SLOT(paletteChangedAction()));
	connect(m_paletteEditor, SIGNAL(undoStateSignal()), this, SLOT(addUndoState()));
	lastDock = dock;

	// mutations widget
	logInfo("MainWindow::MainWindow : creating MutationsWidget");
	dock = new QDockWidget(tr("Mutations"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_mutations = new MutationWidget(&genomes, m_rthread, dock);
	dock->setWidget(m_mutations);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_mutations, SIGNAL(genomeSelected(flam3_genome*)), this, SLOT(mutationSelectedSlot(flam3_genome*)));
	connect(m_genomeSelectWidget, SIGNAL(genomesModified()), m_mutations, SLOT(reset()));

	// directory view widget
	logInfo("MainWindow::MainWindow : creating DirectoryViewWidget");
	dock = new QDockWidget(tr("Browse"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_directoryViewWidget = new DirectoryViewWidget(dock);
	dock->setWidget(m_directoryViewWidget);
	tabifyDockWidget(lastDock, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_directoryViewWidget, SIGNAL(flam3FileSelected(const QString&)),
			this, SLOT(flam3FileSelectAction(const QString&)));
	connect(m_directoryViewWidget, SIGNAL(flam3FileAppended(const QString&)),
			this, SLOT(flam3FileAppendAction(const QString&)));

	// sheep-loop widget
	logInfo("MainWindow::MainWindow : creating SheepLoopWidget");
	dock = new QDockWidget(tr("Sheep Loops"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_sheepLoopWidget = new SheepLoopWidget(&genomes, dock);
	dock->setWidget(m_sheepLoopWidget);
	addDockWidget(Qt::TopDockWidgetArea, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_sheepLoopWidget, SIGNAL(runSheepLoop(bool)), this, SLOT(runSheepLoop(bool)));
	connect(m_sheepLoopWidget, SIGNAL(saveSheepLoop()), this, SLOT(saveSheepLoop()));
	connect(m_rthread, SIGNAL(flameRenderingKilled()), m_sheepLoopWidget, SLOT(reset()));
	connect(m_genomeSelectWidget, SIGNAL(genomeSelected(int)), m_sheepLoopWidget, SLOT(genomeSelectedSlot(int)));
	connect(m_genomeSelectWidget, SIGNAL(genomesModified()), m_sheepLoopWidget, SLOT(genomesModifiedSlot()));
	connect(m_xfeditor, SIGNAL(triangleListChangedSignal()), m_sheepLoopWidget, SLOT(genomesModifiedSlot()));

	// script editing widget
	logInfo("MainWindow::MainWindow : creating ScriptEditorWidget");
	dock = new QDockWidget(tr("Edit Script"), this);
	dock->setObjectName(dock->windowTitle());
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_scriptEditWidget = new ScriptEditWidget(this, dock);
	dock->setWidget(m_scriptEditWidget);
	addDockWidget(Qt::BottomDockWidgetArea, dock);
	dockActions << dock->toggleViewAction();
	dock->hide();
	m_dockWidgets << dock;
	connect(m_directoryViewWidget, SIGNAL(luaScriptSelected(const QString&)),
			m_scriptEditWidget, SLOT(loadScript(const QString&)));

	createActions();
	createToolBars();
	createMenus();
	createStatusBar();
	foreach(QAction* a, dockActions)
		settingsMenu->addAction(a);
	// install this eventfilter to capture globally the spacebar key
	qApp->installEventFilter(this);
	setCurrentFile("");
}


void MainWindow::triangleSelectedSlot(Triangle* t)
{
	logFine(QString("MainWindow::triangleSelectedSlot : t=0x%1")
			.arg((long)t, 0, 16));
	selectedTriangle = t;
	lastSelected = 0;
	// sneakily update the statusBar without changing position.
	// this is just a way to avoid adding more public functions to
	// figureeditor.
	QString msg = coordsLabel.text();
	int tid = m_xfeditor->selectedTriangleIndex() + 1;
	int tnum = m_xfeditor->getNumberOfTriangles();
	int idx = msg.indexOf(']',0);
	if (idx > -1)
	{
		msg.replace(0,idx,QString(" [%1/%2").arg(tid).arg(tnum));
		coordsLabel.setText(msg);
	}
}

void MainWindow::showMainViewer(QString filename)
{
	if (m_dialogsEnabled)
	{
		if (!m_fileViewer)
			m_fileViewer = new MainViewer(0, "Viewer");
		m_fileViewer->show();
		m_fileViewer->setPixmap(QPixmap(filename));
	}
}

void MainWindow::flameRenderedSlot(RenderEvent* e)
{
	RenderRequest* req = e->request();

	if (req == &m_preview_request)
	{
		logFiner(QString("MainWindow::flameRenderedSlot : updating preview"));
		m_previewWidget->setPixmap(QPixmap::fromImage(req->image()));
		e->accept();
	}
	else if (req == &m_viewer_request)
	{
		logFiner(QString("MainWindow::flameRenderedSlot : updating viewer"));
		m_viewer->setPixmap(QPixmap::fromImage(req->image()));
		e->accept();
	}
	else if (req->name().contains(QString("sheep")))
	{
		logFiner(QString("MainWindow::flameRenderedSlot : displaying sheep %1").arg(req->name()));
		m_previewWidget->setPixmap(QPixmap::fromImage(req->image()));
		// tell the sheeploop widget that the last frame has been shown
		if (req == m_sheep_requests.last())
			m_sheepLoopWidget->reset();
		e->accept();
	}
	else if (req == &m_file_request)
		e->accept();
}


void MainWindow::updateStatus(double posX, double posY)
{
	double tx, ty;
	QMatrix trans(m_xfeditor->transform().inverted());
	trans.map(posX, posY, &tx, &ty);
	switch (m_coordsWidget->coordType())
	{
		case TriangleCoordsWidget::Rect:
			break;
		case TriangleCoordsWidget::Degree:
		{
			Util::rectToPolarDeg(tx, ty, &tx, &ty);
			break;
		}
		case TriangleCoordsWidget::Radian:
			Util::rectToPolar(tx, ty, &tx, &ty);
	}
	CoordinateMark* mark = m_xfeditor->mark();
	QChar separator(QLocale().groupSeparator());
	if (mark->isVisible())
	{
		double mx, my;
		trans.map(mark->center().x(), mark->center().y(), &mx, &my);
		switch (m_coordsWidget->coordType())
		{
			case TriangleCoordsWidget::Rect:
				break;
			case TriangleCoordsWidget::Degree:
			{
				Util::rectToPolarDeg(mx, my, &mx, &my);
				break;
			}
			case TriangleCoordsWidget::Radian:
				Util::rectToPolar(mx, my, &mx, &my);
		}
		coordsLabel.setText(QString(" [%1/%2] : (%L3%4%L5) : mark (%L6%7%L8)")
			.arg(m_xfeditor->selectedTriangleIndex() + 1)
			.arg(m_xfeditor->getNumberOfTriangles())
			.arg(tx, 0, 'f', 4).arg(separator).arg(ty, 0, 'f', 4)
			.arg(mx, 0, 'f', 4).arg(separator).arg(my, 0, 'f', 4));
	}
	else
		coordsLabel.setText(QString(" [%1/%2] : (%L3%4%L5)")
			.arg(m_xfeditor->selectedTriangleIndex() + 1)
			.arg(m_xfeditor->getNumberOfTriangles())
			.arg(tx, 0, 'f', 4).arg(separator).arg(ty, 0, 'f', 4));
}


void MainWindow::closeEvent(QCloseEvent* event)
{
	logInfo("MainWindow::closeEvent : saving current genome");
	Flam3FileStream::autoSave(&genomes, GenomeVector::SaveOnExit | GenomeVector::AlwaysSave);
	m_rthread->stopRendering();
	m_rthread->running = false;

	writeSettings();
	// call close() on dock widgets so they can save settings if needed
	foreach (QDockWidget* dock, m_dockWidgets)
		dock->widget()->close();

	event->accept();
	logInfo("MainWindow::closeEvent : quitting");
}


void MainWindow::showEvent(QShowEvent* event)
{
	logInfo("MainWindow::showEvent : showing mainwindow");
	if (!event->spontaneous())
	{
		logInfo("MainWindow::showEvent : initializing");
		readSettings();

		QFile file(QOSMIC_USERDIR + "/init.lua");
		if (file.open(QIODevice::ReadOnly))
		{
			logInfo("MainWindow::showEvent : reading lua config");
			Lua::LuaThread lua_thread(this);
			QTextStream os(&file);
			lua_thread.setLuaText(os.readAll());
			file.close();
			lua_thread.start();
			while (lua_thread.isRunning())
				QCoreApplication::processEvents();
			logInfo("MainWindow::showEvent : finished reading config");
		}

		render();

		connect(m_viewer, SIGNAL(viewerResized(const QSize&)),
			this, SLOT(mainViewerResizedAction(const QSize&)));
		connect(m_viewer, SIGNAL(viewerHidden()),
				this, SLOT(mainViewerHiddenAction()));
		connect(m_previewWidget, SIGNAL(previewResized(const QSize&)),
				this, SLOT(previewResizedAction(const QSize&)));
	}
}


void MainWindow::newFile()
{
	setFlameXML(DEFAULT_FLAME_XML);
	setCurrentFile("");
}


void MainWindow::open()
{
	QFileDialog dialog(this, tr("Open a flame"), lastDir,
		tr("flam3 xml (*.flam *.flam3 *.flame);;All files (*)"));
	FlamFileIconProvider p;
	dialog.setIconProvider(&p);
	if (dialog.exec())
	{
		QString fileName = dialog.selectedFiles().first();
		if (!fileName.isEmpty())
		{
			lastDir = QFileInfo(fileName).dir().canonicalPath();
			if (loadFile(fileName))
				render();
		}
	}
}


bool MainWindow::save()
{
	if (curFile.isEmpty()) {
		return saveAs();
	} else {
		return saveFile(curFile);
	}
}


bool MainWindow::saveAs()
{
	QFileDialog dialog(this, tr("Save a flame"), lastDir,
			tr("flam3 xml (*.flam *.flam3 *.flame);;All files (*)"));
	FlamFileIconProvider p;
	dialog.setIconProvider(&p);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDefaultSuffix("flam3");
	dialog.selectFile(curFile);
	if (dialog.exec())
	{
		QString fileName(dialog.selectedFiles().first());
		lastDir = QFileInfo(fileName).dir().canonicalPath();
		return saveFile(fileName);
	}
	return false;
}


bool MainWindow::quickSave()
{
	QStringList filter;
	filter << "q*.flam3";
	QDir quickDir(m_directoryViewWidget->currentPath());
	QFileInfoList files = quickDir.entryInfoList(filter, QDir::Files,
			QDir::Name | QDir::Reversed);

	int pad = 8;
	QFileInfo file(quickDir, QString("q%1.flam3").arg(1, pad, 10, QChar('0')));

	if (files.size() > 0)
	{
		QRegExp r("q(\\d+).flam3");
		foreach (QFileInfo i, files)
		{
			QString lastname = i.fileName();
			if (r.exactMatch(lastname))
			{
				int n = r.cap(1).toInt();
				if (++n > 99999999)
					pad += 4;
				lastname = QString("q%1.flam3").arg(n, pad, 10, QChar('0'));
				file = QFileInfo(quickDir, lastname);
				break;
			}
		}
	}

	QString filename = file.absoluteFilePath();
	QString basename = file.baseName();
	QFileInfo imgFile = QFileInfo(file.dir(), basename + ".png");
	logInfo(QString("MainWindow::quickSave : quicksaving to file %1")
			.arg(filename));
	if (saveImage(imgFile.absoluteFilePath()))
	{
		saveFile(filename);
		return true;
	}
	return false;
}

bool MainWindow::saveImage(const QString& filename, int idx)
{
	QString fileName(filename);
	QString origName(fileName);
	if (fileName.isEmpty())
	{
		fileName = origName = curFile;
		if (fileName.isEmpty())
			fileName = "untitled.png";
		else
		{
			int pos = fileName.lastIndexOf('.');
			fileName.replace(pos, fileName.size() - pos, ".png");
		}
	}

	flam3_genome* current_genome;
	if (idx > -1)
		current_genome = genomes.data() + idx;
	else
		current_genome = genomes.selectedGenome();

	QSize fileSize(0,0);
	QSize currentSize(current_genome->width,current_genome->height);
	QString filePreset;
	if (m_dialogsEnabled)
	{
		RenderDialog dialog(this, fileName, lastDir, currentSize,
			ViewerPresetsModel::getInstance()->presetNames());
		if (dialog.exec() == QDialog::Accepted)
		{
			fileName = dialog.absoluteFilePath();
			if (dialog.presetSelected())
				filePreset = dialog.selectedPreset();
			if (dialog.sizeSelected())
				fileSize = dialog.selectedSize();
		}
		else
			return false;
	}

	if (fileName.isEmpty() || !QFileInfo(QFileInfo(fileName).path()).isWritable())
	{
		if (m_dialogsEnabled)
			QMessageBox::warning(this, tr("Application error"),
				tr("Cannot write file %1\n").arg(fileName));

		logWarn(QString("MainWindow::saveImage : couldn't save to file %1")
			.arg(fileName));
		return false;
	}

	lastDir = QFileInfo(fileName).dir().canonicalPath();
	logInfo(QString("MainWindow::saveImage : rendering to file %1 at %2x%3 / %4")
		.arg(fileName)
		.arg(fileSize.width()  ? fileSize.width()  : currentSize.width())
		.arg(fileSize.height() ? fileSize.height() : currentSize.height())
		.arg(filePreset));

	m_file_request.setGenome(current_genome);
	if (filePreset.isEmpty())
		m_file_request.setImagePresets(*current_genome);
	else
		m_file_request.setImagePresets(ViewerPresetsModel::getInstance()->preset(filePreset));
	m_file_request.setName(fileName);
	m_file_request.setType(RenderRequest::File);
	m_file_request.setSize(fileSize);
	m_rthread->render(&m_file_request);

	if (m_dialogsEnabled)
	{
		RenderProgressDialog progress(this, m_rthread);
		if (progress.exec() == QDialog::Rejected)
		{
			m_rthread->stopRendering();
			return false;
		}
		else
		{
			if (progress.showMainViewer())
				showMainViewer(fileName);
			m_directoryViewWidget->fileImageRendered(origName);
		}
	}
	return true;
}

void MainWindow::importAction()
{
	QString fileName
			= QFileDialog::getOpenFileName(this, tr("Import genomes from a file"),
			lastDir, tr("flam3 xml (*.flam *.flam3 *.flame);;All files (*)"));
	if (!fileName.isEmpty())
	{
		lastDir = QFileInfo(fileName).dir().canonicalPath();
		if (importGenome(fileName))
			render();
		else
			QMessageBox::warning(this, tr("Error"),
				tr("Cannot import file %1").arg(fileName));
	}
}

void MainWindow::exportAction()
{
	QString fileName(curFile);
	if (!fileName.isEmpty())
		fileName.replace(QRegExp("\\.flam[3e]?$"),
			QString("_%1.flam3").arg(genomes.selected()));

	QFileDialog dialog(this, tr("Save the current genome"), lastDir,
			tr("flam3 xml (*.flam *.flam3 *.flame);;All files (*)"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDefaultSuffix("flam3");
	dialog.selectFile(fileName);
	if (dialog.exec())
	{
		fileName = dialog.selectedFiles().first();
		lastDir = QFileInfo(fileName).dir().canonicalPath();
		if (!exportGenome(fileName, genomes.selected()))
			QMessageBox::warning(this, tr("Error"),
				tr("Cannot write file to %1").arg(fileName));
	}
}

void MainWindow::about()
{
	static const char* msg =
	"<p><b>Qosmic version %1</b></p>"
	"<p>Copyright (c) 2007 - 2011 David Bitseff</p>"
	"<p>Use and redistribute under the terms of the<br>"
	"<a href=\"http://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">GNU General Public License Version 2</a></p>"
	"<p>Thanks to:<br>"
	"- Scott Draves for the <a href=\"http://flam3.com/\">flam3</a> library<br>"
	"- Erik Reckase for his work on the flam3 library<br>"
	"- Mark James for his <a href=\"http://www.famfamfam.com/lab/icons/silk/\">Silk</a> icon set<br>"
	"- Mark Townsend for the <a href=\"www.apophysis.org\">Apophysis</a> editor"
	"<p>This version was compiled against Qt " QT_VERSION_STR "</p>";
	QMessageBox::about(this, tr("About Qosmic"), QString::fromLatin1(msg).arg(QOSMIC_VERSION));
}


void MainWindow::createActions()
{
	logInfo("MainWindow::createActions : creating actions");
	QList<QKeySequence> keys;

	newAct = new QAction(QIcon(":icons/silk/application.xpm"), tr("Reset"), this);
	newAct->setShortcut(QKeySequence::New);
	newAct->setStatusTip(tr("Load default genome"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

	openAct = new QAction(QIcon(":icons/silk/folder.xpm"), tr("&Open..."), this);
	openAct->setShortcut(QKeySequence::Open);
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	saveAct = new QAction(QIcon(":icons/silk/disk.xpm"), tr("&Save"), this);
	saveAct->setShortcut(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save the flame to disk"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	saveAsAct = new QAction(QIcon(":icons/silk/page_save.xpm"),tr("Save &As..."), this);
	saveAsAct->setShortcut(QKeySequence::SaveAs);
	saveAsAct->setStatusTip(tr("Save the flame under a new name"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	saveImageAct = new QAction(QIcon(":icons/silk/picture_save.xpm"),tr("Render to &file..."), this);
	saveImageAct->setShortcut(QString("Ctrl+I"));
	saveImageAct->setStatusTip(tr("Save an image of current flame"));
	connect(saveImageAct, SIGNAL(triggered()), this, SLOT(saveImage()));

	quickSaveAct = new QAction(QIcon(":icons/silk/disk_multiple.xpm"),tr("Q&uicksave flame..."), this);
	quickSaveAct->setShortcut(QString("Ctrl+P"));
	quickSaveAct->setStatusTip(tr("Quickly save file and image of current flame"));
	connect(quickSaveAct, SIGNAL(triggered()), this, SLOT(quickSave()));

	for (int i = 0; i < NumRecentFiles; ++i) {
		recentFileActions[i] = new QAction(this);
		recentFileActions[i]->setVisible(false);
		connect(recentFileActions[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
	}

	exitAct = new QAction(QIcon(":icons/silk/cross.xpm"),tr("&Quit"), this);
	exitAct->setShortcut(QString("Ctrl+Q"));
	exitAct->setStatusTip(tr("Quit the application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	killAct = new QAction(QIcon(":icons/silk/stop.xpm"),tr("&Stop rendering"), this);
	killAct->setShortcut(QString("Ctrl+K"));
	killAct->setStatusTip(tr("Stop rendering"));
	connect(killAct, SIGNAL(triggered()), this, SLOT(kill()));

	randomAct = new QAction(QIcon(":icons/silk/wand.xpm"),tr("&Random Flame"), this);
	randomAct->setShortcut(QString("Ctrl+R"));
	randomAct->setStatusTip(tr("Generate a random xform set"));
	connect(randomAct, SIGNAL(triggered()), this, SLOT(randomizeGenomeAction()));

	rescaleAct = new QAction(QIcon(":icons/silk/shape_group.xpm"), tr("Rescale View"), this);
	rescaleAct->setShortcut(QString("Ctrl+A"));
	rescaleAct->setStatusTip(tr("Rescale the editor"));
	connect(rescaleAct, SIGNAL(triggered()), m_xfeditor, SLOT(autoScale()));

	selNextAct = new QAction(tr("Select next triangle"), this);
	selNextAct->setShortcut(Qt::Key_F2);
	addAction(selNextAct);
	connect(selNextAct, SIGNAL(triggered()), m_xfeditor, SLOT(selectNextTriangle()));

	selPrevAct = new QAction(tr("Select previous triangle"), this);
	selPrevAct->setShortcut(Qt::Key_F1);
	addAction(selPrevAct);
	connect(selPrevAct, SIGNAL(triggered()), m_xfeditor, SLOT(selectPreviousTriangle()));

	scaleInAct = new QAction(tr("Scale in the scene"), this);
	keys.clear();
	keys << Qt::Key_Semicolon << Qt::Key_Z;
	scaleInAct->setShortcuts(keys);
	addAction(scaleInAct);
	connect(scaleInAct, SIGNAL(triggered()), m_xfeditor, SLOT(scaleInScene()));

	scaleOutAct = new QAction(tr("Scale out the scene"), this);
	keys.clear();
	keys << Qt::Key_Colon << Qt::SHIFT + Qt::Key_Z;
	scaleOutAct->setShortcuts(keys);
	addAction(scaleOutAct);
	connect(scaleOutAct, SIGNAL(triggered()), m_xfeditor, SLOT(scaleOutScene()));

	undoAct = new QAction(QIcon(":icons/silk/arrow_undo.xpm"), tr("Undo"), this);
	keys.clear();
	keys << QKeySequence::Undo << QString("Ctrl+;") << Qt::Key_F3;
	undoAct->setShortcuts(keys);
	undoAct->setStatusTip(tr("Undo"));
	addAction(undoAct);
	connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

	redoAct = new QAction(QIcon(":icons/silk/arrow_redo.xpm"), tr("Redo"), this);
	keys.clear();
	keys << QKeySequence::Redo << QString("Ctrl+Shift+;") << Qt::Key_F4;
	redoAct->setShortcuts(keys);
	redoAct->setStatusTip(tr("Redo"));
	addAction(redoAct);
	connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

	cutAct = new QAction(QIcon(":icons/silk/cut.xpm"), tr("Cut"), this);
	cutAct->setShortcut(QKeySequence::Cut);
	cutAct->setStatusTip(tr("Cut"));
	addAction(cutAct);
	connect(cutAct, SIGNAL(triggered()), m_xfeditor, SLOT(cutTriangleAction()));

	copyAct = new QAction(QIcon(":icons/silk/page_copy.xpm"), tr("Copy"), this);
	copyAct->setShortcut(QKeySequence::Copy);
	copyAct->setStatusTip(tr("Copy"));
	addAction(copyAct);
	connect(copyAct, SIGNAL(triggered()), m_xfeditor, SLOT(copyTriangleAction()));

	pasteAct = new QAction(QIcon(":icons/silk/page_paste.xpm"), tr("Paste"), this);
	pasteAct->setShortcut(QKeySequence::Paste);
	pasteAct->setStatusTip(tr("Paste"));
	addAction(pasteAct);
	connect(pasteAct, SIGNAL(triggered()), m_xfeditor, SLOT(pasteTriangleAction()));

	importAct = new QAction(QIcon(":icons/silk/page_white_get.xpm"), tr("Import genomes"), this);
	importAct->setStatusTip(tr("Import genomes"));
	addAction(importAct);
	connect(importAct, SIGNAL(triggered()), this, SLOT(importAction()));

	exportAct = new QAction(QIcon(":icons/silk/page_white_put.xpm"), tr("Export current genome"), this);
	exportAct->setStatusTip(tr("Export the current genome"));
	addAction(exportAct);
	connect(exportAct, SIGNAL(triggered()), this, SLOT(exportAction()));

	aboutAct = new QAction(QIcon(":icons/silk/information.xpm"),tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}


void MainWindow::createMenus()
{
	logInfo("MainWindow::createMenus : adding menus");
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addAction(saveImageAct);
	separatorAct = fileMenu->addSeparator();
	for (int i = 0; i < NumRecentFiles; ++i)
		fileMenu->addAction(recentFileActions[i]);
	updateRecentFileActions();
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(undoAct);
	editMenu->addAction(redoAct);
	editMenu->addSeparator();
	editMenu->addAction(cutAct);
	editMenu->addAction(copyAct);
	editMenu->addAction(pasteAct);
	editMenu->addSeparator();
	editMenu->addAction(rescaleAct);
	editMenu->addAction(randomAct);
	editMenu->addAction(killAct);
	editMenu->addSeparator();
	editMenu->addAction(importAct);
	editMenu->addAction(exportAct);

	settingsMenu = menuBar()->addMenu(tr("&Widgets"));
	settingsMenu->addAction(showWidgetsBarAct);
	settingsMenu->addAction(showFileBarAct);
	settingsMenu->addAction(showEditBarAct);
	settingsMenu->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Info"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);
}

void MainWindow::updateRecentFileActions()
{
	QSettings settings;
	QStringList files = settings.value("mainwindow/recentfiles").toStringList();

	int numRecentFiles = qMin(files.size(), (int)NumRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = QString("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
		recentFileActions[i]->setText(text);
		recentFileActions[i]->setData(files[i]);
		recentFileActions[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < NumRecentFiles; ++j)
		recentFileActions[j]->setVisible(false);

	separatorAct->setVisible(numRecentFiles > 0);
}

void MainWindow::createToolBars()
{
	logInfo("MainWindow::createToolBars : creating toolbars");

	widgetsToolBar = addToolBar(tr("Widgets"));
	widgetsToolBar->setObjectName("WidgetsToolBar");
	showWidgetsBarAct = new QAction(tr("Show &Widgets Toolbar"), this);
	showWidgetsBarAct->setStatusTip(tr("Show the widgets toolbar"));
	connect(showWidgetsBarAct, SIGNAL(triggered()), widgetsToolBar, SLOT(show()));

	addToolBarBreak();
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->setObjectName("FileToolBar");
	showFileBarAct = new QAction(tr("Show &File Toolbar"), this);
	showFileBarAct->setStatusTip(tr("Show the file toolbar"));
	connect(showFileBarAct, SIGNAL(triggered()), fileToolBar, SLOT(show()));

	editToolBar = addToolBar(tr("Edit"));
	editToolBar->setObjectName("EditToolBar");
	showEditBarAct = new QAction(tr("Show &Edit Toolbar"), this);
	showEditBarAct->setStatusTip(tr("Show scene editing toolbar"));
	connect(showEditBarAct, SIGNAL(triggered()), editToolBar, SLOT(show()));

	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);
	fileToolBar->addAction(saveAsAct);
	fileToolBar->addAction(saveImageAct);
	fileToolBar->addAction(quickSaveAct);

	QAction* action;
	action = qobject_cast<QDockWidget*>(m_viewer->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/monitor.xpm"));
	action->setStatusTip(tr("Viewer"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_previewWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/photo.xpm"));
	action->setStatusTip(tr("Previewer"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_selectTriangleWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/shape_move_forwards.xpm"));
	action->setStatusTip(tr("Triangles"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_variationsWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/table.xpm"));
	action->setStatusTip(tr("Triangle Variations"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_colorSettingsWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/color_swatch.xpm"));
	action->setStatusTip(tr("Triangle Color"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_imageSettingsWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/images.xpm"));
	action->setStatusTip(tr("Image Quality"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_cameraSettingsWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/camera.xpm"));
	action->setStatusTip(tr("Camera"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_colorBalanceWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/contrast.xpm"));
	action->setStatusTip(tr("Color Settings"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_paletteEditor->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/palette.xpm"));
	action->setStatusTip(tr("Palettes"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_mutations->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/application_view_tile.xpm"));
	action->setStatusTip(tr("Mutations"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_genomeSelectWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/pictures.xpm"));
	action->setStatusTip(tr("Genome List"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_triangleDensityWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/chart_bar.xpm"));
	action->setStatusTip(tr("Triangle Densities"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_chaosWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/link.xpm"));
	action->setStatusTip(tr("Chaos"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_directoryViewWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/folder_explore.xpm"));
	action->setStatusTip(tr("Directory Browser"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_coordsWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/text_columns.xpm"));
	action->setStatusTip(tr("Triangle Coordinates"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_sheepLoopWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/film.xpm"));
	action->setStatusTip(tr("Sheep Loop"));
	widgetsToolBar->addAction(action);

	action = qobject_cast<QDockWidget*>(m_scriptEditWidget->parentWidget())->toggleViewAction();
	action->setIcon(QIcon(":icons/silk/script.xpm"));
	action->setStatusTip(tr("Script Editor"));
	widgetsToolBar->addAction(action);


	editToolBar->addAction(undoAct);
	editToolBar->addAction(redoAct);
	editToolBar->addSeparator();
	editToolBar->addAction(cutAct);
	editToolBar->addAction(copyAct);
	editToolBar->addAction(pasteAct);
	editToolBar->addSeparator();
	editToolBar->addAction(randomAct);
	editToolBar->addAction(killAct);
}


void MainWindow::createStatusBar()
{
	statusBar()->addWidget(&coordsLabel);
	statusBar()->addPermanentWidget(m_statusWidget);
}


void MainWindow::readSettings()
{
	logInfo("MainWindow::readSettings : reading settings");

	QSettings settings;
	settings.beginGroup("mainwindow");
	lastDir = settings.value("lastdirectory", QDir::homePath()).toString();

	if (!restoreGeometry(settings.value("geometry").toByteArray()))
	{
		logInfo("MainWindow::readSettings : restoring main window to default geometry");
		resize(920, 760);
		const QRect p( QApplication::desktop()->availableGeometry(this) );
		move(p.center() + QPoint(-400, -350));
		qobject_cast<QDockWidget*>(m_viewer->parentWidget())->setFloating(true);
		qobject_cast<QDockWidget*>(m_mutations->parentWidget())->setFloating(true);
		qobject_cast<QDockWidget*>(m_scriptEditWidget->parentWidget())->setFloating(true);
		m_viewer->parentWidget()->resize(400, 340);
		m_viewer->parentWidget()->move(pos() + QPoint(200, 150));
		m_mutations->parentWidget()->move(pos() + QPoint(20, 60));
		m_scriptEditWidget->parentWidget()->resize(500, 420);
		m_scriptEditWidget->parentWidget()->move(pos() + QPoint(20, 60));
	}
	else
	{
		logInfo("MainWindow::readSettings : restored main window geometry");
	}

	if (!QMainWindow::restoreState(settings.value("state", QByteArray()).toByteArray()))
	{
		logInfo("MainWindow::readSettings : restoring main window to default state");
		// trick the layout into thinking the previewWidget has something to display
		QPixmap pix(QSize(160, 130));
		pix.fill(QColor(Qt::black));
		m_previewWidget->setPixmap(pix);
		qobject_cast<QDockWidget*>(m_previewWidget->parentWidget())->show();
		qobject_cast<QDockWidget*>(m_selectTriangleWidget->parentWidget())->show();
		qobject_cast<QDockWidget*>(m_colorSettingsWidget->parentWidget())->show();
	}
	else
	{
		logInfo(QString("MainWindow::readSettings : restored main window state"));
	}

	logInfo(QString("MainWindow::readSettings : finished"));
}


void MainWindow::writeSettings()
{
	logInfo("MainWindow::writeSettings : writing settings");
	QSettings settings;
	settings.beginGroup("mainwindow");
	settings.setValue("state", saveState());
	settings.setValue("geometry", saveGeometry());
	settings.setValue("lastdirectory", lastDir);
	settings.endGroup();
	m_xfeditor->writeSettings();
}


void MainWindow::setFlameXML(const QString& s)
{
	int ncps = 0;
	flam3_genome* in;
	if (s.isEmpty())
		in = Util::read_xml_string(DEFAULT_FLAME_XML, &ncps);
	else
		in = Util::read_xml_string(s, &ncps);
	logInfo(QString("MainWindow::setFlameXML : found %1 genome").arg(ncps));
	genomes.setData(in, ncps);
	reset();
	render();
}


void MainWindow::openRecentFile()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (action)
	{
		loadFile(action->data().toString());
		render();
	}
}


bool MainWindow::loadFile(const QString& fname)
{
	logInfo(QString("MainWindow::loadFile : opening %1").arg(fname));
	QFile file(fname);
	Flam3FileStream s(&file);
	if (s.read(&genomes))
	{
		logInfo("MainWindow::loadFile : found %d genomes", genomes.size());
		setCurrentFile(fname);
		reset();
		emit mainWindowChanged();
		return true;
	}

	if (m_dialogsEnabled)
		QMessageBox::warning(this, tr("Application error"),
		tr("Couldn't open file %1\n").arg(fname));

	return false;
}


bool MainWindow::saveFile(const QString& fname)
{
	QFile file(fname);
	Flam3FileStream s(&file);
	if (s.write(&genomes))
	{
		setCurrentFile(fname);
		statusBar()->showMessage(tr("File saved"), 2000);
		return true;
	}

	if (m_dialogsEnabled)
		QMessageBox::warning(this, tr("Application error"),
		tr("Couldn't save file %1\n").arg(fname));

	return false;
}


bool MainWindow::importGenome(const QString& fname)
{
	logInfo(QString("MainWindow::importGenome : opening %1").arg(fname));
	int ncps(0);
	flam3_genome* in;
	QFile file(fname);
	Flam3FileStream s(&file);
	if (s.read(&in, &ncps))
	{
		logInfo(QString("MainWindow::importGenome : found %1 genomes").arg(ncps));
		genomes.insertRows(genomes.size(), ncps, in);
		Flam3FileStream::autoSave(&genomes);
		emit mainWindowChanged();
		return true;
	}
	return false;
}


bool MainWindow::exportGenome(const QString& fname, int index)
{
	QFile file(fname);
	Flam3FileStream s(&file);
	logInfo(QString("MainWindow::exportGenome : saving to '%1'").arg(fname));
	if (s.write(genomes.data() + index, 1))
	{
		statusBar()->showMessage(tr("File saved"), 2000);
		return true;
	}
	return false;
}


void MainWindow::setCurrentFile(const QString& fileName)
{
	curFile = fileName;
	QString shownName;
	if (curFile.isEmpty())
		shownName = "untitled.flam3";
	else
	{
		shownName = strippedName(curFile);
		if (shownName != "autosave.flam3")
		{
			QSettings settings;
			QStringList files = settings.value("mainwindow/recentfiles").toStringList();
			QString filePath(QFileInfo(fileName).absoluteFilePath());
			files.removeAll(filePath);
			files.prepend(filePath);
			while (files.size() > NumRecentFiles)
				files.removeLast();
			settings.setValue("mainwindow/recentfiles", files);
			updateRecentFileActions();
		}
	}
	setWindowTitle(QString("Qosmic - %1[*]").arg(shownName));
}


QString MainWindow::strippedName(const QString& fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}


MainWindow::~MainWindow()
{
	if (m_fileViewer)
		delete m_fileViewer;
	delete m_directoryViewWidget;
	delete m_mutations;
	delete m_paletteEditor;
	delete m_viewer;
	delete m_xfeditor;
	delete m_rthread;
}


void MainWindow::paletteChangedAction()
{
	// copy palette to g.palette
	m_paletteEditor->getPalette(genomes.selectedGenome()->palette);
	m_colorSettingsWidget->reset();
	m_colorBalanceWidget->reset();
	m_xfeditor->colorChangedAction(selectedTriangle->xform()->color);
	render();
}

void MainWindow::paletteHueChangedAction()
{
	m_paletteEditor->setPalette(genomes.selectedGenome()->palette);
	m_colorSettingsWidget->reset();
	m_xfeditor->colorChangedAction(selectedTriangle->xform()->color);
	render();
}

void MainWindow::mainViewerResizedAction(const QSize& s)
{
	if (s.isValid())
	{
		logFine(QString("MainWindow::mainViewerResizedAction : new size %1,%2")
				.arg(s.width()).arg(s.height()));
		renderViewer();
	}
}

void MainWindow::previewResizedAction(const QSize& s)
{
	if (s.isValid())
	{
		logFine(QString("MainWindow::previewResizedAction : new size %1,%2")
				.arg(s.width()).arg(s.height()));
		renderPreview();
	}
}


void MainWindow::render()
{
	renderPreview();
	renderViewer();
}

void MainWindow::renderViewer()
{
	if (m_viewer->isVisible())
	{
		flam3_genome* render_genome;
		if (m_triangleDensityWidget->hasMergedGenome())
			render_genome = m_triangleDensityWidget->getMergedGenome();
		else
			render_genome = genomes.selectedGenome();

		m_viewer_request.setGenome(render_genome);
		m_viewer_request.setSize(m_viewer->getViewerSize());
		if (m_viewer->isPresetSelected())
		{
			ViewerPresetsModel* model = ViewerPresetsModel::getInstance();
			m_viewer_request.setImagePresets(model->preset(m_viewer->presetName()));
		}
		else
			m_viewer_request.setImagePresets(*render_genome);
		m_rthread->render(&m_viewer_request);
	}
}

void MainWindow::renderPreview(int idx)
{
	if (m_previewWidget->isVisible())
	{
		flam3_genome* render_genome;
		if (idx > -1)
			render_genome = genomes.data() + idx;
		else if (m_triangleDensityWidget->hasMergedGenome())
			render_genome = m_triangleDensityWidget->getMergedGenome();
		else
			render_genome = genomes.selectedGenome();

		m_preview_request.setGenome(render_genome);
		m_preview_request.setSize(m_previewWidget->getPreviewSize());
		if (m_previewWidget->isPresetSelected())
		{
			ViewerPresetsModel* model = ViewerPresetsModel::getInstance();
			m_preview_request.setImagePresets(model->preset(m_previewWidget->presetName()));
		}
		else
			m_preview_request.setImagePresets(*render_genome);
		m_rthread->render(&m_preview_request);
	}
}


void MainWindow::randomizeGenomeAction()
{
	int ivar = flam3_variation_random;
	flam3_genome s;
	flam3_genome* g = genomes.selectedGenome();
	s = *g; // no need to copy xforms
	flam3_random(g, &ivar, 1, 0, 0);
	// restore the camera/image settings which flam3_random resets
	g->width                  = s.width;
	g->height                 = s.height;
	g->estimator              = s.estimator;
	g->estimator_curve        = s.estimator_curve;
	g->estimator_minimum      = s.estimator_minimum;
	g->sample_density         = s.sample_density;
	g->pixels_per_unit        = s.pixels_per_unit;
	g->spatial_filter_radius  = s.spatial_filter_radius;
	g->nbatches               = s.nbatches;
	g->symmetry               = s.symmetry;
	g->ntemporal_samples      = s.ntemporal_samples;
	m_paletteEditor->setPalette(g->palette);
	m_xfeditor->reset();
	render();
	addUndoState();
}


void MainWindow::flam3FileSelectAction(const QString& name)
{
	if (loadFile(name))
		render();
}

void MainWindow::flam3FileAppendAction(const QString& name)
{
	if (importGenome(name))
		render();
}

void MainWindow::mutationSelectedSlot(flam3_genome* newg)
{
	flam3_genome* g = genomes.selectedGenome();
	flam3_copy(g, newg);
	reset();
	render();
	addUndoState();
}

void MainWindow::reset()
{
	logFine("MainWindow::reset : resetting genome selector");
	m_genomeSelectWidget->reset();
	// the colorsettings and variations are updated when a triangle is selected
	logFine("MainWindow::reset : resetting figure editor");
	m_xfeditor->reset();
	logFine("MainWindow::reset : resetting palettes");
	m_paletteEditor->setPalette(genomes.selectedGenome()->palette);
	logFine("MainWindow::reset : resetting color balance");
	m_colorBalanceWidget->reset();
	logFine("MainWindow::reset : resetting image settings");
	m_imageSettingsWidget->reset();
	logFine("MainWindow::reset : resetting sheep-loop settings");
	m_sheepLoopWidget->reset();
	logFine("MainWindow::reset : resetting camera settings");
	m_cameraSettingsWidget->reset();
}

void MainWindow::scriptFinishedSlot()
{
	// make sure there is something to reset, scripts can do wacky things
	if (genomes.size() < 1)
		newFile();
	else
		reset();
}


GenomeVector* MainWindow::genomeVector()
{
	return &genomes;
}

FigureEditor* MainWindow::xformEditor() const
{
	return m_xfeditor;
}


bool MainWindow::dialogsEnabled() const
{
	return m_dialogsEnabled;
}

void MainWindow::setDialogsEnabled(bool value)
{
	m_dialogsEnabled = value;
}

void MainWindow::genomesModifiedSlot()
{
	logFine("MainWindow::genomesModifiedSlot : enter");
	genomeSelectedSlot(genomes.selected());
}

void MainWindow::genomeSelectedSlot(int /*idx*/)
{
	reset();
	render();
}

void MainWindow::presetSelectedSlot()
{
	logFine("MainWindow::presetSelectedSlot : applying preset");
	m_imageSettingsWidget->reset();
	m_cameraSettingsWidget->reset();
	m_colorBalanceWidget->reset();
	renderPreview();
	if (!m_viewer->isPresetSelected())
		renderViewer();
}

RenderThread* MainWindow::renderThread() const
{
	return m_rthread;
}

void MainWindow::mainViewerHiddenAction()
{
	// stop rendering the mainviewer if it's waiting for an image.
	if (m_rthread->isRendering() && m_rthread->current() == &m_viewer_request)
		m_rthread->stopRendering();
}

bool MainWindow::eventFilter(QObject* /*obj*/, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ev = static_cast<QKeyEvent*>(event);
		int key = ev->key();
		if (key == Qt::Key_Space
			&& (ev->modifiers() & Qt::ControlModifier) )
		{
			addUndoState();
			return true;
		}
		if (key >= Qt::Key_0 && key <= Qt::Key_9
			&& (ev->modifiers() & Qt::AltModifier))
		{
			int idx = key - Qt::Key_0 - 1 ;
			if (idx < 0) idx = 9;
			m_xfeditor->selectTriangle(idx);
			return true;
		}
	}
	return false;
}

void MainWindow::kill()
{
	 if (m_scriptEditWidget->isScriptRunning())
		 m_scriptEditWidget->stopScript();
	 m_rthread->killAll();
}

void MainWindow::provideState(UndoState*)
{
}

void MainWindow::restoreState(UndoState*)
{
	logFine("MainWindow::restoreState : resetting widgets");
	m_paletteEditor->setPalette(genomes.selectedGenome()->palette);
	m_colorBalanceWidget->reset();
	m_imageSettingsWidget->reset();
	m_sheepLoopWidget->reset();
	m_cameraSettingsWidget->reset();
	m_modeSelectorWidget->reset();
	render();
	genomes.updateSelectedPreview();
}

void MainWindow::undo()
{
	QString message;
	if (genomes.undo())
	{
		UndoRing* ring = genomes.undoRing();
		message = tr("undo %1/%2").arg(ring->index()).arg(ring->size());
	}
	else
		message = tr("last undo");

	statusBar()->showMessage(message, 1000);
}

void MainWindow::redo()
{
	QString message;
	if (genomes.redo())
	{
		UndoRing* ring = genomes.undoRing();
		message = tr("redo %1/%2").arg(ring->index()).arg(ring->size());
	}
	else
		message = tr("last redo");

	statusBar()->showMessage(message, 1000);
}

void MainWindow::addUndoState()
{
	logFine(QString("MainWindow::addUndoState : adding"));
	genomes.addUndoState();
	genomes.updateSelectedPreview();
	Flam3FileStream::autoSave(&genomes);
}

void MainWindow::runSheepLoop(bool flag)
{
	static bool run_sequence; // signal this routine to start/stop sequence

	if (!m_previewWidget->isVisible())
		return;

	if (flag)
	{
		int dncp = 0;
		flam3_genome* sheep = m_sheepLoopWidget->createSheepLoop(dncp);

		if (sheep != NULL)
		{
			// resize the request list if neccessary
			while (m_sheep_requests.size() > dncp)
				delete m_sheep_requests.takeLast();

			run_sequence = true;
			for (int i = 0 ; run_sequence && i < dncp ; i++)
			{
				// find a render request
				RenderRequest* request;
				if (i < m_sheep_requests.size())
					request = m_sheep_requests.at(i);
				else
				{
					request = new RenderRequest();
					request->setType(RenderRequest::Queued);
					request->setName(QString("sheep %1").arg(i));
					m_sheep_requests.append(request);
				}

				// and send it to the renderthread
				request->setGenome(sheep);
				request->setTime(i);
				request->setNumGenomes(dncp);
				request->setSize(m_previewWidget->getPreviewSize());
				m_rthread->render(request);
			}
		}
	}
	else
	{
		run_sequence = false;
		m_rthread->killAll();
	}
}

void MainWindow::saveSheepLoop()
{
	int dncp = 0;
	flam3_genome* sheep = m_sheepLoopWidget->createSheepLoop(dncp);
	if (sheep != NULL)
	{
		QFileDialog dialog(this, tr("Save a sheep"), lastDir,
			tr("flam3 xml (*.flam *.flam3 *.flame);;All files (*)"));
		FlamFileIconProvider p;
		dialog.setIconProvider(&p);
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setDefaultSuffix("flam3");
		if (dialog.exec())
		{
			QString fileName(dialog.selectedFiles().first());
			lastDir = QFileInfo(fileName).dir().canonicalPath();
			QFile file(fileName);
			Flam3FileStream s(&file);
			if (s.write(sheep, dncp))
				statusBar()->showMessage(tr("File saved"), 2000);
		}
	}
}
