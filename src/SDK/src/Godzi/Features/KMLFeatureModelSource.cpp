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

#include <Godzi/Features/KMLFeatureModelSource>
#include <Godzi/Features/KMLGeometrySymbolizer>
#include <Godzi/Features/Symbol>
#include <osgEarthFeatures/FeatureModelSource>
#include <osgEarthFeatures/FeatureSymbolizer>
#include <osgEarthFeatures/TransformFilter>
#include <osg/MatrixTransform>
#include <osgText/Text>
#include <osgEarthFeatures/BuildTextOperator>
#include <osg/Depth>

using namespace Godzi;
using namespace Godzi::Features;


struct CullPlaneCallback : public osg::NodeCallback
{
    osg::Vec3d _n;

    CullPlaneCallback( const osg::Vec3d& planeNormal ) : _n(planeNormal) {
        _n.normalize();
    }

    void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        if (nv && nv->getEyePoint() * _n <= 0) {
            return;
        }
        traverse(node,nv);
    }
};

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
        // check if it's a model
        if (style) {
            const KMLModelSymbol* model = style->getSymbol<KMLModelSymbol>();
            if (model) {
                osg::Node* node = osgDB::readNodeFile(model->marker().value());
                const osg::EllipsoidModel* em = _model->getMap()->getProfile()->getSRS()->getEllipsoid();
                if (em) {
                    osg::Matrix matrix;
                    double lat = model->getLocation()[0];
                    double lon = model->getLocation()[1];
                    double alt = model->getLocation()[2];
                    em->computeLocalToWorldTransformFromLatLongHeight(lat, lon, alt, matrix);
                    osg::MatrixTransform* mtr = new osg::MatrixTransform;
                    mtr->setMatrix(matrix);
                    mtr->addChild(node);
                    return mtr;
                }
            }
        }

        // A processing context to use with the filters:
        osgEarth::Features::FilterContext contextFilter;
        contextFilter.profile() = _model->getFeatureSource()->getFeatureProfile();

        // Transform them into the map's SRS:
        osgEarth::Features::TransformFilter xform( _model->getMap()->getProfile()->getSRS() );
        xform.setMakeGeocentric( _model->getMap()->isGeocentric() );
        xform.setLocalizeCoordinates( true );

        // Apply the height offset if necessary:
        double height = 0.0;
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


            const KMLLabelSymbol* sym = style->getSymbol<KMLLabelSymbol>();
            if (sym) {
                if (sym->content().isSet()) {
                    osgText::Text* text = new osgText::Text;
                    text->setCharacterSizeMode( osgText::TextBase::SCREEN_COORDS );
                    text->setText(sym->content().value());

                    std::string font = "fonts/arial.ttf";
                    if (sym->font().isSet() && !sym->font().get().empty())
                    {
                        font = sym->font().value();
                    }

                    text->setFont( font );
                    text->setCharacterSize(sym->size().value());
                    text->setAlignment( osgText::TextBase::CENTER_CENTER );
                    text->getOrCreateStateSet()->setAttributeAndModes( new osg::Depth(osg::Depth::ALWAYS), osg::StateAttribute::ON );
                    text->getOrCreateStateSet()->setRenderBinDetails( 99999, "RenderBin" );
                    text->setAutoRotateToScreen( true );
                    osg::Vec4f textColor = sym->fill()->color();
                    text->setColor( textColor);
                    osg::Vec4f haloColor = sym->halo()->color();
        
                    text->setBackdropColor( haloColor );
                    text->setBackdropType( osgText::Text::OUTLINE );
                    osg::Geode* geode = new osg::Geode;
                    geode->addDrawable(text);
                    delocalizer->addChild(geode);
                }
            }

            delocalizer->setCullCallback( new CullPlaneCallback( osg::Vec3(0,0,0) * contextFilter.inverseReferenceFrame() ) );
        }

#if 0
        const KMLLabelSymbol* sym = style->getSymbol<KMLLabelSymbol>();
        if (sym) {
            osgEarth::Features::BuildTextOperator textOperator;
            osg::Node* text = textOperator(features, sym, contextFilter);
            if (text) {
                osg::Group* grp = new osg::Group;
                grp->addChild(text);
                grp->addChild(result);
                result = grp;
            }
        }
#endif

        return result;
    }
};


KMLFeatureModelSource::KMLFeatureModelSource( const osgEarth::DriverConfigOptions& options ) : osgEarth::Features::FeatureModelSource( options )
{
}

//override
void KMLFeatureModelSource::initialize( const std::string& referenceURI, const osgEarth::Map* map )
{
    osgEarth::Features::FeatureModelSource::initialize( referenceURI, map );
}

osg::Node* KMLFeatureModelSource::createNode( osgEarth::ProgressCallback* progress )
{
    return new osgEarth::Features::FeatureSymbolizerGraph(new FactoryKMLSymbolizer(this));
}


