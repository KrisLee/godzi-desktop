/* --*-c++-*-- */
/* Godzi
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
#include <Godzi/ViewerWidgets>
#include <osgViewer/View>

#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QtGui/QGridLayout>

#define EARTH_FILE "http://demo.pelicanmapping.com/rmweb/maps/bluemarble.earth"

int
main( int argc, char** argv )
{
    QApplication app( argc, argv );

    QWidget* top = new QWidget();
    QGridLayout* layout = new QGridLayout();
    top->setLayout( layout );

    Godzi::ViewerWidget* osg = new Godzi::ViewerWidget( top, 0, 0, true );
    layout->addWidget( osg );

    osg::Node* map = osgDB::readNodeFile( EARTH_FILE );
    map->getOrCreateStateSet()->setMode( GL_LIGHTING, 0 );
    osg->getView()->setSceneData( map );
    
    top->resize( 800, 600 );
    top->show();

    app.connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );
    return app.exec();
}
