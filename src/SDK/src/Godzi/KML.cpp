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

#include <osg/Math>
#include <Godzi/KML>
#include <Godzi/Features/Placemark>
#include <iostream>

#include "kml/dom.h"
#include "kml/base/file.h"
#include "kml/engine.h"
using namespace Godzi;
using namespace Godzi::Features;

static osg::Vec3dArray* CoordinatesToVec3dArray(kmldom::CoordinatesPtr coords)
{
    if (!coords || !coords->get_coordinates_array_size())
        return 0;

    osg::Vec3dArray* array = new osg::Vec3dArray(coords->get_coordinates_array_size());
    for (size_t i = 0; i < coords->get_coordinates_array_size(); ++i)
    {
        kmlbase::Vec3 in = coords->get_coordinates_array_at(i);

        (*array)[i] = osg::Vec3d(osg::DegreesToRadians(in.get_longitude()),
                                 osg::DegreesToRadians(in.get_latitude()),
                                 in.has_altitude()? in.get_altitude() : 0);
    }
    return array;
}

static void printIndented(std::string item, int depth) {
  while (depth--) {
      std::cout << "  ";
  }
  std::cout << item;
}


static const kmldom::FeaturePtr getRootFeature(const kmldom::ElementPtr& root) 
{
    const kmldom::KmlPtr kml = kmldom::AsKml(root);
    if (kml && kml->has_feature()) {
        return kml->get_feature();
    }
    return kmldom::AsFeature(root);
}


static
Geometry* createGeometryFromElement(const kmldom::GeometryPtr kmlGeom)
{
    switch (kmlGeom->Type()) 
    {
    case kmldom::Type_Point:
    {
        const kmldom::CoordinatesPtr coord = kmldom::AsPoint(kmlGeom)->get_coordinates();
        osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
        if (array) {
            Point* geom = new Point(array);
            return (geom);
        }
    }
    break;

    case kmldom::Type_LineString:
    {
        const kmldom::CoordinatesPtr coord = kmldom::AsLineString(kmlGeom)->get_coordinates();
        osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
        if (array) {
            LineString* geom = new LineString(array);
            return (geom);
        }
    }
    break;

    case kmldom::Type_LinearRing:
    {
        const kmldom::CoordinatesPtr coord = kmldom::AsLinearRing(kmlGeom)->get_coordinates();
        osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
        if (array) {
            LinearRing* geom = new LinearRing(array);
            return (geom);
        }
    }
    break;

    case kmldom::Type_Polygon:
    {
        const kmldom::PolygonPtr poly = kmldom::AsPolygon(kmlGeom);
        if (poly->has_outerboundaryis()) {
            const kmldom::CoordinatesPtr coord = poly->get_outerboundaryis()->get_linearring()->get_coordinates();

            osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
            if (array) {
                Polygon* geom = new Polygon(array);

                for (size_t i = 0; i < poly->get_innerboundaryis_array_size(); ++i) {
                    const kmldom::CoordinatesPtr inner = poly->get_innerboundaryis_array_at(i)->get_linearring()->get_coordinates();
                    if (inner)
                        geom->getHoles().push_back(new osgEarth::Symbology::Ring(CoordinatesToVec3dArray(inner)));
                }

                return (geom);
            }
        }
    }
    break;

    case kmldom::Type_MultiGeometry:
    {
        const kmldom::MultiGeometryPtr mgeom = kmldom::AsMultiGeometry(kmlGeom);
        if (mgeom && mgeom->get_geometry_array_size() > 0)
        {
            MultiGeometry* geom = new MultiGeometry;
        
            for (size_t i = 0; i < mgeom->get_geometry_array_size(); ++i) {
                const kmldom::GeometryPtr g = mgeom->get_geometry_array_at(i);
                Geometry* newgeom = createGeometryFromElement(g);
                if (newgeom)
                    geom->getGeometryList().push_back(newgeom);
            }
            return geom;
        }
    }
    break;
    }

    return 0;
}

void collectFeature(FeatureList& fl, const kmldom::FeaturePtr& feature, int depth)
{
    switch (feature->Type()) {
    case kmldom::Type_Document:
        printIndented("Document", depth);
        break;
    case kmldom::Type_Folder:
        printIndented("Folder", depth);
        break;
    case kmldom::Type_GroundOverlay:
        printIndented("GroundOverlay", depth);
        break;
    case kmldom::Type_NetworkLink:
        printIndented("NetworkLink", depth);
        break;
    case kmldom::Type_PhotoOverlay:
        printIndented("PhotoOverlay", depth);
        break;
    case kmldom::Type_Placemark:
    {
        const kmldom::PlacemarkPtr placemark = kmldom::AsPlacemark(feature);
        if (placemark) {
            Placemark* p = new Placemark;
            p->setName(placemark->get_name());
            if (placemark->has_geometry()) {
                Geometry* geom = createGeometryFromElement(placemark->get_geometry());
                if (geom)
                    p->setGeometry(geom);
                else {
                    osg::notify(osg::WARN) << "cant retrieve geometry for placemark " << p->getName() << std::endl;
                }
                printIndented("Placemark", depth);
                fl.push_back(p);
            }
        }
    }
    break;
    case kmldom::Type_ScreenOverlay:
        printIndented("ScreenOverlay", depth);
        break;
    default:
        printIndented("other", depth);
        break;
    }

    if (feature->has_name()) {
        std::cout << " " << feature->get_name();
    }
    std::cout << std::endl;

    if (const kmldom::ContainerPtr container = kmldom::AsContainer(feature)) {
        for (size_t i = 0; i < container->get_feature_array_size(); ++i) {
            collectFeature(fl, container->get_feature_array_at(i), depth+1);
        }
    }
}


FeatureList Godzi::readFeaturesFromKML(const std::string& file)
{
    FeatureList features;
    // Read it.
    std::string kml;
    if (!kmlbase::File::ReadFileToString(file, &kml)) {
        std::cout << file << " read failed" << std::endl;
        return features;
    }

    // Parse it.
    std::string errors;
    kmldom::ElementPtr root = kmldom::Parse(kml, &errors);

    if (!root) {
        std::cout << errors << std::endl;
        return features;
    }

    const kmldom::FeaturePtr feature = getRootFeature(root);

    if (feature) {
        collectFeature(features, feature, 0);
    } else {
        std::cout << "No root feature" << std::endl;
    }

    return features;
}
