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

#include <Godzi/KML/KMLFeatureSource>
#include <Godzi/Features/Placemark>
#include <Godzi/Features/Symbol>
#include <osgEarthSymbology/Style>
#include <osgEarth/FileUtils>
#include <osgEarth/Registry>
#include <osgEarth/Notify>
#include <osgEarth/HTTPClient>
#include <osgDB/FileNameUtils>
#include "kml/dom.h"
#include "kml/base/file.h"
#include "kml/engine.h"
#include <iostream>

#define DEFAULT_LABEL_SIZE 32
#define LC "[Godzi::KMLFeatureSource] "

using namespace Godzi::KML;
using namespace Godzi::Features;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;

//------------------------------------------------------------------------

namespace
{
    /** Finds the root feature in a KML document */
    const kmldom::FeaturePtr
    getRootFeature(const kmldom::ElementPtr& root) 
    {
        const kmldom::KmlPtr kml = kmldom::AsKml(root);
        if (kml && kml->has_feature()) {
            return kml->get_feature();
        }
        return kmldom::AsFeature(root);
    }

    /** Converts a KML coordinate array to an OSG coordinate array */
    osg::Vec3dArray*
    CoordinatesToVec3dArray(kmldom::CoordinatesPtr coords)
    {
        if (!coords || !coords->get_coordinates_array_size())
            return 0;

        osg::Vec3dArray* array = new osg::Vec3dArray(coords->get_coordinates_array_size());
        for (size_t i = 0; i < coords->get_coordinates_array_size(); ++i)
        {
            kmlbase::Vec3 in = coords->get_coordinates_array_at(i);

            (*array)[i] = osg::Vec3d(
                in.get_longitude(),
                in.get_latitude(),
                in.has_altitude()? in.get_altitude() : 0);
        }
        return array;
    }

    /** Dumps indented text */
    void
    printIndented(std::string item, int depth)
    {
        while (depth--) {
            std::cout << "  ";
        }
        std::cout << item;
    }

    /** Creates osgEarth Geometry from KML geometry */
    Geometry*
    createGeometryFromElement(const kmldom::GeometryPtr kmlGeom)
    {
        switch (kmlGeom->Type()) 
        {
        case kmldom::Type_Point:
            {
                const kmldom::CoordinatesPtr coord = kmldom::AsPoint(kmlGeom)->get_coordinates();
                osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
                if (array) {
                    osgEarth::Symbology::PointSet* geom = new osgEarth::Symbology::PointSet(array);
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
                if (poly->has_outerboundaryis())
                {
                    const kmldom::CoordinatesPtr coord = poly->get_outerboundaryis()->get_linearring()->get_coordinates();

                    osg::Vec3dArray* array = CoordinatesToVec3dArray(coord);
                    if (array) {
                        osgEarth::Symbology::Polygon* geom = new osgEarth::Symbology::Polygon(array);

                        for (size_t i = 0; i < poly->get_innerboundaryis_array_size(); ++i)
                        {
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

    template <class T, class G>
    void
    getAltitudeMode(T* symbol, const G kmlGeom)
    {
        switch (kmlGeom->get_altitudemode())
        {
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

    template <class T, class G>
    void
    getExtrudeMode(T* symbol, const G kmlGeom)
    {
        symbol->extrude()->setExtrude(kmlGeom->get_extrude());
    }

    template <class T>
    T*
    createSymbol(const kmldom::GeometryPtr kmlGeom)
    {
        T* symbol = new T;

        {
            kmldom::PolygonPtr geom = kmldom::AsPolygon(kmlGeom);
            if (geom) {
                getAltitudeMode<T,kmldom::PolygonPtr>(symbol, geom);
                getExtrudeMode<T,kmldom::PolygonPtr>(symbol, geom);
                return symbol;
            }
        }

        {
            kmldom::LineStringPtr geom = kmldom::AsLineString(kmlGeom);
            if (geom) {
                getAltitudeMode<T,kmldom::LineStringPtr>(symbol, geom);
                getExtrudeMode<T,kmldom::LineStringPtr>(symbol, geom);
                return symbol;
            }
        }

        {
            kmldom::LinearRingPtr geom = kmldom::AsLinearRing(kmlGeom);
            if (geom) {
                getAltitudeMode<T,kmldom::LinearRingPtr>(symbol, geom);
                getExtrudeMode<T,kmldom::LinearRingPtr>(symbol, geom);
                return symbol;
            }
        }

        {
            kmldom::PointPtr geom = kmldom::AsPoint(kmlGeom);
            if (geom) {
                getAltitudeMode<T,kmldom::PointPtr>(symbol, geom);
                getExtrudeMode<T,kmldom::PointPtr>(symbol, geom);
                return symbol;
            }
        }

        {
            kmldom::ModelPtr geom = kmldom::AsModel(kmlGeom);
            if (geom) {
                getAltitudeMode<T,kmldom::ModelPtr>(symbol, geom);
                return symbol;
            }
        }

        return symbol;
    }

    /** Converts a KML color to an OSG color */
    osg::Vec4
    getColor(const kmlbase::Color32& color)
    {
        float r = color.get_red() * 1.0 / 255.0;
        float g = color.get_green() * 1.0 / 255.0;
        float b = color.get_blue() * 1.0 / 255.0;
        float a = color.get_alpha() * 1.0 / 255.0;
        return osg::Vec4(r,g,b,a);
    }

    /** Creates an osgEarth Style from a KML style. */
    Style*
    createStyle(kmldom::StylePtr kmlStyle, const kmldom::GeometryPtr kmlGeom)
    {
        osg::ref_ptr<Style> earthStyle = new Style;

        // labeling: todo: convert this to a text style.
        if (kmlStyle->has_labelstyle())
        {
            kmldom::LabelStylePtr kmls = kmlStyle->get_labelstyle();
            KMLLabelSymbol* s =  new KMLLabelSymbol;
            if (kmls->has_color())
            {
                // do not support yet random color
                //kmls->has_colormode()
                s->fill()->color() = getColor(kmls->get_color());
            }

            s->size() = DEFAULT_LABEL_SIZE;
            if (kmls->has_scale())
            {
                s->size() = s->size().value() * kmls->get_scale();
            }

            earthStyle->addSymbol(s);
        }

        // line style => LineSymbol
        if (kmlStyle->has_linestyle())
        {
            kmldom::LineStylePtr kmls = kmlStyle->get_linestyle();
            KMLLineSymbol* s =  createSymbol<KMLLineSymbol>(kmlGeom);
            if (kmls->has_color())
            {
                // do not support yet random color
                //kmls->has_colormode()
                s->stroke()->color() = getColor(kmls->get_color());
            }

            if (kmls->has_width())
            {
                s->stroke()->width() = kmls->get_width();
            }

            earthStyle->addSymbol(s);
        }

        // poly symbol => PolygonSymbol
        if (kmlStyle->has_polystyle())
        {
            kmldom::PolyStylePtr kmls = kmlStyle->get_polystyle();

            if ( !kmls->has_fill() || kmls->get_fill() == true )
            {
                PolygonSymbol* poly = createSymbol<KMLPolygonSymbol>( kmlGeom );
                earthStyle->addSymbol( poly );

                if ( kmls->has_color() )
                    poly->fill()->color() = getColor(kmls->get_color() );
            }

            else if ( kmls->has_outline() )
            {
                LineSymbol* line = createSymbol<KMLLineSymbol>( kmlGeom );
                earthStyle->addSymbol( line );

                if ( kmls->has_color() )
                    line->stroke()->color() = getColor( kmls->get_color() );
            }
        }

        if (kmlStyle->has_iconstyle())
        {
            kmldom::IconStylePtr kmls = kmlStyle->get_iconstyle();
            KMLIconSymbol* s = createSymbol<KMLIconSymbol>(kmlGeom);

            if (kmls->has_color()) {
                s->fill()->color() = getColor(kmls->get_color());
            }

            if (kmls->has_scale()) {
                s->size() = kmls->get_scale();
            }

            if (kmls->has_icon() && kmls->get_icon()->has_href()) {
                s->marker() = kmls->get_icon()->get_href();
            }

            earthStyle->addSymbol(s);
        }
        return earthStyle.release();
    }

    /** Generates a feature list from the data found in a KML file */
    void
    collectFeature(kmlengine::KmlFilePtr kml_file, 
                   FeatureList& fl, 
                   const kmldom::FeaturePtr& feature, 
                   int depth)
    {
        //TODO: maybe use "IsA()" here instead, so polymorphism will work
        switch (feature->Type())
        {
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
                if (placemark)
                {
                    Placemark* p = new Placemark;
                    // Create a resolved style for the Feature and print it.
                    kmldom::StylePtr kmlStyle = CreateResolvedStyle(feature, kml_file,kmldom::STYLESTATE_NORMAL);
                    //std::cout << kmldom::SerializePretty(style);
                    //osg::notify(osg::WARN) << "use style " << placemark->get_styleurl() << std::endl;
                    p->setName(placemark->get_name());
                    if (placemark->has_geometry())
                    {
                        if (placemark->get_geometry()->Type() == kmldom::Type_Model)
                        {
                            // specific case for model
                            KMLModelSymbol* modelSymbol = new KMLModelSymbol;
                            kmldom::ModelPtr kmlModel = kmldom::AsModel(placemark->get_geometry());
                            if (kmlModel->has_location())
                            {
                                kmldom::LocationPtr l = kmlModel->get_location();
                                modelSymbol->setLocation(osg::Vec3d(l->get_longitude(),
                                    l->get_latitude(),
                                    l->has_altitude()? l->get_altitude() : 0));
                            }
                            if (kmlModel->has_orientation())
                            {
                                kmldom::OrientationPtr l = kmlModel->get_orientation();
                                modelSymbol->setHeading(l->get_heading());
                                modelSymbol->setTilt(l->get_tilt());
                                modelSymbol->setRoll(l->get_roll());
                            }
                            if (kmlModel->has_scale())
                            {
                                kmldom::ScalePtr s = kmlModel->get_scale();
                                modelSymbol->setScale(osg::Vec3d(s->get_x(),
                                    s->get_y(),
                                    s->get_z()));
                            }

                            if (kmlModel->has_link())
                            {
                                modelSymbol->marker() = kmlModel->get_link()->get_href();
                                osg::notify(osg::NOTICE) << "Model link " << modelSymbol->marker().value() << std::endl;
                            }
                            else
                            {
                                osg::notify(osg::WARN) << "no link found on Model " << p->getName() << std::endl;
                            }

                            Style* style = new Style();
                            style->addSymbol(modelSymbol);
                            p->style() = style;
                            // empty geometry use symbol instead

                        }
                        
                        else
                        {
                            Geometry* geom = createGeometryFromElement(placemark->get_geometry());
                            if (geom)
                            {
                                p->setGeometry(geom);

                                Style* style = createStyle(kmlStyle, placemark->get_geometry());
                                if (style)
                                    p->style() = style;

                            } 
                            
                            else
                            {
                                osg::notify(osg::WARN) << "cant retrieve geometry for placemark " << p->getName() << std::endl;
                            }
                        }

                        KMLLabelSymbol* label = p->style().get()->getSymbol<KMLLabelSymbol>();
                        if (!label)
                        {
                            label = new KMLLabelSymbol;
                            p->style().get()->addSymbol(label);
                            label->size() = DEFAULT_LABEL_SIZE;
                        }
                        label->content() = p->getName();
                        osg::notify(osg::NOTICE) << "label " << label->content().value() << std::endl;

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

        if (feature->has_name())
        {
            std::cout << " " << feature->get_name();
        }
        std::cout << std::endl;

        if (const kmldom::ContainerPtr container = kmldom::AsContainer(feature))
        {
            for (size_t i = 0; i < container->get_feature_array_size(); ++i)
            {
                collectFeature(kml_file, fl, container->get_feature_array_at(i), depth+1);
            }
        }
    }
}

//------------------------------------------------------------------------

#if 0
KMLFeatureCursor::KMLFeatureCursor(const FeatureList& featureList) :
_featureList(featureList) 
{
    _iterator = _featureList.begin();
}

bool
KMLFeatureCursor::hasMore() const
{
    if (_iterator != _featureList.end())
        return true;
    return false;
}

Feature*
KMLFeatureCursor::nextFeature()
{
    if (!hasMore())
        return 0;
    osgEarth::Features::Feature* f = *_iterator;
    _iterator++;
    return f;
}
#endif

FeatureCursor*
KMLFeatureSource::createFeatureCursor( const Query& query )
{
    return new FeatureListCursor( this->_features );
    //return new KMLFeatureCursor(_features);
}

//------------------------------------------------------------------------

KMLFeatureSource::KMLFeatureSource(const KMLFeatureSourceOptions& options) :
FeatureSource(options), _options(options)
{
	//nop
}

FeatureProfile*
KMLFeatureSource::createFeatureProfile()
{
  return new FeatureProfile(
      osgEarth::Registry::instance()->getGlobalGeodeticProfile()->getExtent() );
}

//override
void
KMLFeatureSource::initialize( const std::string& referenceURI )
{
    if (!_features.empty())
        return;

    if ( _options.url().isSet() )
    {
        _url = osgEarth::getFullPath( referenceURI, _options.url().value() );
    }

    std::string file = _url;

    FeatureList features;

    // Read it.
    std::string kml;
    if ( HTTPClient::readString( file, kml ) != HTTPClient::RESULT_OK )
    {
        OE_WARN << LC << file << " read failed" << std::endl;
        return;
    }

    std::string errors;
    kmlengine::KmlFilePtr kml_file = kmlengine::KmlFile::CreateFromParse(kml, &errors);
    if (!kml_file)
    {
        OE_WARN << LC << errors << std::endl;
        return;
    }

    // Parse it.
    errors.clear();
    kmldom::ElementPtr root = kmldom::Parse(kml, &errors);
    if (!root)
    {
        OE_WARN << LC << errors << std::endl;
        return;
    }

    const kmldom::FeaturePtr feature = getRootFeature(root);

    if (feature)
    {
        collectFeature(kml_file, features, feature, 0);
    }
    else
    {
        OE_WARN << LC << "No root feature" << std::endl;
    }

    _features = features;
}

//------------------------------------------------------------------------

class KMLFeatureSourceFactory : public FeatureSourceDriver
{
public:
    KMLFeatureSourceFactory()
    {
        supportsExtension( "osgearth_feature_kml", "KML feature driver for Godzi" );
    }

    virtual const char* className()
    {
        return "KML Feature Reader";
    }

    virtual ReadResult readObject(const std::string& file_name, const Options* options) const
    {
        if ( !acceptsExtension(osgDB::getLowerCaseFileExtension( file_name )))
            return ReadResult::FILE_NOT_HANDLED;

        return ReadResult( new KMLFeatureSource( getFeatureSourceOptions(options) ) );
    }
};

REGISTER_OSGPLUGIN(osgearth_feature_kml, KMLFeatureSourceFactory)
