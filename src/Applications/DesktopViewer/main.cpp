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
#include <QtGui/QApplication>
#include <Godzi/Earth>
#include <Godzi/Application>
#include <Godzi/Project>
#include "GodziQtApplication"
#include "DataSourceManager"
#include "DesktopMainWindow"

#define EARTH_FILE "http://demo.pelicanmapping.com/rmweb/maps/godzi.earth"
#define LOCAL_EARTH_FILE "data/local_default.earth"

int
main( int argc, char** argv )
{
    //QApplication qtApp( argc, argv );
		GodziQtApplication qtApp(argc, argv);

		osg::ref_ptr<Godzi::Application> app = new Godzi::Application();

		std::string defaultMap = "";
		osgEarth::MapNode* node = Godzi::readEarthFile(EARTH_FILE);
		if (node)
		{
			defaultMap = EARTH_FILE;
		}
		else
		{
			node = Godzi::readEarthFile(LOCAL_EARTH_FILE);
			if (node)
				defaultMap = LOCAL_EARTH_FILE;
		}

		DesktopMainWindow top(app, defaultMap);
    top.resize( 800, 600 );
    top.show();

		DataSourceManager manager(app);

		app->actionManager()->doAction(NULL, new Godzi::NewProjectAction(node ? node->getMap() : 0L));

    qtApp.connect( &qtApp, SIGNAL(lastWindowClosed()), &qtApp, SLOT(quit()) );
    return qtApp.exec();
}
