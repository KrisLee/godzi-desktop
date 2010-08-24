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

#include <Godzi/Features/SymbolicNode>
#include <osg/Material>
#include <osg/LineWidth>
#include <osg/MatrixTransform>
#include <osg/AutoTransform>

using namespace Godzi;
using namespace Godzi::Features;


static osg::Node* getNode(const std::string& str)
{
#if OSG_VERSION_LESS_THAN(2,9,8)
    osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
    options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);
    osg::Node* node = osgDB::readNodeFile(str, options.get());
    return node;
#else
    osg::ref_ptr<osgDB::Options> options = new osgDB::Options;
    options->setObjectCacheHint(osgDB::Options::CACHE_ALL);
    osg::Node* node = osgDB::readNodeFile(str, options.get());
    return node;
#endif
}


static osg::Vec3dArray* ConvertFromLongitudeLatitudeAltitudeTo3D(osg::EllipsoidModel* elipse, osg::Vec3dArray* longLatAlt)
{
    if (!longLatAlt || longLatAlt->empty() || !elipse)
        return 0;

    osg::Vec3dArray* a3d = new osg::Vec3dArray(longLatAlt->size());
    for (int i = 0; i < longLatAlt->size(); ++i) {
        double x,y,z;
        const osg::Vec3d& l = (*longLatAlt)[i];
        elipse->convertLatLongHeightToXYZ( l[1], l[0], l[2],
                                           x, y, z);
        (*a3d)[i] = osg::Vec3d(x,y,z);
    }
    return a3d;
}


osg::Group* PlacemarkSymbolizer::PlacemarkSymbolizerOperator::createMarker(const Placemark* placemark, const osgEarth::Symbology::MarkerSymbol* symbol, PlacemarkContext* context)
{
    if (symbol)
    {
        const Point* geom = dynamic_cast<const Point*>(placemark->getGeometry());
        if (geom && symbol && geom->getCoordinates()->size() && !symbol->marker().value().empty())
        {
            osg::Node* node = getNode(symbol->marker().value());
            if (!node) {
                osg::notify(osg::WARN) << "can't load Marker Node " << symbol->marker().value() << std::endl;
                return 0;
            }
            osg::Group* group = new osg::Group;
            for ( osg::Vec3dArray::const_iterator it = geom->getCoordinates()->begin(); it != geom->getCoordinates()->end(); ++it)
            {
                osg::Vec3d pos = *it;
                osg::MatrixTransform* tr = new osg::MatrixTransform;

                double lon = (*it)[0];
                double lat = (*it)[1];
                double alt = (*it)[2];
                switch (geom->getAltitudeMode()) {
                case Point::ClampToGround:
                    alt = 0;
                    break;
                case Point::RelativeToGround:
                    break;
                case Point::Absolute:
                {
                    osg::Vec3d dir = context->getMapNode()->getEllipsoidModel()->computeLocalUpVector(lat, lon, alt);
                    osg::Vec3d posOnGlobe;
                    context->getMapNode()->getEllipsoidModel()->convertLatLongHeightToXYZ(lat, lon, 0, posOnGlobe[0], posOnGlobe[1], posOnGlobe[2]);
                    alt = alt - posOnGlobe.length();
                }
                break;
                }
                osg::notify(osg::NOTICE) << placemark->getName() << " Lat " << osg::RadiansToDegrees( lat) << " long " << osg::RadiansToDegrees(lon) << " alt " << alt << std::endl;
                osg::Matrixd matrix;
                context->getMapNode()->getEllipsoidModel()->computeLocalToWorldTransformFromLatLongHeight(lat, lon, alt, matrix);

                osg::AutoTransform* autoTransform = new osg::AutoTransform;
                autoTransform->addChild(node);
                autoTransform->setAutoScaleToScreen(true);
                autoTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);

                tr->setMatrix(matrix);
                tr->addChild(autoTransform);
                group->addChild(tr);

                if (geom->getExtrude()) {
                    osg::Vec3d start;
                    osg::Vec3d end;
                    context->getMapNode()->getEllipsoidModel()->convertLatLongHeightToXYZ(lat, lon, 0, start[0], start[1], start[2]);
                    context->getMapNode()->getEllipsoidModel()->convertLatLongHeightToXYZ(lat, lon, alt, end[0], end[1], end[2]);
                    osg::Geometry* line = new osg::Geometry;
                    osg::Vec3dArray* a = new osg::Vec3dArray;
                    a->push_back(start);
                    a->push_back(end);
                    line->setVertexArray(a);
                    line->getOrCreateStateSet()->setAttributeAndModes(new osg::Material);
                    line->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(2.0));
                    line->getPrimitiveSetList().push_back(new osg::DrawArrays(GL_LINES, 0, 2));
                    osg::Geode* geode = new osg::Geode;
                    geode->addDrawable(line);
                    group->addChild(geode);
                }
            }
            return group;
        }
    }
    return 0;
}



osg::Node* PlacemarkSymbolizer::PlacemarkSymbolizerOperator::operator()(const Placemark* placemark,const osgEarth::Symbology::Style* style,PlacemarkContext* context)
{
    switch(placemark->getGeometry()->getType()) {
    case Geometry::TYPE_POINT:
    {
        const osgEarth::Symbology::MarkerSymbol* symbol = style->getSymbol<const osgEarth::Symbology::MarkerSymbol>();
        PlacemarkContext* ctx = dynamic_cast<PlacemarkContext*>(context);
        if (ctx && symbol)
            return createMarker(placemark, symbol, ctx);
    }
    break;

#if 0
    case Geometry::TYPE_LINESTRING:
    {
        const osgEarth::Symbology::MarkerSymbol* symbol = style->getSymbol<const osgEarth::Symbology::MarkerSymbol*>();
        PlacemarkContext* ctx = dynamic_cast<PlacemarkContext*>(context);
        if (ctx && symbol)
            return createMarker(placemark, symbol, ctx);
    }
    break;
#endif

    }
    return 0;
}




PlacemarkSymbolizer::PlacemarkSymbolizer()
{
    //nop
}

bool
PlacemarkSymbolizer::compile(PlacemarkState* state,
                             osg::Group* attachPoint)
{
    if ( !state || !state->getContent() || !attachPoint || !state->getStyle() )
        return false;

    const Placemark* placemark = state->getContent()->getPlacemark();
    PlacemarkContext* context = dynamic_cast<PlacemarkContext*>(state->getContext());

    if (!context) {
        osg::notify(osg::WARN) << "context is a mandatory for PlacemarkSymbolizer" << std::endl;
        return false;
    }
    PlacemarkSymbolizerOperator functor;
    osg::Node* node = (functor)(placemark, state->getStyle(), context);
    if (node)
    {
        attachPoint->removeChildren(0, attachPoint->getNumChildren());
        attachPoint->addChild(node);
        return true;
    }

    return false;
}
