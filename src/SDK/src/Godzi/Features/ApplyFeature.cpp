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


static osg::Group* createPlacemarkSymbology()
{
    osg::ref_ptr<osgEarth::Symbology::GeometryContent> content = new osgEarth::Symbology::GeometryContent;
    osg::Vec3dArray* array = new osg::Vec3dArray;
    array->push_back(osg::Vec3d(0, 0, 0));
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


// we have map node put our feature here
void ApplyFeature::apply(osgEarth::MapNode& node)
{

    for (FeatureList::iterator it = _features.begin(); it != _features.end(); ++it) {
        Godzi::Features::Placemark* p = dynamic_cast<Godzi::Features::Placemark*>(it->get());
        if (p) {
            osg::Matrixd matrix;
            node.getEllipsoidModel()->computeLocalToWorldTransformFromLatLongHeight(p->getLat(), p->getLong(), p->getAlt(), matrix);
            osg::notify(osg::NOTICE) << p->getName() << " Lat " << osg::RadiansToDegrees( p->getLat()) << " long " << osg::RadiansToDegrees(p->getLong()) << " alt " << p->getAlt() << std::endl;
            osg::MatrixTransform* transform = new osg::MatrixTransform;
            transform->setMatrix(matrix);

            transform->addChild(createPlacemarkSymbology());
#if 0
            {
                osg::Vec3dArray* array = new osg::Vec3dArray;
                array->push_back(osg::Vec3d(0,0,0));
                osg::ref_ptr<osgEarth::Symbology::Geometry> geom = osgEarth::Symbology::Geometry::create(Geometry::TYPE_POINTSET, array);
                
            }
#endif
            node.addChild(transform);
        }
    }
}
