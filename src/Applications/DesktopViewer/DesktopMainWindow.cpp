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
#include <Godzi/KML>
#include <Godzi/Features/ApplyFeature>
#include "OpenFileDialog"
#include "DesktopMainWindow"

DesktopMainWindow::DesktopMainWindow(const std::string& filename = "")
{
	setWindowTitle(tr("Godzi"));
	setWindowIcon(QIcon(":/resources/images/globe.png"));

	_osgViewer = new Godzi::UI::ViewerWidget( this, 0, 0, true );
	
	if (filename.length() > 0)
		LoadScene(filename);

	setCentralWidget(_osgViewer);

	CreateActions();
	CreateMenus();
	CreateToolbars();
	CreateDockWindows();
	UpdateStatusBar(tr("Ready"));
}

void DesktopMainWindow::CreateActions()
{
	_openAction = new QAction(QIcon(":/resources/images/open.png"), tr("&Open"), this);
	_openAction->setShortcut(QKeySequence::Open);
  _openAction->setStatusTip(tr("Open an existing file"));
  connect(_openAction, SIGNAL(triggered()), this, SLOT(open()));

	_saveProjectAction = new QAction(QIcon(":/resources/images/save.png"), tr("&Save Project"), this);
	_saveProjectAction->setShortcut(QKeySequence::Save);
	_saveProjectAction->setStatusTip(tr("Save current project"));
	connect(_saveProjectAction, SIGNAL(triggered()), this, SLOT(saveProject()));

	_aboutAction = new QAction(QIcon(":/resources/images/info.png"), tr("&About"), this);
	_aboutAction->setStatusTip(tr("About Godzi"));
	connect(_aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

void DesktopMainWindow::CreateMenus()
{
	_fileMenu = menuBar()->addMenu(tr("&File"));
	_fileMenu->addAction(_openAction);
	_fileMenu->addAction(_saveProjectAction);

	_viewMenu = menuBar()->addMenu(tr("&View"));

	_helpMenu = menuBar()->addMenu(tr("&Help"));
	_helpMenu->addAction(_aboutAction);
}

void DesktopMainWindow::CreateToolbars()
{
	_fileToolbar = addToolBar(tr("File"));
	_fileToolbar->setIconSize(QSize(24, 24));
	_fileToolbar->addAction(_openAction);
	_fileToolbar->addAction(_saveProjectAction);
}

void DesktopMainWindow::CreateDockWindows()
{
	QDockWidget *dock = new QDockWidget(tr("Servers"), this);
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	_serverManager = new ServerManagementWidget();
	dock->setWidget(_serverManager);
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	_viewMenu->addAction(dock->toggleViewAction());
}

void DesktopMainWindow::UpdateStatusBar(const QString &message)
{
	statusBar()->showMessage(message);
}

void DesktopMainWindow::LoadScene(const std::string& filename)
{
	osg::Node* map = osgDB::readNodeFile( filename );
	map->getOrCreateStateSet()->setMode( GL_LIGHTING, 0 );
	_osgViewer->getView()->setSceneData( map );

	//TEST
	//Godzi::Features::FeatureList featureList = Godzi::readFeaturesFromKML("./data/example.kml");
  //Godzi::Features::ApplyFeature featuresMaker;
  //featuresMaker.setFeatures(featureList);
  //map->accept(featuresMaker);
	//TEST
}

void DesktopMainWindow::open()
{
	OpenFileDialog ofd(tr("Select globe file..."), tr(""), tr("osgEarth files (*.earth);;All files (*.*)"));
	if (ofd.exec() == QDialog::Accepted)
	{
		QString url = ofd.getUrl();
		if (!url.isNull() && !url.isEmpty())
			LoadScene(url.toStdString());
	}
}

void DesktopMainWindow::saveProject()
{

}

void DesktopMainWindow::showAbout()
{
}
