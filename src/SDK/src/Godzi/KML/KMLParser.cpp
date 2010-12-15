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
#include <Godzi/KML/KMLParser>
#include <Godzi/KML/KMLSymbol>
#include <Godzi/Placemark>
#include <osgEarth/HTTPClient>
#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/GeometrySymbol>
#include <osgEarthUtil/Viewpoint>

using namespace Godzi;
using namespace Godzi::KML;
using namespace osgEarth::Util;

#define LC "[Godzi.KMLParser] "
#define DEFAULT_LABEL_SIZE 32

namespace
{
    /** Converts a KML "abstract view" into an osgEarth::Util::Viewpoint. */
    bool
    s_parseView( const kmldom::AbstractViewPtr& av, Viewpoint& out )
    {
        const kmldom::LookAt* lookAt = dynamic_cast<kmldom::LookAt*>( av.get() );
        if ( lookAt )
        {
            if ( !lookAt->has_latitude() || !lookAt->has_longitude() )
                return false;

            out.setFocalPoint( osg::Vec3d(
                lookAt->get_longitude(),
                lookAt->get_latitude(),
                lookAt->has_altitude() ? lookAt->get_altitude() : 0L ) );

            if ( lookAt->has_heading() )
                out.setHeading( lookAt->get_heading() );

            if ( lookAt->has_tilt() )
                out.setPitch( lookAt->get_tilt() - 90.0 );

            out.setRange( lookAt->has_range() ? lookAt->get_range() : 10000.0 );

            return true;
        }
        return false;
    }

    /** Finds the root feature in a KML document */
    const kmldom::FeaturePtr
    s_getRootFeature(const kmldom::ElementPtr& root) 
    {
        const kmldom::KmlPtr kml = kmldom::AsKml(root);
        if (kml && kml->has_feature()) {
            return kml->get_feature();
        }
        return kmldom::AsFeature(root);
    }

    /** Converts a KML coordinate array to an OSG coordinate array */
    osg::Vec3dArray*
    s_kmlCoordinatesToVec3dArray(kmldom::CoordinatesPtr coords)
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
    s_printIndented(std::string item, int depth)
    {
        while (depth--) {
            std::cout << "  ";
        }
        std::cout << item;
    }

    /** Creates osgEarth Geometry from KML geometry */
    Geometry*
    s_createGeometryFromElement(const kmldom::GeometryPtr kmlGeom)
    {
        switch (kmlGeom->Type()) 
        {
        case kmldom::Type_Point:
            {
                const kmldom::CoordinatesPtr coord = kmldom::AsPoint(kmlGeom)->get_coordinates();
                osg::Vec3dArray* array = s_kmlCoordinatesToVec3dArray(coord);
                if (array) {
                    PointSet* geom = new PointSet(array);
                    return (geom);
                }
            }
            break;

        case kmldom::Type_LineString:
            {
                const kmldom::CoordinatesPtr coord = kmldom::AsLineString(kmlGeom)->get_coordinates();
                osg::Vec3dArray* array = s_kmlCoordinatesToVec3dArray(coord);
                if (array) {
                    LineString* geom = new LineString(array);
                    return (geom);
                }
            }
            break;

        case kmldom::Type_LinearRing:
            {
                const kmldom::CoordinatesPtr coord = kmldom::AsLinearRing(kmlGeom)->get_coordinates();
                osg::Vec3dArray* array = s_kmlCoordinatesToVec3dArray(coord);
                if (array) {
                    Ring* geom = new Ring(array);
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

                    osg::Vec3dArray* array = s_kmlCoordinatesToVec3dArray(coord);
                    if (array) {
                        Polygon* geom = new Polygon(array);

                        for (size_t i = 0; i < poly->get_innerboundaryis_array_size(); ++i)
                        {
                            const kmldom::CoordinatesPtr inner = poly->get_innerboundaryis_array_at(i)->get_linearring()->get_coordinates();
                            if (inner)
                                geom->getHoles().push_back(new Ring(s_kmlCoordinatesToVec3dArray(inner)));
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
                        Geometry* newgeom = s_createGeometryFromElement(g);
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
    s_getAltitudeMode(T* symbol, const G kmlGeom)
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
    s_getExtrudeMode(T* symbol, const G kmlGeom)
    {
        symbol->extrude()->setExtrude(kmlGeom->get_extrude());
    }

    template <class T>
    T*
    s_createSymbol(const kmldom::GeometryPtr kmlGeom)
    {
        T* symbol = new T;

        {
            kmldom::PolygonPtr geom = kmldom::AsPolygon(kmlGeom);
            if (geom) {
                s_getAltitudeMode<T,kmldom::PolygonPtr>(symbol, geom);
                s_getExtrudeMode<T,kmldom::PolygonPtr>(symbol, geom);
                return symbol;
            }
        }

        {
            kmldom::LineStringPtr geom = kmldom::AsLineString(kmlGeom);
            if (geom) {
                s_getAltitudeMode<T,kmldom::LineStringPtr>(symbol, geom);
                s_getExtrudeMode<T,kmldom::LineStringPtr>(symbol, geom);
                return symbol;
            }
        }

        {
            kmldom::LinearRingPtr geom = kmldom::AsLinearRing(kmlGeom);
            if (geom) {
                s_getAltitudeMode<T,kmldom::LinearRingPtr>(symbol, geom);
                s_getExtrudeMode<T,kmldom::LinearRingPtr>(symbol, geom);
                return symbol;
            }
        }

        {
            kmldom::PointPtr geom = kmldom::AsPoint(kmlGeom);
            if (geom) {
                s_getAltitudeMode<T,kmldom::PointPtr>(symbol, geom);
                s_getExtrudeMode<T,kmldom::PointPtr>(symbol, geom);
                return symbol;
            }
        }

        {
            kmldom::ModelPtr geom = kmldom::AsModel(kmlGeom);
            if (geom) {
                s_getAltitudeMode<T,kmldom::ModelPtr>(symbol, geom);
                return symbol;
            }
        }

        return symbol;
    }

    /** Converts a KML color to an OSG color */
    osg::Vec4
    s_getColor(const kmlbase::Color32& color)
    {
        float r = color.get_red() * 1.0 / 255.0;
        float g = color.get_green() * 1.0 / 255.0;
        float b = color.get_blue() * 1.0 / 255.0;
        float a = color.get_alpha() * 1.0 / 255.0;
        return osg::Vec4(r,g,b,a);
    }

    /** Creates an osgEarth Style from a KML style. */
    Style*
    s_createStyle(kmldom::StylePtr kmlStyle, const kmldom::GeometryPtr kmlGeom)
    {
        Style* earthStyle = new Style();

        // labeling: todo: convert this to a text style.
        if (kmlStyle->has_labelstyle())
        {
            kmldom::LabelStylePtr kmls = kmlStyle->get_labelstyle();
            KMLLabelSymbol* s =  new KMLLabelSymbol;
            if (kmls->has_color())
            {
                // do not support yet random color
                //kmls->has_colormode()
                s->fill()->color() = s_getColor(kmls->get_color());
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
            KMLLineSymbol* s =  s_createSymbol<KMLLineSymbol>(kmlGeom);
            if (kmls->has_color())
            {
                // do not support yet random color
                //kmls->has_colormode()
                s->stroke()->color() = s_getColor(kmls->get_color());
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
                PolygonSymbol* poly = s_createSymbol<KMLPolygonSymbol>( kmlGeom );
                earthStyle->addSymbol( poly );

                if ( kmls->has_color() )
                    poly->fill()->color() = s_getColor(kmls->get_color() );
            }

            else if ( kmls->has_outline() )
            {
                LineSymbol* line = s_createSymbol<KMLLineSymbol>( kmlGeom );
                earthStyle->addSymbol( line );

                if ( kmls->has_color() )
                    line->stroke()->color() = s_getColor( kmls->get_color() );
            }
        }

        if (kmlStyle->has_iconstyle())
        {
            kmldom::IconStylePtr kmls = kmlStyle->get_iconstyle();
            KMLIconSymbol* s = s_createSymbol<KMLIconSymbol>(kmlGeom);

            if (kmls->has_color()) {
                s->fill()->color() = s_getColor(kmls->get_color());
            }

            if (kmls->has_scale()) {
                s->size() = kmls->get_scale();
            }

            if (kmls->has_icon() && kmls->get_icon()->has_href()) {
                s->marker() = kmls->get_icon()->get_href();
            }
            else {
                s->marker() = "http://demo.pelicanmapping.com/rmweb/godzi_marker.png";
            }

            earthStyle->addSymbol(s);
        }
        return earthStyle;
    }
}

//------------------------------------------------------------------------

KMLParser::KMLParser() :
_depth( 0 ),
_nextUID( 0L )
{
    //NOP
}

bool
KMLParser::parse( const std::string& location, FeatureList& out_results )
{    
    _depth = -1;
    _contextStack.push( ParserContext(out_results, _nextUID) );
    bool ok = parseLocation( location );
    _contextStack.pop();
    return ok;
}

bool
KMLParser::parseLocation( const std::string& location )
{
    OE_INFO << LC << "KML: Reading from: " << location << std::endl;

    // Read it.
    std::string content;
    if ( HTTPClient::readString( location, content ) != HTTPClient::RESULT_OK )
    {
        OE_WARN << LC << location << ": read failed" << std::endl;
        return false;
    }

    std::string errors;
    kmlengine::KmlFilePtr kmlFile = kmlengine::KmlFile::CreateFromParse(content, &errors);
    if (!kmlFile)
    {
        OE_WARN << LC << errors << std::endl;
        return false;
    }

    // Parse it.
    errors.clear();
    kmldom::ElementPtr root = kmldom::Parse(content, &errors);
    if (!root)
    {
        OE_WARN << LC << errors << std::endl;
        return false;
    }

    const kmldom::FeaturePtr rootKmlFeature = s_getRootFeature(root);

    if ( rootKmlFeature )
    {
        ++_depth;
        _contextStack.push( ParserContext( context(), kmlFile ) );
        parseFeature( rootKmlFeature );
        _contextStack.pop();
        --_depth;
    }
    else
    {
        OE_WARN << LC << "No root feature" << std::endl;
        return false;
    }
}

bool
KMLParser::parseFeature(const kmldom::FeaturePtr& kmlFeature)
{
    switch( kmlFeature->Type() )
    {
    case kmldom::Type_Document:
        // NYI
        break;

    case kmldom::Type_Folder:
        // NYI
        break;

    case kmldom::Type_GroundOverlay:
        // NYI
        break;

    case kmldom::Type_PhotoOverlay:
        // NYI
        break;

    case kmldom::Type_NetworkLink:
        parseNetworkLink( kmldom::AsNetworkLink(kmlFeature) );
        break;

    case kmldom::Type_Placemark:
        parsePlacemark( kmldom::AsPlacemark(kmlFeature) );
        break;

    case kmldom::Type_ScreenOverlay:
        // NYI
        break;

    default:
        break;
    }

    if ( kmlFeature->has_name() )
    {
        OE_INFO << " " << kmlFeature->get_name() << std::endl;
    }

    // parse children.
    if ( const kmldom::ContainerPtr container = kmldom::AsContainer(kmlFeature) )
    {
        ++_depth;
        for (size_t i = 0; i < container->get_feature_array_size(); ++i)
        {
            parseFeature( container->get_feature_array_at(i) );
        }
        --_depth;
    }

    return true;
}

bool
KMLParser::parseNetworkLink(const kmldom::NetworkLinkPtr& networkLink)
{
    if ( networkLink.get() && networkLink->has_link() )
    {
        const kmldom::LinkPtr link = networkLink->get_link();
        if ( link->has_href() )
        {
            parseLocation( link->get_href() );
        }
    }
    return true;
}

bool
KMLParser::parsePlacemark(const kmldom::PlacemarkPtr& kmlPlacemark)
{
    if ( !kmlPlacemark.get() )
        return false;

    Placemark* p = new Placemark( context()._nextUID++ );

    // resolve the style attached to this feature:
    kmldom::StylePtr kmlStyle = kmlengine::CreateResolvedStyle( kmlPlacemark, context()._kmlFile, kmldom::STYLESTATE_NORMAL );

    p->setName( kmlPlacemark->get_name() );

    if ( kmlPlacemark->has_geometry() )
    {
        if (kmlPlacemark->get_geometry()->Type() == kmldom::Type_Model)
        {
            // specific case for model
            KMLModelSymbol* modelSymbol = new KMLModelSymbol;
            kmldom::ModelPtr kmlModel = kmldom::AsModel( kmlPlacemark->get_geometry() );
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
            }
            else
            {
                OE_WARN << LC << "no link found on Model " << p->getName() << std::endl;
            }

            Style* style = new Style();
            style->addSymbol( modelSymbol );
            p->style() = style;
            // no geometry for the model.
        }

        else // regular geometry.
        {
            Geometry* geom = s_createGeometryFromElement( kmlPlacemark->get_geometry() );
            if (geom)
            {
                p->setGeometry(geom);

                Style* style = s_createStyle(kmlStyle, kmlPlacemark->get_geometry());
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

        s_printIndented("Placemark", _depth);

        // See if the placemark has a "lookat" location:
        if ( kmlPlacemark->has_abstractview() && kmlPlacemark->get_abstractview()->IsA( kmldom::Type_LookAt ) )
        {
            Viewpoint vp;
            if( s_parseView( kmlPlacemark->get_abstractview(), vp ) )
                p->lookAt() = vp;
        }

        context()._results.push_back( p );
    }

    return true;
}
