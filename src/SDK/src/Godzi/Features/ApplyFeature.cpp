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

#include <osg/MatrixTransform>
#include <osg/AutoTransform>
#include <Godzi/Features/ApplyFeature>
#include <Godzi/Features/Placemark>
#include <osgEarthSymbology/Content>
#include <osgEarthSymbology/Style>
#include <osgEarthSymbology/SymbolicNode>
#include <osgEarthSymbology/MarkerSymbol>
#include <osgEarthSymbology/MarkerSymbolizer>

#include <osgEarthSymbology/GeometrySymbolizer>

using namespace Godzi;
using namespace Godzi::Features;

typedef osgEarth::Symbology::SymbolicNode< osgEarth::Symbology::State<osgEarth::Symbology::GeometryContent> > GeometrySymbolicNode;


ApplyFeature::ApplyFeature() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
void ApplyFeature::setFeatures(const FeatureList& features) { _features = features; }

void ApplyFeature::apply(osg::CoordinateSystemNode& node)
{
    osgEarth::MapNode* mapNode = dynamic_cast<osgEarth::MapNode*> ( &node);
    if (mapNode)
        apply(*mapNode);
}


static osg::Group* createPlacemarkPointSymbology()
{
    osg::ref_ptr<osgEarth::Symbology::GeometryContent> content = new osgEarth::Symbology::GeometryContent;
    osg::Vec3dArray* array = new osg::Vec3dArray;
    array->push_back(osg::Vec3d(0,0,0));
    content->getGeometryList().push_back(osgEarth::Symbology::Geometry::create(osgEarth::Symbology::Geometry::TYPE_POINTSET, array));

    osg::ref_ptr<osgEarth::Symbology::Style> style = new osgEarth::Symbology::Style;
    style->setName("Marker");
    osg::ref_ptr<osgEarth::Symbology::MarkerSymbol> pointSymbol = new osgEarth::Symbology::MarkerSymbol;
    pointSymbol->marker() = "../../data/marker.osg";
    style->addSymbol(pointSymbol.get());

    GeometrySymbolicNode* node = new GeometrySymbolicNode();
    node->setSymbolizer( new osgEarth::Symbology::MarkerSymbolizer() );
    node->getState()->setStyle(style.get());
    node->getState()->setContent(content.get());

    osg::AutoTransform* tr = new osg::AutoTransform;
    {
    tr->addChild(node);
    tr->setAutoScaleToScreen(true);
    tr->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
    }

    return tr;
}


static osg::Group* createPlacemarkLineStringSymbology(osg::Vec3dArray* array)
{
    osg::ref_ptr<osgEarth::Symbology::GeometryContent> content = new osgEarth::Symbology::GeometryContent;
    content->getGeometryList().push_back(osgEarth::Symbology::Geometry::create(osgEarth::Symbology::Geometry::TYPE_LINESTRING, array));

    osg::ref_ptr<osgEarth::Symbology::Style> style = new osgEarth::Symbology::Style;
    style->setName("Lines");
    osg::ref_ptr<osgEarth::Symbology::LineSymbol> symbol = new osgEarth::Symbology::LineSymbol;
    symbol->stroke()->color() = osg::Vec4(1,0,1,1);
    symbol->stroke()->width() = 3.0;
    style->addSymbol(symbol.get());

    GeometrySymbolicNode* node = new GeometrySymbolicNode();
    node->setSymbolizer( new osgEarth::Symbology::GeometrySymbolizer() );
    node->getState()->setStyle(style.get());
    node->getState()->setContent(content.get());

    return node;
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


// we have map node put our feature here
void ApplyFeature::apply(osgEarth::MapNode& node)
{

    for (FeatureList::iterator it = _features.begin(); it != _features.end(); ++it) {
        Godzi::Features::Placemark* p = dynamic_cast<Godzi::Features::Placemark*>(it->get());
        if (p->getGeometry()) {

            switch(p->getGeometry()->getType()) {
            case Geometry::TYPE_POINT:
            {
                if (p->getGeometry()->getCoordinates()->size() > 0) {
                    double lon, lat, alt;
                    lon = (*p->getGeometry()->getCoordinates())[0][0];
                    lat = (*p->getGeometry()->getCoordinates())[0][1];
                    alt = (*p->getGeometry()->getCoordinates())[0][2];
                    osg::Matrixd matrix;
                    node.getEllipsoidModel()->computeLocalToWorldTransformFromLatLongHeight(lat, lon, alt, matrix);

                    osg::notify(osg::NOTICE) << p->getName() << " Lat " << osg::RadiansToDegrees( lat) << " long " << osg::RadiansToDegrees(lon) << " alt " << alt << std::endl;
                    osg::MatrixTransform* transform = new osg::MatrixTransform;
                    transform->setMatrix(matrix);
                    transform->addChild(createPlacemarkPointSymbology());
                    node.addChild(transform);
                } else {
                    osg::notify(osg::WARN) << "no point in placemark " << p->getName() << std::endl;
                }
            }
            break;
            case Geometry::TYPE_LINESTRING:
            {
                if (p->getGeometry()->getCoordinates()->size() > 0) {
                    osg::Vec3dArray* array = ConvertFromLongitudeLatitudeAltitudeTo3D(node.getEllipsoidModel(), p->getGeometry()->getCoordinates());
                    if (array) {
                        node.addChild(createPlacemarkLineStringSymbology(array));
                    } else {
                        osg::notify(osg::WARN) << "can't convert LineString from placemark " << p->getName() << " original number of coordinates: " << p->getGeometry()->getCoordinates()->size() << std::endl;
                    }
                        
                } else {
                    osg::notify(osg::WARN) << "no lines in placemark " << p->getName() << std::endl;
                }
            }
            break;
            case Geometry::TYPE_UNKNOWN:
            {
                osg::Vec3dArray* position3d = ConvertFromLongitudeLatitudeAltitudeTo3D(node.getEllipsoidModel(), p->getGeometry()->getCoordinates());
            }
            break;

            }
        }
    }
}
