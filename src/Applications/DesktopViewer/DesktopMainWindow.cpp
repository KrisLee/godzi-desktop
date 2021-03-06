/* --*-c++-*-- */
/**
 * Godzi
 * Copyright 2010 Pelican Mapping
 * http://pelicanmapping.com
 * http://github.com/gwaldron/godzi
 *
 * Godzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <QtGui>
#include <QString>
#include <osgViewer/View>
#include <osgEarth/XmlUtils>
#include <osgEarthUtil/SkyNode>
#include <osgEarthDrivers/cache_sqlite3/Sqlite3CacheOptions>
#include <Godzi/UI/ViewerWidgets>
#include <Godzi/Earth>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/Actions>
#include "OpenFileDialog"
#include "AppSettingsDialog"
#include "AboutDialog"
#include "MapLayerCatalogWidget"
#include "PlaceSearchWidget"
#include "DesktopMainWindow"

#define ORG_NAME "Pelican Mapping"
#define APP_NAME "Godzi Desktop"

DesktopMainWindow::DesktopMainWindow(GodziApp* app, const std::string& configPath, const std::string& defaultMap)
: _app(app), _configPath(configPath), _defaultMap(defaultMap)
{
	initUi();
	_app->actionManager()->addAfterActionCallback(this);

	connect(_app, SIGNAL(projectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)), this, SLOT(onProjectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)));
}

void DesktopMainWindow::initUi()
{
	setWindowTitle(tr("Godzi[*] - Pelican Mapping"));
	setWindowIcon(QIcon(":/resources/images/pmicon32.png"));
    
	_osgViewer = new Godzi::UI::ViewerWidget( this, 0, 0, true );
    _app->setView( _osgViewer );
	setCentralWidget(_osgViewer);

	createActions();
	createMenus();
	createToolbars();
	createDockWindows();

  QSettings settings(ORG_NAME, APP_NAME);

  //Set default state/geometry if unset
  //if (!settings.value("default_state").isValid())
    settings.setValue("default_state", saveState());

  //Restore window state
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowstate").toByteArray());
  
  updateStatusBar(tr("Ready"));
}

void DesktopMainWindow::createActions()
{
	_newProjectAction = new QAction(tr("New Project"), this);
	_newProjectAction->setShortcut(QKeySequence::New);
	_newProjectAction->setStatusTip(tr("Start a new project"));
	connect(_newProjectAction, SIGNAL(triggered()), this, SLOT(newProject()));
	
	_openProjectAction = new QAction(QIcon(":/resources/images/open.png"), tr("&Open Project"), this);
	_openProjectAction->setShortcut(QKeySequence::Open);
  _openProjectAction->setStatusTip(tr("Open an existing file"));
  connect(_openProjectAction, SIGNAL(triggered()), this, SLOT(openProject()));

	_saveProjectAction = new QAction(QIcon(":/resources/images/save.png"), tr("&Save Project"), this);
	_saveProjectAction->setShortcut(QKeySequence::Save);
	_saveProjectAction->setStatusTip(tr("Save current project"));
	connect(_saveProjectAction, SIGNAL(triggered()), this, SLOT(saveProject()));

	_exitAction = new QAction(tr("&Exit"), this);
	connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	_undoAction = new QAction(QIcon(":/resources/images/undo.png"), tr("&Undo"), this);
	connect(_undoAction, SIGNAL(triggered()), this, SLOT(undo()));

	_settingsAction = new QAction(QIcon(":/resources/images/gear.png"), tr("&Options"), this);
	connect(_settingsAction, SIGNAL(triggered()), this, SLOT(editSettings()));

  _restoreLayoutAction = new QAction(tr("&Restore Layout"), this);
  connect(_restoreLayoutAction, SIGNAL(triggered()), this, SLOT(restoreLayout()));

	_aboutAction = new QAction(QIcon(":/resources/images/info.png"), tr("&About"), this);
	_aboutAction->setStatusTip(tr("About Godzi"));
	connect(_aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

void DesktopMainWindow::createMenus()
{
	_fileMenu = menuBar()->addMenu(tr("&File"));
	_fileMenu->addAction(_newProjectAction);
	_fileMenu->addAction(_openProjectAction);
	_fileMenu->addAction(_saveProjectAction);
	_fileMenu->addSeparator();
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);

	_editMenu = menuBar()->addMenu(tr("&Edit"));
	_editMenu->addAction(_undoAction);
	_editMenu->addSeparator();
	_editMenu->addAction(_settingsAction);

	_viewMenu = menuBar()->addMenu(tr("&View"));
  _viewSeparator = _viewMenu->addSeparator();
  _viewMenu->addAction(_restoreLayoutAction);

	_helpMenu = menuBar()->addMenu(tr("&Help"));
	_helpMenu->addAction(_aboutAction);
}

void DesktopMainWindow::createToolbars()
{
	_fileToolbar = addToolBar(tr("File Toolbar"));
  _fileToolbar->setObjectName(tr("FILE_TOOLBAR"));
	_fileToolbar->setIconSize(QSize(24, 24));
	_fileToolbar->addAction(_openProjectAction);
	_fileToolbar->addAction(_saveProjectAction);
	_fileToolbar->addSeparator();
	_fileToolbar->addAction(_undoAction);
	//_fileToolbar->addSeparator();
	//_fileToolbar->addAction(_settingsAction);

  //Add spacer widget to right-align items that follow
  QWidget* spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _fileToolbar->addWidget(spacer);

  PlaceSearchWidget* searchWidget = new PlaceSearchWidget(_app);
  _fileToolbar->addWidget(searchWidget);

  _viewMenu->insertAction(_viewSeparator, _fileToolbar->toggleViewAction());
}

void DesktopMainWindow::createDockWindows()
{
	QDockWidget *catalogDock = new QDockWidget(tr("Base Map"), this);
  catalogDock->setObjectName(tr("CATALOG_DOCK_WINDOW"));
  catalogDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	MapLayerCatalogWidget* layerCatalog = new MapLayerCatalogWidget(_app);
	catalogDock->setWidget(layerCatalog);
	addDockWidget(Qt::LeftDockWidgetArea, catalogDock);
  _viewMenu->insertAction(_viewSeparator, catalogDock->toggleViewAction());

	QDockWidget *serverDock = new QDockWidget(tr("User Data"), this);
  serverDock->setObjectName(tr("SERVER_DOCK_WINDOW"));
  serverDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	_serverManager = new ServerManagementWidget(_app);
	serverDock->setWidget(_serverManager);
	addDockWidget(Qt::LeftDockWidgetArea, serverDock);
	_viewMenu->insertAction(_viewSeparator, serverDock->toggleViewAction());
}

void DesktopMainWindow::updateStatusBar(const QString &message)
{
	statusBar()->showMessage(message);
}

osgEarth::Map* DesktopMainWindow::loadDefaultMap()
{
	osgEarth::Map* map = 0L;

	if (!_defaultMap.empty())
	{
		osgEarth::MapNode* node = Godzi::readEarthFile(_defaultMap);
		if (node)
			map = node->getMap();
	}

	return map;
}

void DesktopMainWindow::loadScene(const std::string& filename)
{
    if (filename.length() > 0)
			loadScene(osgDB::readNodeFile(filename));
}

void DesktopMainWindow::loadScene(osg::Node* n)
{
	if (n)
	{
		//n->getOrCreateStateSet()->setMode(GL_LIGHTING, 0);
		_osgViewer->getView()->setSceneData(n);

        _osgViewer->resetManipulator();
	}
}

bool DesktopMainWindow::checkSave()
{
	if (_app->isProjectDirty())
	{
		QMessageBox::StandardButton ret = 
			QMessageBox::warning(this, tr("Project Modified"),
			                     tr("The project has been modified.\nDo you want to save your changes?"),
													 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

		if (ret == QMessageBox::Save)
			return saveProject();
		else if (ret == QMessageBox::Cancel)
			return false;
	}

	return true;
}

bool DesktopMainWindow::saveSettings()
{
	 if (_configPath.empty())
		 return false;

   //Save window state via Qt
   QSettings settings(ORG_NAME, APP_NAME);
   settings.setValue("geometry", saveGeometry());
   settings.setValue("windowstate", saveState());

	 Godzi::Config conf("godzi_desktop");
	 conf.addChild(_app->toConfig());

	 osg::ref_ptr<osgEarth::XmlDocument> doc = new osgEarth::XmlDocument( conf );
	 if ( doc.valid() )
	 {
		 std::ofstream output( _configPath.c_str() );
		 if ( output.is_open() )
		 {
			 doc->store( output );
			 return true;
		 }
	 }

	 return false;
}

void DesktopMainWindow::closeEvent(QCloseEvent *event)
 {
	 if (checkSave())
	 {
		 saveSettings();
		 event->accept();
	 }
	 else
		 event->ignore();
 }

void DesktopMainWindow::operator()( void* sender, Godzi::Action* action )
{
	setWindowModified(_app->isProjectDirty());
}

void DesktopMainWindow::newProject()
{
	if (checkSave())
		_app->actionManager()->doAction(this, new Godzi::NewProjectAction(loadDefaultMap()));
}

void DesktopMainWindow::openProject()
{
	if (checkSave())
	{
		QString filename = QFileDialog::getOpenFileName(this, tr("Select project file..."), tr(""), tr("Godzi project files (*.godzi);;All files (*.*)"));

		if (!filename.isNull())
		{
			_app->actionManager()->doAction(this, new Godzi::OpenProjectAction( filename.toUtf8().data(), loadDefaultMap()));
		}
	}
}

bool DesktopMainWindow::saveProject()
{
	std::string filename = _app->getProjectLocation();

	if (filename.empty())
		filename = QFileDialog::getSaveFileName(this, tr("Save..."), tr(""), tr("Godzi project files (*.godzi);;All files (*.*)")).toUtf8().data();

	if (!filename.empty())
	{
		_app->actionManager()->doAction(this, new Godzi::SaveProjectAction(filename));
		return true;
	}

	return false;
}

//TEST
//void DesktopMainWindow::loadMap()
//{
//    OpenFileDialog ofd(tr("Select globe file..."), tr(""), tr("osgEarth files (*.earth);;All files (*.*)"));
//    if (ofd.exec() == QDialog::Accepted)
//    {
//        QString url = ofd.getUrl();
//        if (!url.isNull() && !url.isEmpty())
//            loadScene( url.toUtf8().data() );
//    }
//}
//TEST

void DesktopMainWindow::undo()
{
	_app->actionManager()->undoAction();
}

void DesktopMainWindow::editSettings()
{
  AppSettingsDialog settingsDialog(_app.get());
	if (settingsDialog.exec() == QDialog::Accepted)
	{
    _app->setSunMode(settingsDialog.getSunMode());

    double sunLat, sunLon;
    settingsDialog.getSunPosition(sunLat, sunLon);
    _app->setSunPosition(sunLat, sunLon);


    QDir cachePath(QString(settingsDialog.getCachePath().c_str()));

    osgEarth::Drivers::Sqlite3CacheOptions newOpt;
    newOpt.path() = cachePath.absolutePath().toUtf8().data();
    newOpt.maxSize() = settingsDialog.getCacheMax();

    _app->setCache(newOpt);

		_app->setCacheEnabled(settingsDialog.getCacheEnabled());
	}
}

void DesktopMainWindow::restoreLayout()
{
  QSettings settings(ORG_NAME, APP_NAME);
  restoreState(settings.value("default_state").toByteArray());
}

void DesktopMainWindow::showAbout()
{
	AboutDialog ad;
	ad.exec();
}

void DesktopMainWindow::onProjectChanged(osg::ref_ptr<Godzi::Project> oldProject, osg::ref_ptr<Godzi::Project> newProject)
{
	_root = new osg::Group();

    osgEarth::MapNode* mapNode = new osgEarth::MapNode( _app->getProject()->map() );

    _root->addChild( mapNode );

#ifndef _DEBUG
  // add a sky model:
  osgEarth::Util::SkyNode* sky = _app->createSkyNode();
  if (sky)
  {
	  sky->attach(_osgViewer->getView());
	  _root->addChild(sky);
  }
#endif // _DEBUG

    loadScene(_root);
}
