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
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include <Godzi/Common>
#include <Godzi/Project>
#include <Godzi/Application>
#include <Godzi/Earth>
#include <osgEarth/XmlUtils>
#include <osgEarth/MapNode>
#include <osgEarth/Map>
#include <iterator>

using namespace Godzi;

//---------------------------------------------------------------------------

ProjectProperties::ProjectProperties() :
_name( "Untitled" )
{
    //nop
}

ProjectProperties::ProjectProperties( const Godzi::Config& conf )
{
    conf.getIfSet( "name", _name );
		conf.getIfSet( "map", _map);
		conf.getIfSet( "visibleimagelayers", _visibleImageLayers );
		conf.getIfSet( "visisblemodellayers", _visibleModelLayers );
}

Godzi::Config
ProjectProperties::toConfig() const
{
		Godzi::Config conf;
    conf.addIfSet( "name", _name );
		conf.addIfSet( "map", _map );
		conf.addIfSet( "visibleImageLayers", _visibleImageLayers );
		conf.addIfSet( "visibleModelLayers", _visibleModelLayers );

    return conf;
}

//---------------------------------------------------------------------------

Project::Project(osgEarth::Map* map, const Config& conf, const std::string& mapLocation)
: _map(map)
{
		_props = ProjectProperties( conf.child( "properties" ) );
		if (!mapLocation.empty())
			_props.map() = mapLocation;

		if (!map)
			_map = new osgEarth::Map();

		setVisibleLayers();

		osgEarth::ConfigSet sources = conf.children("datasource");

		for (osgEarth::ConfigSet::const_iterator it = sources.begin(); it != sources.end(); ++it)
		{
			DataSourceFactory* factory = Application::dataSourceFactoryManager->getFactory(*it);
			if (factory)
			{
				DataSource* source = factory->createDataSource(*it);
				if (source)
					_sources.push_back(source);
			}
			else
			{
				osgEarth::optional<std::string> type;
				printf("[Godzi::Project] Skipping data source of unknown type \"%s\"\n", (*it).getIfSet("type", type) ? type.get().c_str() : "");
			}
		}
}

Godzi::Config
Project::toConfig()
{
	  Godzi::Config conf( "godzi_project" );

		updateVisibleLayers();

    conf.add( "properties", _props.toConfig() );
		for (int i=0; i < _sources.size(); i++)
			conf.add(_sources[i]->toConfig());

    return conf;
}

void
Project::addDataSource(Godzi::DataSource* source)
{
	if (!source)
		return;

	_sources.push_back(source);

	dirty();
	emit dataSourceAdded(source, _sources.size() - 1);
}

void
Project::removeDataSource(Godzi::DataSource* source)
{
	std::vector<osg::ref_ptr<Godzi::DataSource>>::iterator pos;
	for (pos = _sources.begin(); pos != _sources.end(); ++pos)
	{
		if ((*pos)->getLocation() == source->getLocation())
			break;
	}
	if (pos != _sources.end())
		_sources.erase(pos);

	dirty();
	emit dataSourceRemoved(source);
}

bool
Project::updateDataSource(Godzi::DataSource* source, bool dirtyProject, Godzi::DataSource** out_old)
{
	for (int i=0; i < _sources.size(); i++)
	{
		if (_sources[i].get()->getLocation().compare(source->getLocation()) == 0)
		{
			if (out_old)
				*out_old = _sources[i].get();

			_sources[i] = source;

			if (dirtyProject)
				dirty();
			emit dataSourceUpdated(source);

			return true;
		}
	}

	return false;
}

void
Project::moveDataSource(Godzi::DataSource* source, int position)
{
	if (position < 0)
		return;

	int found = -1;
	for (int i=0; i < _sources.size(); i++)
	{
		if (_sources[i].get()->getLocation().compare(source->getLocation()) == 0)
		{
			if (i == position)
				return;
			
			found = i;
			break;
		}
	}

	if (found != -1)
		_sources.erase(_sources.begin() + found);

	if (position > _sources.size())
		_sources.push_back(source);
	else
		_sources.insert(_sources.begin() + position, source);

	dirty();
	emit dataSourceMoved(source, position);
}

void
Project::setVisibleLayers()
{
	if (!_map.valid())
		return;

	if (_props.visibleImageLayers().isSet())
	{
		std::vector<std::string> visibleLayers = Godzi::csvToVector(_props.visibleImageLayers().get());

		osgEarth::ImageLayerVector imageLayers;
		_map->getImageLayers(imageLayers);
		for (osgEarth::ImageLayerVector::iterator it = imageLayers.begin(); it != imageLayers.end(); ++it)
		{
			(*it)->setEnabled(std::find(visibleLayers.begin(), visibleLayers.end(), (*it)->getName()) != visibleLayers.end());
		}
	}
}

void
Project::updateVisibleLayers()
{
	std::string visibleImages = "";
	osgEarth::ImageLayerVector imageLayers;
	_map->getImageLayers(imageLayers);
	for (osgEarth::ImageLayerVector::const_iterator it = imageLayers.begin(); it != imageLayers.end(); ++it)
		if ((*it)->getEnabled())
			visibleImages += visibleImages.length() == 0 ? (*it)->getName() : "," + (*it)->getName();

	if (visibleImages.length() > 0)
		_props.visibleImageLayers() = visibleImages;


	std::string visibleModels;
	osgEarth::ModelLayerVector modelLayers;
	_map->getModelLayers(modelLayers);
	for (osgEarth::ModelLayerVector::const_iterator it = modelLayers.begin(); it != modelLayers.end(); ++it)
		if ((*it)->getEnabled())
			visibleModels += visibleModels.length() == 0 ? (*it)->getName() : "," + (*it)->getName();

	if (visibleModels.length() > 0)
		_props.visibleModelLayers() = visibleModels;
}

//---------------------------------------------------------------------------

bool
NewProjectAction::doAction( void* sender, Application* app )
{
		app->setProject(new Godzi::Project(_map.get()));
    return true;
}

//---------------------------------------------------------------------------

bool
OpenProjectAction::doAction( void* sender, Application* app )
{    
    std::ifstream input( _location.c_str() );
    osg::ref_ptr<osgEarth::XmlDocument> doc = osgEarth::XmlDocument::load( input );
    if ( doc.valid() )
    {
				Config conf = doc->getConfig().child( "godzi_project" );

				osgEarth::MapNode* mapNode = 0L;
				ProjectProperties props = ProjectProperties( conf.child( "properties" ) );
				if (props.map().isSet())
					mapNode = Godzi::readEarthFile(props.map().get());

				Project* project = new Project( mapNode ? mapNode->getMap() : _defaultMap.get(), conf );
        app->setProject( project, _location );

				return true;
    }
    else
    {
        return setError( "Failed to read project XML document" );
    }
}

//---------------------------------------------------------------------------

bool
SaveProjectAction::doAction( void* sender, Application* app )
{
    std::string location = !_location.empty() ? _location : app->getProjectLocation();

    if ( location.empty() || app->getProject() == 0L )
        return setError( "Illegal: empty project location" );

    osg::ref_ptr<osgEarth::XmlDocument> doc = new osgEarth::XmlDocument( app->getProject()->toConfig() );
    if ( !doc.valid() )
        return setError( "Failed to serialize project to XML" );

    std::ofstream output( location.c_str() );
    if ( !output.is_open() )
        return setError( "Failed to open output file for writing" );

    doc->store( output );

    app->setProjectClean();
    return true;
}

//---------------------------------------------------------------------------

bool
UpdateProjectPropertiesAction::doAction( const ActionContext& ac, Application* app )
{
    Project* project = app->getProject();
    if ( !project )
        return setError( "Illegal: no active project" );

    // save the old one
    _oldProps = project->getProperties();

    // ..and apply the new one.
    project->setProperties( _newProps );
    project->dirty();

    return true;
}

bool
UpdateProjectPropertiesAction::undoAction( const ActionContext& ac, Application* app )
{
    Project* project = app->getProject();
    if ( !project )
        return setError( "Illegal: no active project" );

    project->setProperties( _oldProps );
    project->dirty();

    return true;
}

//---------------------------------------------------------------------------
