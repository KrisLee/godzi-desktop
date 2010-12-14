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
#include <Godzi/KML/KMLParser>

#include <osgEarth/Registry>
#include <osgEarth/FileUtils>

#include <osgDB/FileNameUtils>

using namespace Godzi::KML;


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

void
KMLFeatureSource::initialize( const std::string& referenceURI )
{
    if (!_features.empty())
        return;

    if ( _options.url().isSet() )
    {
        _url = osgEarth::getFullPath( referenceURI, _options.url().value() );

        KMLParser parser;
        parser.parse( _url, _features );
    }
}

FeatureCursor*
KMLFeatureSource::createFeatureCursor( const Query& query )
{
    return new FeatureListCursor( this->_features );
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
