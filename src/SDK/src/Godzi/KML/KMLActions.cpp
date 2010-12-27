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
#include <Godzi/KML/KMLActions>
#include <Godzi/KML/KMLDataSource>
#include <Godzi/Application>
#include <Godzi/Placemark>
#include <osgViewer/Viewer>
#include <osgEarthUtil/EarthManipulator>

using namespace Godzi;
using namespace Godzi::KML;
using namespace osgEarth::Util;

bool
ZoomToKmlObjectAction::doAction(void* sender, Application* app)
{
    const KMLDataSource* kmlds = static_cast<const KMLDataSource*>( _ds );

    Placemark* placemark = dynamic_cast<Placemark*>( kmlds->getFeature( _objectUID ) );
    if ( placemark && placemark->lookAt().isSet() )
    {
        app->getView()->getManipulator()->setViewpoint( *placemark->lookAt(), 5.0 );
    }
    return true;
}

//------------------------------------------------------------------------
