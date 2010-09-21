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

#include <Godzi/Features/Symbolizer>
#include <Godzi/Features/Symbol>
#include <osgEarth/ModelSource>
#include <osgEarth/Registry>
#include <osgEarth/Map>

#include <osgEarthFeatures/FeatureSymbolizer>
#include <osgEarthFeatures/FeatureModelSource>
#include <osgEarthFeatures/TransformFilter>
#include <osgEarthSymbology/MarkerSymbol>
#include <osgEarthSymbology/MarkerSymbolizer>
#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/Content>
#include <osg/AutoTransform>
#include <osg/MatrixTransform>

#include <osgUtil/Tessellator>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/Geode>

using namespace Godzi;
using namespace Godzi::Features;

typedef osgEarth::Symbology::SymbolicNode< osgEarth::Symbology::State<osgEarth::Symbology::GeometryContent> > GeometrySymbolicNode;

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

osg::Node* KMLGeometrySymbolizer::KMLGeometrySymbolizerOperator::operator()(const osgEarth::Symbology::GeometryList& geometryList, const osgEarth::Symbology::Style* style)
{

    osg::ref_ptr<osg::Group> group = new osg::Group;

    for (int i = 0; i < geometryList.size(); ++i) {
        const osgEarth::Symbology::Geometry* geom = geometryList[i].get(); 
        osg::ref_ptr<osg::Geometry> osgGeom = new osg::Geometry;
        osg::PrimitiveSet::Mode primMode;
        osg::Vec4 color = osg::Vec4(1.0, 0.0, 1.0, 1.);
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;

        switch( geom->getType())
        {
        case osgEarth::Symbology::Geometry::TYPE_POINTSET:
        {
            const KMLPointSymbol* point = dynamic_cast<const KMLPointSymbol*>(style->getSymbol<osgEarth::Symbology::PointSymbol>());
            if (point)
            {
                osg::MatrixTransform* m = new osg::MatrixTransform;
                m->setMatrix(osg::Matrix::translate(osg::Vec3((*geom)[0])));
                m->addChild(createPlacemarkPointSymbology());
                group->addChild(m);
                return group.release();
            }
        }
        break;

        case osgEarth::Symbology::Geometry::TYPE_LINESTRING:
        {
            primMode = osg::PrimitiveSet::LINE_STRIP;
            const KMLLineSymbol* line = dynamic_cast<const KMLLineSymbol*>(style->getSymbol<osgEarth::Symbology::LineSymbol>());
            if (line)
            {
                color = line->stroke()->color();
                float size = line->stroke()->width().value();
                osgGeom->getOrCreateStateSet()->setAttributeAndModes( new osg::LineWidth(size));
            }
        }
        break;

        case osgEarth::Symbology::Geometry::TYPE_RING:
        {
            primMode = osg::PrimitiveSet::LINE_LOOP;
            const KMLLineSymbol* line = dynamic_cast<const KMLLineSymbol*>(style->getSymbol<osgEarth::Symbology::LineSymbol>());
            if (line)
            {
                color = line->stroke()->color();
                float size = line->stroke()->width().value();
                osgGeom->getOrCreateStateSet()->setAttributeAndModes( new osg::LineWidth(size));
            }
        }
        break;

        case osgEarth::Symbology::Geometry::TYPE_POLYGON:
        {
            primMode = osg::PrimitiveSet::LINE_LOOP; // loop will tessellate into polys
            const KMLPolygonSymbol* poly = dynamic_cast<const KMLPolygonSymbol*>(style->getSymbol<osgEarth::Symbology::PolygonSymbol>());
            if (poly)
            {
                color = poly->fill()->color();
            }
        }
        break;
        }

        osg::Material* material = new osg::Material;
        material->setDiffuse(osg::Material::FRONT_AND_BACK, color);

        if ( geom->getType() == osgEarth::Symbology::Geometry::TYPE_POLYGON && static_cast<const osgEarth::Symbology::Polygon*>(geom)->getHoles().size() > 0 )
        {
            const osgEarth::Symbology::Polygon* poly = static_cast<const osgEarth::Symbology::Polygon*>(geom);
            int totalPoints = poly->getTotalPointCount();
            osg::Vec3Array* allPoints = new osg::Vec3Array( totalPoints );
            int offset = 0;
            for( osgEarth::Symbology::RingCollection::const_iterator h = poly->getHoles().begin(); h != poly->getHoles().end(); ++h )
            {
                osgEarth::Symbology::Geometry* hole = h->get();
                std::copy( hole->begin(), hole->end(), allPoints->begin() + offset );
                osgGeom->addPrimitiveSet( new osg::DrawArrays( primMode, offset, hole->size() ) );
                offset += hole->size();
            }
            osgGeom->setVertexArray( allPoints );
        }
        else
        {
            osgGeom->setVertexArray( geom->toVec3Array() );
            osgGeom->addPrimitiveSet( new osg::DrawArrays( primMode, 0, geom->size() ) );
        }

        // tessellate all polygon geometries. Tessellating each geometry separately
        // with TESS_TYPE_GEOMETRY is much faster than doing the whole bunch together
        // using TESS_TYPE_DRAWABLE.

        if ( geom->getType() == osgEarth::Symbology::Geometry::TYPE_POLYGON)
        {
            osgUtil::Tessellator tess;
            tess.setTessellationType( osgUtil::Tessellator::TESS_TYPE_GEOMETRY );
            tess.setWindingType( osgUtil::Tessellator::TESS_WINDING_POSITIVE );
            tess.retessellatePolygons( *osgGeom );
        }
        osgGeom->getOrCreateStateSet()->setAttributeAndModes(material);
        geode->addDrawable(osgGeom);

        if (geode->getNumDrawables())
            group->addChild(geode);
    }
    if (group->getNumChildren())
        return group.release();
    return 0;
}

#if 0
bool
KMLGeometrySymbolizer::compile(GeometrySymbolizerState* state,
                                osg::Group* attachPoint)
{
    if ( !state || !state->getContent() || !attachPoint || !state->getStyle() )
        return false;

    const GeometryList& geometryList = state->getContent()->getGeometryList();

    KMLGeometrySymbolizerOperator functor;
    osg::Node* node = (functor)(geometryList, state->getStyle());
    if (node)
    {
        attachPoint->removeChildren(0, attachPoint->getNumChildren());
        attachPoint->addChild(node);
        return true;
    }

    return false;
}
#endif





class FactoryKMLSymbolizer : public osgEarth::Features::SymbolizerFactory
{
protected:
    osg::ref_ptr<osgEarth::Features::FeatureModelSource> _model;

public:
    FactoryKMLSymbolizer(osgEarth::Features::FeatureModelSource* model) : _model(model) {}
    osgEarth::Features::FeatureModelSource* getFeatureModelSource() { return _model.get(); }
    //override

    virtual osg::Node* createNodeForStyle(
        const osgEarth::Symbology::Style* style,
        const osgEarth::Features::FeatureList& features,
        osgEarth::Features::FeatureSymbolizerContext* context,
        osg::Node** out_newNode)
    {

        // break the features out into separate lists for geometries and text annotations:
        osgEarth::Features::FeatureList geomFeatureList, textAnnoList;

        for (osgEarth::Features::FeatureList::const_iterator it = features.begin(); it != features.end(); ++it)
        {
            osgEarth::Features::Feature* f = osg::clone((*it).get(),osg::CopyOp::DEEP_COPY_ALL);
            geomFeatureList.push_back( f );
        }

        // a single group to hold the results:
        osg::Group* root = new osg::Group;

        // compile the geometry features:
        if ( geomFeatureList.size() > 0 )
        {
            osg::Node* node = compileGeometries( geomFeatureList, style );
            if ( node ) root->addChild( node );
        }

        // set the output node if necessary:
        if ( out_newNode )
            *out_newNode = root;

        return root;
    }

    osg::Node*
    compileGeometries( osgEarth::Features::FeatureList& features, const osgEarth::Symbology::Style* style )
    {
        // A processing context to use with the filters:
        osgEarth::Features::FilterContext contextFilter;
        contextFilter.profile() = _model->getFeatureSource()->getFeatureProfile();

        // Transform them into the map's SRS:
        osgEarth::Features::TransformFilter xform( _model->getMap()->getProfile()->getSRS() );
        xform.setMakeGeocentric( _model->getMap()->isGeocentric() );
        xform.setLocalizeCoordinates( true );

        // Apply the height offset if necessary:
        double height = 1.0;
        xform.setHeightOffset( height);
        contextFilter = xform.push( features, contextFilter );

        // Assemble the geometries into a list:
        osgEarth::Symbology::GeometryList geometryList;
        for (osgEarth::Features::FeatureList::iterator it = features.begin(); it != features.end(); ++it)
        {
            osgEarth::Features::Feature* feature = it->get();
            if ( feature )
            {
                osgEarth::Symbology::Geometry* geometry = feature->getGeometry();
                if ( geometry )
                    geometryList.push_back(geometry);
            }
        }

        // build the geometry.
        KMLGeometrySymbolizer::KMLGeometrySymbolizerOperator geometryOperator;
        osg::Node* result = geometryOperator( geometryList, style );
        
        // install the localization transform if necessary.
        if ( contextFilter.hasReferenceFrame() )
        {
            osg::MatrixTransform* delocalizer = new osg::MatrixTransform( contextFilter.inverseReferenceFrame() );
            delocalizer->addChild( result );
            result = delocalizer;
        }

        return result;
    }
};




FeatureKMLModelSource::FeatureKMLModelSource( const PluginOptions* options ) : osgEarth::Features::FeatureModelSource( options )
{
}

//override
void FeatureKMLModelSource::initialize( const std::string& referenceURI, const osgEarth::Map* map )
{
    osgEarth::Features::FeatureModelSource::initialize( referenceURI, map );
}

osg::Node* FeatureKMLModelSource::createNode( osgEarth::ProgressCallback* progress )
{
    return new osgEarth::Features::FeatureSymbolizerGraph(new FactoryKMLSymbolizer(this));
}


