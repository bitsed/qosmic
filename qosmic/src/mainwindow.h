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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"
#include "renderthread.h"
#include "mainviewer.h"
#include "mutationwidget.h"
#include "camerasettingswidget.h"
#include "colorsettingswidget.h"
#include "colorbalancewidget.h"
#include "imgsettingswidget.h"
#include "trianglecoordswidget.h"
#include "triangledensitywidget.h"
#include "mainpreviewwidget.h"
#include "variationswidget.h"
#include "chaoswidget.h"
#include "directoryviewwidget.h"
#include "statuswidget.h"
#include "scripteditwidget.h"
#include "paletteeditor.h"
#include "selectgenomewidget.h"
#include "selecttrianglewidget.h"
#include "adjustscenewidget.h"
#include "editmodeselectorwidget.h"
#include "sheeploopwidget.h"
#include "xfedit.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();
		GenomeVector* genomeVector();
		FigureEditor* xformEditor() const;
		RenderThread* renderThread() const;
		void setCurrentFile(const QString &fileName);
		void setFlameXML(const QString& =QString());
		void reset();
		void setDialogsEnabled(bool);
		bool dialogsEnabled() const;
		void showMainViewer(QString file);
		bool eventFilter(QObject*, QEvent*);
		bool importGenome(const QString&);
		bool exportGenome(const QString&, int);

	public slots:
		void render();
		void renderPreview(int =-1);
		void renderViewer();
		void flameRenderedSlot(RenderEvent* e);
		void triangleSelectedSlot(Triangle*);
		void triangleModifiedSlot(Triangle*);
		void updateStatus(double, double);
		bool loadFile(const QString& filename);
		bool saveFile(const QString& fileName);
		void paletteChangedAction();
		void paletteHueChangedAction();
		void randomizeGenomeAction();
		void mutationSelectedSlot(flam3_genome*);
		void flam3FileSelectedAction(const QString&, bool =false);
		bool saveImage(const QString& =QString(), int =-1);
		bool save();
		void open();
		bool saveAs();
		bool quickSave();
		void genomeSelectedSlot(int);
		void genomesModifiedSlot();
		void presetSelectedSlot();
		void scriptFinishedSlot();
		void importAction();
		void exportAction();
		void runSheepLoop(bool);
		void saveSheepLoop();

	signals:
		void mainWindowChanged();

	protected:
		void closeEvent(QCloseEvent*);
		void showEvent(QShowEvent*);

	private slots:
		void newFile();
		void about();
		void mainViewerResizedAction(const QSize&);
		void previewResizedAction(const QSize&);
		void openRecentFile();
		void mainViewerHiddenAction();
		void addUndoState();
		void undo();
		void redo();
		void kill();

	private:
		void createActions();
		void createMenus();
		void createToolBars();
		void createStatusBar();
		void readSettings();
		void writeSettings();
		void loadGenomeList(flam3_genome*, int);
		QString strippedName(const QString&);
		void updateRecentFileActions();
		void appendFlam3ToGenome(flam3_genome*, int);
		bool readFlam3File(const QString&, flam3_genome**, int*);
		bool writeGenomesToFile(const QString&, flam3_genome*, int);
		void setUndoState(UndoState*);

	protected:
		GenomeVector genomes;
		RenderRequest m_preview_request;
		RenderRequest m_viewer_request;
		RenderRequest m_file_request;
		RenderRequestList m_sheep_requests;
		bool m_dialogsEnabled;

		FigureEditor* m_xfeditor;
		RenderThread* m_rthread;
		MainViewer* m_viewer;
		MutationWidget* m_mutations;
		CameraSettingsWidget* m_cameraSettingsWidget;
		ColorSettingsWidget* m_colorSettingsWidget;
		ColorBalanceWidget* m_colorBalanceWidget;
		ImageSettingsWidget* m_imageSettingsWidget;
		SheepLoopWidget* m_sheepLoopWidget;
		TriangleCoordsWidget* m_coordsWidget;
		TriangleDensityWidget* m_triangleDensityWidget;
		MainPreviewWidget* m_previewWidget;
		VariationsWidget* m_variationsWidget;
		ChaosWidget* m_chaosWidget;
		DirectoryViewWidget* m_directoryViewWidget;
		StatusWidget* m_statusWidget;
		ScriptEditWidget* m_scriptEditWidget;
		Triangle* selectedTriangle;
		Triangle* lastSelected;
		MainViewer* m_fileViewer;
		PaletteEditor* m_paletteEditor;
		SelectGenomeWidget* m_genomeSelectWidget;
		SelectTriangleWidget* m_selectTriangleWidget;
		AdjustSceneWidget* m_adjustSceneWidget;
		EditModeSelectorWidget* m_modeSelectorWidget;
		QList<QDockWidget*> m_dockWidgets;

	private:
		QString curFile;
		QString lastDir;
		QLabel coordsLabel;
		QMenu* fileMenu;
		QMenu* editMenu;
		QMenu* helpMenu;
		QMenu* settingsMenu;
		QToolBar* fileToolBar;
		QToolBar* editToolBar;
		QToolBar* widgetsToolBar;
		QAction* newAct;
		QAction* openAct;
		QAction* saveAct;
		QAction* saveAsAct;
		QAction* saveImageAct;
		QAction* quickSaveAct;
		QAction* openViewerAct;
		QAction* exitAct;
		QAction* killAct;
		QAction* randomAct;
		QAction* rescaleAct;
		QAction* paletteAct;
		QAction* aboutAct;
		QAction* aboutQtAct;
		QAction* openMutationsAct;
		QAction* showFileBarAct;
		QAction* showEditBarAct;
		QAction* showWidgetsBarAct;
		QAction* separatorAct;
		QAction* undoAct;
		QAction* redoAct;
		QAction* cutAct;
		QAction* copyAct;
		QAction* pasteAct;
		QAction* importAct;
		QAction* exportAct;
		QAction* selPrevAct;
		QAction* selNextAct;
		QAction* scaleInAct;
		QAction* scaleOutAct;

		enum { NumRecentFiles = 5 };
		QAction* recentFileActions[NumRecentFiles];
};


#endif
