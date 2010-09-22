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
#include <Godzi/Features/AGGLiteRasterizerTileSource>
#include <osgEarthFeatures/FeatureTileSource>
#include <osgEarthFeatures/TransformFilter>
#include <osgEarthFeatures/BufferFilter>
#include <osgEarthSymbology/Style>
#include <osgEarthSymbology/GeometrySymbol>
#include <osgEarthSymbology/Geometry>
#include <osgEarth/Registry>

//TODO: replace this with ImageRasterizer
#include <osgEarthSymbology/AGG.h>
#include <osg/Notify>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <sstream>


using namespace Godzi;
using namespace Godzi::Features;

AGGLiteRasterizerTileSource::AGGLiteRasterizerTileSource(const Godzi::Features::GeometryList& geomList) : _geomList(geomList) 
{
    osg::ref_ptr<osgEarth::Symbology::Style> style = new osgEarth::Symbology::Style;
    style->setName("Lines");
    osg::ref_ptr<osgEarth::Symbology::LineSymbol> symbol = new osgEarth::Symbology::LineSymbol;
    symbol->stroke()->color() = osg::Vec4(0,1,1,1);
    symbol->stroke()->width() = 3.0;
    style->addSymbol(symbol.get());
    _style = style;
}

//override
osg::Referenced* AGGLiteRasterizerTileSource::createBuildData()
{
    return new BuildData();
}

//override
bool AGGLiteRasterizerTileSource::preProcess(osg::Image* image, osg::Referenced* buildData)
{
    agg::rendering_buffer rbuf( image->data(), image->s(), image->t(), image->s()*4 );
    agg::renderer<agg::span_abgr32> ren(rbuf);
    ren.clear(agg::rgba8(0,0,0,0));
    return true;
}

//override
bool AGGLiteRasterizerTileSource::renderFeaturesForStyle(
    const Symbology::Style* style,
    osgEarth::Features::FeatureList& features,
    osg::Referenced* buildData,
    const GeoExtent& imageExtent,
    osg::Image* image )
{

    BuildData* bd = static_cast<BuildData*>( buildData );

    // A processing context to use with the filters:
    osgEarth::Features::FilterContext context;
    //context.profile() = getFeatureSource()->getFeatureProfile();
    osgEarth::Features::FeatureProfile* profile = new osgEarth::Features::FeatureProfile( osgEarth::Registry::instance()->getGlobalGeodeticProfile()->getExtent() );

    context.profile() = profile;
    const osgEarth::Symbology::LineSymbol* line = style->getSymbol<osgEarth::Symbology::LineSymbol>();

    // initialize:
    double xmin = imageExtent.xMin();
    double ymin = imageExtent.yMin();
    double s = (double)image->s();
    double t = (double)image->t();
    double xf = (double)image->s() / imageExtent.width();
    double yf = (double)image->t() / imageExtent.height();

    if (line) {
        osgEarth::Features::BufferFilter buffer;
        buffer.capStyle() = line->stroke()->lineCap().value();
        bool relativeLine = true;
        if (relativeLine) {
            double ratio = 1.0/xf;
            buffer.distance() = ratio * line->stroke()->width().value();
        } else {
            buffer.distance() = line->stroke()->width().value();
        }
        context = buffer.push( features, context );
    }

    // First, transform the features into the map's SRS:
#if 0
    osgEarth::Features::TransformFilter xform( imageExtent.getSRS() );
    xform.setLocalizeCoordinates( false );
    context = xform.push( features, context );
#endif

    // set up the AGG renderer:
    agg::rendering_buffer rbuf( image->data(), image->s(), image->t(), image->s()*4 );

    // Create the renderer and the rasterizer
    agg::renderer<agg::span_abgr32> ren(rbuf);
    agg::rasterizer ras;

    // Setup the rasterizer
    ras.gamma(1.3);
    ras.filling_rule(agg::fill_even_odd);

    GeoExtent cropExtent = GeoExtent(imageExtent);
    cropExtent.scale(1.1, 1.1);

    osg::ref_ptr<Symbology::Polygon> cropPoly = new Symbology::Polygon( 4 );
    cropPoly->push_back( osg::Vec3d( cropExtent.xMin(), cropExtent.yMin(), 0 ));
    cropPoly->push_back( osg::Vec3d( cropExtent.xMax(), cropExtent.yMin(), 0 ));
    cropPoly->push_back( osg::Vec3d( cropExtent.xMax(), cropExtent.yMax(), 0 ));
    cropPoly->push_back( osg::Vec3d( cropExtent.xMin(), cropExtent.yMax(), 0 ));

    double lineWidth = 1.0;
    if (line)
        lineWidth = (double)line->stroke()->width().value();

    osg::Vec4 color = osg::Vec4(1, 1, 1, 1);
    if (line) {
        color = line->stroke()->color();
    }
    // render the features
    for(osgEarth::Features::FeatureList::iterator i = features.begin(); i != features.end(); i++)
    {
        bool first = bd->_pass == 0 && i == features.begin();

        osg::ref_ptr< osgEarth::Symbology::Geometry > croppedGeometry;
        if ( ! i->get()->getGeometry()->crop( cropPoly.get(), croppedGeometry ) )
            continue;

        osg::Vec4 c = color;
        unsigned int a = 127+(c.a()*255)/2; // scale alpha up
        agg::rgba8 fgColor( c.r()*255, c.g()*255, c.b()*255, a );

        osgEarth::Symbology::GeometryIterator gi( croppedGeometry.get() );
        while( gi.hasMore() )
        {
            c = color;
            osgEarth::Symbology::Geometry* g = gi.next();
            if (g->getType() == osgEarth::Symbology::Geometry::TYPE_POLYGON) {
                const osgEarth::Symbology::PolygonSymbol* symbol = style->getSymbol<osgEarth::Symbology::PolygonSymbol>();
                if (symbol)
                    c = symbol->fill()->color();
            } else if (g->getType() == osgEarth::Symbology::Geometry::TYPE_RING || g->getType() == osgEarth::Symbology::Geometry::TYPE_LINESTRING) {
                const osgEarth::Symbology::LineSymbol* symbol = style->getSymbol<osgEarth::Symbology::LineSymbol>();
                if (symbol)
                    c = symbol->stroke()->color();
            }

            a = 127+(c.a()*255)/2; // scale alpha up
            fgColor = agg::rgba8( c.r()*255, c.g()*255, c.b()*255, a );

            ras.filling_rule( agg::fill_even_odd );
            for( osgEarth::Symbology::Geometry::iterator p = g->begin(); p != g->end(); p++ )
            {
                const osg::Vec3d& p0 = *p;
                double x0 = xf*(p0.x()-xmin);
                double y0 = yf*(p0.y()-ymin);

                const osg::Vec3d& p1 = p+1 != g->end()? *(p+1) : g->front();
                double x1 = xf*(p1.x()-xmin);
                double y1 = yf*(p1.y()-ymin);

                if ( p == g->begin() )
                    ras.move_to_d( x0, y0 );
                else
                    ras.line_to_d( x0, y0 );
            }
        }
        ras.render(ren, fgColor);
        ras.reset();
    }

    bd->_pass++;
    return true;            
}



osg::Image*
AGGLiteRasterizerTileSource::createImage( const TileKey* key, ProgressCallback* progress )
{
    // if ( !_features.valid() || !_features->getFeatureProfile() )
    //     return 0L;

    // implementation-specific data
    osg::ref_ptr<osg::Referenced> buildData = createBuildData();

    // allocate the image.
    osg::ref_ptr<osg::Image> image = new osg::Image();
    image->allocateImage( getPixelsPerTile(), getPixelsPerTile(), 1, GL_RGBA, GL_UNSIGNED_BYTE );

    preProcess( image.get(), buildData.get() );

    // Each feature has its own embedded style data, so use that:
    for (int i = 0; i < _geomList.size(); ++i) {
        osgEarth::Features::FeatureList list;
        osgEarth::Features::Feature* f = new osgEarth::Features::Feature;
        list.push_back(f);
        const Geometry* geom = _geomList[i].get();
        if (geom) {
            
            osgEarth::Symbology::Geometry* g = 0;
            switch (geom->getType()) {
            case Geometry::TYPE_LINESTRING:
            {
                g = osgEarth::Symbology::Geometry::create(osgEarth::Symbology::Geometry::TYPE_LINESTRING, geom->getCoordinates());
            }
                break;
            case Geometry::TYPE_LINEARRING:
            {
                g = osgEarth::Symbology::Geometry::create(osgEarth::Symbology::Geometry::TYPE_RING, geom->getCoordinates());
            }
                break;
            }
            if (g) {
                f->setGeometry(g);


                renderFeaturesForStyle( _style.get(), list, buildData.get(),
                                        key->getGeoExtent(), image.get() );
            }
        }
    }

    // final tile processing after all styles are done
    postProcess( image.get(), buildData.get() );

    return image.release();
}


//override
bool AGGLiteRasterizerTileSource::postProcess( osg::Image* image, osg::Referenced* data )
{
		//convert from ABGR to RGBA
		unsigned char* pixel = image->data();
		for(int i=0; i<image->s()*image->t()*4; i+=4, pixel+=4)
		{
        std::swap( pixel[0], pixel[3] );
        std::swap( pixel[1], pixel[2] );
		}
    return true;
}
