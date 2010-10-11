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
#include <Godzi/Application>
#include <Godzi/Project>
#include "DataSourceManager"

#include "DesktopMainWindow"

#define EARTH_FILE "http://demo.pelicanmapping.com/rmweb/maps/godzi.earth"

int
main( int argc, char** argv )
{
    QApplication qtApp( argc, argv );

		osg::ref_ptr<Godzi::Application> app = new Godzi::Application(EARTH_FILE);

		DesktopMainWindow top(app);
    top.resize( 800, 600 );
    top.show();

		DataSourceManager manager(app);

		app->actionManager()->doAction(NULL, new Godzi::NewProjectAction());

    qtApp.connect( &qtApp, SIGNAL(lastWindowClosed()), &qtApp, SLOT(quit()) );
    return qtApp.exec();
}
