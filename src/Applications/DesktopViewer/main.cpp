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

#define EARTH_FILE "http://demo.pelicanmapping.com/rmweb/maps/bluemarble.earth"

int
main( int argc, char** argv )
{
	//std::vector<int> v;
	//v.push_back(5);
	//v.push_back(10);
	//v.push_back(15);
	//v.insert(v.begin() + v.size(), 20);
	////v.push_back(20);

	//std::cout << "size: " << v.size() << std::endl;
	//std::cout << "capacity: " << v.capacity() << std::endl;

	//for(std::vector<int>::iterator it = v.begin(); it != v.end(); ++it)
	//{
 //   std::cout << *it;
	//}
	//std::cout << std::endl;

	////v.erase(v.begin() + 2);
	//////std::remove(v.begin(), v.end(), 15);
	//v.erase(remove(v.begin(), v.end(), 15), v.end());

	//std::cout << "size: " << v.size() << std::endl;
	//std::cout << "capacity: " << v.capacity() << std::endl;

	//for(std::vector<int>::iterator it = v.begin(); it != v.end(); ++it)
	//{
 //   std::cout << *it;
	//}
	//std::cout << std::endl;
	//
	//v.insert(v.begin() + v.size(), 15);

	//std::cout << "size: " << v.size() << std::endl;
	//std::cout << "capacity: " << v.capacity() << std::endl;

	//for(std::vector<int>::iterator it = v.begin(); it != v.end(); ++it)
	//{
 //   std::cout << *it;
	//}
	//std::cout << std::endl;
	

    QApplication qtApp( argc, argv );

		osg::ref_ptr<Godzi::Application> app = new Godzi::Application(EARTH_FILE);

		DesktopMainWindow* top = new DesktopMainWindow(app);
    top->resize( 800, 600 );
    top->show();

		DataSourceManager manager(app);

		app->actionManager()->doAction(NULL, new Godzi::NewProjectAction());

    qtApp.connect( &qtApp, SIGNAL(lastWindowClosed()), &qtApp, SLOT(quit()) );
    return qtApp.exec();
}
