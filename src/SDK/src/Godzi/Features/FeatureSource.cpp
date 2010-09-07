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

#include <Godzi/Features/FeatureSource>
#include <Godzi/Features/Placemark>
#include <Godzi/Features/Symbol>
#include <osgEarthSymbology/Style>
#include <osgEarth/FileUtils>
#include <osgEarth/Registry>
#include "kml/dom.h"
#include "kml/base/file.h"
#include "kml/engine.h"
#include <iostream>

using namespace Godzi::Features;

KMLFeatureSource::KMLFeatureSource( const osgEarth::PluginOptions* options): osgEarth::Features::FeatureSource(options)
{
    _options = dynamic_cast<const KMLFeatureSourceOptions*>(options);
    if ( !_options.valid() )
        _options = new KMLFeatureSourceOptions( options );
}


osgEarth::Features::FeatureProfile* KMLFeatureSource::createFeatureProfile()
{
    return new osgEarth::Features::FeatureProfile(osgEarth::Registry::instance()->getGlobalGeodeticProfile()->getExtent());
}

static const kmldom::FeaturePtr getRootFeature(const kmldom::ElementPtr& root) 
{
    const kmldom::KmlPtr kml = kmldom::AsKml(root);
    if (kml && kml->has_feature()) {
        return kml->get_feature();
    }
    return kmldom::AsFeature(root);
}


static osg::Vec3dArray* CoordinatesToVec3dArray(kmldom::CoordinatesPtr coords)
{
    if (!coords || !coords->get_coordinates_array_size())
        return 0;

    osg::Vec3dArray* array = new osg::Vec3dArray(coords->get_coordinates_array_size());
    for (size_t i = 0; i < coords->get_coordinates_array_size(); ++i)
    {
        kmlbase::Vec3 in = coords->get_coordinates_array_at(i);

        (*array)[i] = osg::Vec3d(in.get_longitude(),
                                 in.get_latitude(),
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

static
osgEarth::Symbology::Geometry* createGeometryFromElement(const kmldom::GeometryPtr kmlGeom)
{
    switch (kmlGeom->Type()) 
    {
    case kmldom::Type_Point:
    {
        const kmldom::CoordinatesPtr coord = kmldom::AsPoint(kmlGeom)->get_coordinates();
        osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
        if (array) {
            osgEarth::Symbology::PointSet* geom = new osgEarth::Symbology::PointSet(array);

#if 0
            geom->setExtrude(kmldom::AsPoint(kmlGeom)->get_extrude());
            switch (kmldom::AsPoint(kmlGeom)->get_altitudemode()) {
            case kmldom::ALTITUDEMODE_CLAMPTOGROUND:
                geom->setAltitudeMode(Point::ClampToGround);
                break;
            case kmldom::ALTITUDEMODE_RELATIVETOGROUND:
                geom->setAltitudeMode(Point::RelativeToGround);
                break;
            case kmldom::ALTITUDEMODE_ABSOLUTE:
                geom->setAltitudeMode(Point::Absolute);
                break;
            default:
                geom->setAltitudeMode(Point::ClampToGround);
                break;
            }
#endif
            return (geom);
        }
    }
    break;

    case kmldom::Type_LineString:
    {
        const kmldom::CoordinatesPtr coord = kmldom::AsLineString(kmlGeom)->get_coordinates();
        osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
        if (array) {
            osgEarth::Symbology::LineString* geom = new osgEarth::Symbology::LineString(array);
            return (geom);
        }
    }
    break;

    case kmldom::Type_LinearRing:
    {
        const kmldom::CoordinatesPtr coord = kmldom::AsLinearRing(kmlGeom)->get_coordinates();
        osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
        if (array) {
            osgEarth::Symbology::Ring* geom = new osgEarth::Symbology::Ring(array);
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
                osgEarth::Symbology::Polygon* geom = new osgEarth::Symbology::Polygon(array);

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
            osgEarth::Symbology::MultiGeometry* geom = new osgEarth::Symbology::MultiGeometry;
        
            for (size_t i = 0; i < mgeom->get_geometry_array_size(); ++i) {
                const kmldom::GeometryPtr g = mgeom->get_geometry_array_at(i);
                osgEarth::Symbology::Geometry* newgeom = createGeometryFromElement(g);
                if (newgeom)
                    geom->getComponents().push_back(newgeom);
            }
            return geom;
        }
    }
    break;
    }

    return 0;
}


template <class T> T* createSymbol(const kmldom::GeometryPtr kmlGeom)
{
    T* symbol = new T;
    {
        const kmldom::AltitudeGeometryCommon* alt = dynamic_cast<const kmldom::AltitudeGeometryCommon*>(kmlGeom.get());
        if (alt) {
            switch (alt->get_altitudemode()) {
            case kmldom::ALTITUDEMODE_CLAMPTOGROUND:
                symbol->altitude()->setAltitudeMode(KMLAltitude::ClampToGround);
                break;
            case kmldom::ALTITUDEMODE_RELATIVETOGROUND:
                symbol->altitude()->setAltitudeMode(KMLAltitude::RelativeToGround);
                break;
            case kmldom::ALTITUDEMODE_ABSOLUTE:
                symbol->altitude()->setAltitudeMode(KMLAltitude::Absolute);
                break;
            default:
                symbol->altitude()->setAltitudeMode(KMLAltitude::ClampToGround);
                break;
            }
        }
    }
    {
        symbol->extrude()->setExtrude(false);
        const kmldom::ExtrudeGeometryCommon* ext = dynamic_cast<const kmldom::ExtrudeGeometryCommon*>(kmlGeom.get());
        if (ext) {
            symbol->extrude()->setExtrude(ext->get_extrude());
        }
    }
    
    return symbol;
}

static osgEarth::Symbology::Style* setupStyle(const kmldom::GeometryPtr kmlGeom)
{
    osg::ref_ptr<osgEarth::Symbology::Style> style = new osgEarth::Symbology::Style;
    switch (kmlGeom->Type()) 
    {
    case kmldom::Type_Point:
        style->addSymbol(createSymbol<KMLPointSymbol>(kmlGeom));
    break;
    case kmldom::Type_LineString:
        style->addSymbol(createSymbol<KMLLineSymbol>(kmlGeom));
    break;
    case kmldom::Type_LinearRing:
        style->addSymbol(createSymbol<KMLLineSymbol>(kmlGeom));
    break;
    case kmldom::Type_Polygon:
        style->addSymbol(createSymbol<KMLPolygonSymbol>(kmlGeom));
    break;
    default:
        return 0;
        break;
    }
    return style.release();
}



void collectFeature(osgEarth::Features::FeatureList& fl, const kmldom::FeaturePtr& feature, int depth)
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
                osgEarth::Symbology::Geometry* geom = createGeometryFromElement(placemark->get_geometry());
                if (geom) {
                    p->setGeometry(geom);

                    osgEarth::Symbology::Style* style = setupStyle(placemark->get_geometry());
                    if (style)
                        p->style() = style;

                } else {
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


KMLFeatureCursor::KMLFeatureCursor(const osgEarth::Features::FeatureList& featureList): _featureList(featureList) 
{
    _iterator = _featureList.begin();
}

bool KMLFeatureCursor::hasMore() const
{
    if (_iterator != _featureList.end())
        return true;
    return false;
}

osgEarth::Features::Feature* KMLFeatureCursor::nextFeature()
{
    if (!hasMore())
        return 0;
    osgEarth::Features::Feature* f = *_iterator;
    _iterator++;
    return f;
}


osgEarth::Features::FeatureCursor* KMLFeatureSource::createFeatureCursor( const Symbology::Query& query )
{
    return new KMLFeatureCursor(_features);
}

//override
void KMLFeatureSource::initialize( const std::string& referenceURI )
{
    if ( _options->url().isSet() )
    {
        _url = osgEarth::getFullPath( referenceURI, _options->url().value() );
    }


    std::string file = _url;

    osgEarth::Features::FeatureList features;

    // Read it.
    std::string kml;
    if (!kmlbase::File::ReadFileToString(file, &kml)) {
        std::cout << file << " read failed" << std::endl;
        return;
    }

    // Parse it.
    std::string errors;
    kmldom::ElementPtr root = kmldom::Parse(kml, &errors);

    if (!root) {
        std::cout << errors << std::endl;
        return;
    }

    const kmldom::FeaturePtr feature = getRootFeature(root);

    if (feature) {
        collectFeature(features, feature, 0);
    } else {
        std::cout << "No root feature" << std::endl;
    }

    _features = features;
}
