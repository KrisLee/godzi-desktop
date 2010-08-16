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
#include <Godzi/UI/ViewerWidgets>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/Actions>
#include <Godzi/KML>
#include <Godzi/Features/ApplyFeature>
#include "OpenFileDialog"
#include "AboutDialog"
#include "DesktopMainWindow"

DesktopMainWindow::DesktopMainWindow(Godzi::Application* app)
: _app(app)
{
	initUi();
	_app->actionManager()->addAfterActionCallback(this);
}

void DesktopMainWindow::initUi()
{
	setWindowTitle(tr("Godzi"));
	setWindowIcon(QIcon(":/resources/images/globe.png"));

	_osgViewer = new Godzi::UI::ViewerWidget( this, 0, 0, true );
	setCentralWidget(_osgViewer);

	createActions();
	createMenus();
	createToolbars();
	createDockWindows();
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

	_loadMapAction = new QAction(tr("&Load Map"), this);
	_loadMapAction->setStatusTip(tr("Load new base map"));
	connect(_loadMapAction, SIGNAL(triggered()), this, SLOT(loadMap()));

	_exitAction = new QAction(tr("&Exit"), this);
	connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));

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
	_fileMenu->addAction(_loadMapAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);

	_viewMenu = menuBar()->addMenu(tr("&View"));

	_helpMenu = menuBar()->addMenu(tr("&Help"));
	_helpMenu->addAction(_aboutAction);
}

void DesktopMainWindow::createToolbars()
{
	_fileToolbar = addToolBar(tr("File"));
	_fileToolbar->setIconSize(QSize(24, 24));
	_fileToolbar->addAction(_openProjectAction);
	_fileToolbar->addAction(_saveProjectAction);
}

void DesktopMainWindow::createDockWindows()
{
	QDockWidget *dock = new QDockWidget(tr("Servers"), this);
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	_serverManager = new ServerManagementWidget();
	dock->setWidget(_serverManager);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	_viewMenu->addAction(dock->toggleViewAction());
}

void DesktopMainWindow::updateStatusBar(const QString &message)
{
	statusBar()->showMessage(message);
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
		n->getOrCreateStateSet()->setMode(GL_LIGHTING, 0);
		_osgViewer->getView()->setSceneData(n);
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

void DesktopMainWindow::closeEvent(QCloseEvent *event)
 {
	 if (checkSave())
		 event->accept();
	 else
		 event->ignore();
 }

void DesktopMainWindow::operator()( void* sender, Godzi::Action* action )
{
	setWindowModified(_app->isProjectDirty());
	loadScene(new osgEarth::MapNode(_app->getProject()->map()));
}

void DesktopMainWindow::newProject()
{
	if (checkSave())
		_app->actionManager()->doAction(this, new Godzi::NewProjectAction());
}

void DesktopMainWindow::openProject()
{
	if (checkSave())
	{
		QString filename = QFileDialog::getOpenFileName(this, tr("Select project file..."), tr(""), tr("Godzi project files (*.godzi);;All files (*.*)"));

		if (!filename.isNull())
		{
			_app->actionManager()->doAction(this, new Godzi::OpenProjectAction(filename.toStdString()));
		}
	}
}

bool DesktopMainWindow::saveProject()
{
	std::string filename = _app->getProjectLocation();

	if (filename.empty())
		filename = QFileDialog::getSaveFileName(this, tr("Save..."), tr(""), tr("Godzi project files (*.godzi);;All files (*.*)")).toStdString();

	if (!filename.empty())
	{
		_app->actionManager()->doAction(this, new Godzi::SaveProjectAction(filename));
		return true;
	}

	return false;
}

void DesktopMainWindow::loadMap()
{
	OpenFileDialog ofd(tr("Select globe file..."), tr(""), tr("osgEarth files (*.earth);;All files (*.*)"));
	if (ofd.exec() == QDialog::Accepted)
	{
		QString url = ofd.getUrl();
		if (!url.isNull() && !url.isEmpty())
			loadScene(url.toStdString());
	}
}

void DesktopMainWindow::showAbout()
{
	AboutDialog ad;
	ad.exec();
}