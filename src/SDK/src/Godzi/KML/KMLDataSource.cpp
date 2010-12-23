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
#include <Godzi/KML/KMLDataSource>
#include <Godzi/KML/KMLFeatureSource>
#include <Godzi/KML/KMLActions>
#include <osgEarthDrivers/agglite/AGGLiteOptions>

using namespace Godzi;
using namespace Godzi::KML;

namespace
{
    const std::string EMPTY_STRING ="";
}

const std::string KMLDataSource::TYPE_KML = "KML"; // static initializer


KMLDataSource::KMLDataSource(const KMLFeatureSourceOptions& opt, bool visible)
: DataSource(visible)
{
	osgEarth::Config config = opt.toConfig();
	_opt = KMLFeatureSourceOptions(osgEarth::ConfigOptions(config));
}

KMLDataSource::KMLDataSource(const KMLFeatureSourceOptions& opt, bool visible, KMLFeatureSource* source)
: DataSource(visible), _fs(source)
{
	osgEarth::Config config = opt.toConfig();
	_opt = KMLFeatureSourceOptions(osgEarth::ConfigOptions(config));
}

KMLDataSource::KMLDataSource(const Godzi::Config& conf)
: DataSource(conf)
{
	_opt = KMLFeatureSourceOptions(osgEarth::ConfigOptions(conf.child("options")));
}

Godzi::Config
KMLDataSource::toConfig() const
{
	Godzi::Config conf = DataSource::toConfig();
	conf.add("type", TYPE_KML);
	conf.add("options", _opt.getConfig());

  return conf;
}

const std::string&
KMLDataSource::getLocation() const
{
	return _opt.url().isSet() && _opt.url()->size() > 0 ? _opt.url().get() : EMPTY_STRING;
}

void
KMLDataSource::populate()
{
    if ( _features.empty() )
    {
        osg::ref_ptr<KMLFeatureSource> fs = new KMLFeatureSource( _opt );
        fs->initialize();

        osg::ref_ptr<FeatureCursor> cursor = fs->createFeatureCursor();
        while( cursor->hasMore() )
        {
            Feature* f = cursor->nextFeature();
            if ( f )
            {
                _features.push_back( f );
                _featureMap[ f->getFID() ] = f;
            }
        }
    }
}

bool
KMLDataSource::getDataObjectSpecs( DataObjectSpecVector& out_results ) const
{
    const_cast<KMLDataSource*>(this)->populate();

    out_results.clear();
    for( FeatureList::const_iterator i = _features.begin(); i != _features.end(); ++i )
    {
        Feature* f = i->get();
        out_results.push_back( DataObjectSpec( f->getFID(), f->getName() ) );
    }
    return true;
}

bool
KMLDataSource::getDataObjectActionSpecs( DataObjectActionSpecVector& out_actionSpecs ) const
{
    out_actionSpecs.push_back( new DataObjectActionSpec<ZoomToKmlObjectAction>(
        "Locate",
        "Moves the camera to look at the object",
        true ) );

    return true;
}

Feature*
KMLDataSource::getFeature( int objectUID ) const
{
    const_cast<KMLDataSource*>(this)->populate();

    FeaturesById::const_iterator i = _featureMap.find( objectUID );
    return i != _featureMap.end() ? i->second : 0L;
}

osgEarth::ModelLayer*
KMLDataSource::createModelLayer() const
{
    //TODO: come back to this later - use for 
    return 0L;
}

osgEarth::ImageLayer*
KMLDataSource::createImageLayer() const
{
    std::string name = _name.isSet() ? _name.get() : "KML Source";

    osgEarth::Drivers::AGGLiteOptions options;
    options.featureOptions() = _opt;
    //options.optimizeLineSampling() = false;

    osgEarth::ImageLayerOptions layerOptions( name, options );
    layerOptions.cacheEnabled() = false;

    return new osgEarth::ImageLayer( layerOptions );
}

DataSource*
KMLDataSource::clone() const
{
	// [jas] Following shouldn't be necessary, but the TMSOptions copy
	// constructor does not appear to be working correctly.
	KMLFeatureSourceOptions cOpt;
	if (_opt.url().isSet())
		cOpt.url() = _opt.url().get();

	KMLDataSource* c = new KMLDataSource(cOpt, true, _fs);
	if (_name.isSet())
		c->name() = _name;

	if (_id.isSet())
		c->setId(_id.get());
	
	c->setError(_error);
	c->setErrorMsg(_errorMsg);

	return c;
}

//------------------------------------------------------------------------

bool
KMLDataSourceFactory::canCreate(const Godzi::Config &config)
{
	osgEarth::optional<std::string> type;
	if (config.key().compare("datasource") == 0 && config.getIfSet<std::string>("type", type) && type.get() == KMLDataSource::TYPE_KML)
			return true;

	return false;
}

DataSource*
KMLDataSourceFactory::createDataSource(const Godzi::Config& config)
{
	if (!canCreate(config))
		return 0L;

	return new KMLDataSource(config);
}

