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
#include <Godzi/WMS/WMSActions>
#include <Godzi/WMS/WMSDataSource>
#include <Godzi/Application>
#include <Godzi/Placemark>
#include <osgViewer/Viewer>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/WMS>
#include <osgEarthDrivers/wms/WMSOptions>

using namespace Godzi;
using namespace Godzi::WMS;
using namespace osgEarth::Util;

bool
ZoomToWMSLayerAction::doAction(void* sender, Application* app)
{
  const WMSDataSource* wmsds = static_cast<const WMSDataSource*>( _ds );

	const osgEarth::Util::WMSLayer* layer = wmsds->getLayer(_objectUID);
  if (layer)
  {
      osgViewer::View* view = app->getView();
      EarthManipulator* manip = dynamic_cast<EarthManipulator*>(view->getCameraManipulator());
      if (manip)
      {
				osgEarth::Util::Viewpoint viewpoint = manip->getViewpoint();

				double minLon, minLat, maxLon, maxLat;
				const_cast<osgEarth::Util::WMSLayer*>(layer)->getLatLonExtents(minLon, minLat, maxLon, maxLat);

				// If getLatLonExtents returns all zeroes, try getExtents.
				if (minLon == 0.0 && minLat == 0.0 && maxLon == 0.0 && maxLat == 0.0)
					const_cast<osgEarth::Util::WMSLayer*>(layer)->getExtents(minLon, minLat, maxLon, maxLat);

        viewpoint.setFocalPoint(osg::Vec3d((maxLon + minLon) / 2.0,
                                           (maxLat + minLat) / 2.0,
                                           0L));

				double rangeFactor = maxLat != minLat ? maxLat - minLat : maxLon - minLon;
				viewpoint.setRange(((0.5 * rangeFactor) / 0.267949849) * 111000.0);
				if (viewpoint.getRange() == 0.0)
					viewpoint.setRange(20000000.0);

					manip->setViewpoint(viewpoint, 3.0);
			}
  }
  return true;
}

//------------------------------------------------------------------------