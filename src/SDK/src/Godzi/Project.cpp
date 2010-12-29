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
#include <Godzi/DataSources>
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

Project::Project(osgEarth::Map* defaultMap, const Godzi::Config& conf)
: _currentUID(0), _map(defaultMap)
{
		_props = ProjectProperties( conf.child( "properties" ) );
		if (_props.map().isSet())
		{
			osgEarth::MapNode* mapNode = Godzi::readEarthFile(_props.map().get());
			if (mapNode)
				_map = mapNode->getMap();
		}

		if (!_map.valid())
			_map = new osgEarth::Map();

		_baseLayerOffset = _map->getNumImageLayers();
		setVisibleLayers();

		osgEarth::ConfigSet sources = conf.children("datasource");

		for (osgEarth::ConfigSet::const_iterator it = sources.begin(); it != sources.end(); ++it)
		{
			DataSourceFactory* factory = Application::dataSourceFactoryManager->getFactory(*it);

			if (factory)
			{
				DataSource* source = factory->createDataSource(*it);

				if (source)
				{
					if (!source->id().isSet())
						source->setId(getUID());

					addSource(source);
				}
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

		for (int i=0; i < _sourceLayers.size(); i++)
			if (_sourceLayers[i].valid())
				conf.add(_sourceLayers[i].source->toConfig());

    return conf;
}

void
Project::addDataSource(osg::ref_ptr<Godzi::DataSource> source)
{
	if (!source)
		return;

	if (!source->id().isSet())
		source->setId(getUID());

	addSource(source);

  dirty();
	emit dataSourceAdded(source, _sourceLayers.size() - 1);
}

int
Project::removeDataSource(Godzi::DataSource* source)
{
	int index = removeSource(source);
	if (index >= 0)
	{
		dirty();
		emit dataSourceRemoved(source);
	}

	return index;
}

bool
Project::updateDataSource(Godzi::DataSource* source, bool dirtyProject, Godzi::DataSource** out_old)
{
	if (!source || !source->id().isSet())
		return false;

	int layerIndex = removeSource(source, out_old);
	if (layerIndex >= 0)
	{
		addSource(source, layerIndex);

		if (dirtyProject)
			dirty();

		emit dataSourceUpdated(source);

		return true;
	}

	return false;
}

int
Project::moveDataSource(Godzi::DataSource* source, int position)
{
	if (!source || !source->id().isSet() || position < 0)
		return -1;

	int layerIndex = findSourceLayersIndex(source->id().get());
	if (layerIndex >= 0 && layerIndex != position)
	{
		SourcedLayers layers = _sourceLayers[layerIndex];
		osgEarth::ImageLayer* imageLayer = layers.imageLayer.get();
		osgEarth::ModelLayer* modelLayer = layers.modelLayer.get();

		int mapIndex = position + _baseLayerOffset;

		if (imageLayer)
			_map->moveImageLayer(imageLayer, mapIndex);

		if (modelLayer)
			_map->moveModelLayer(modelLayer, mapIndex);

		_sourceLayers.erase(_sourceLayers.begin() + layerIndex);

		if (_sourceLayers.size() <= position)
			_sourceLayers.resize(position + 1);

		if (!_sourceLayers[position].valid())
			_sourceLayers[position] = layers;
		else
			_sourceLayers.insert(_sourceLayers.begin() + position, layers);

		dirty();
		emit dataSourceMoved(source, position);
	}

	return layerIndex;
}

bool
Project::toggleDataSource(unsigned int id, bool visible)
{
	int layerIndex = findSourceLayersIndex(id);
	if (layerIndex >= 0)
	{
		SourcedLayers layers = _sourceLayers[layerIndex];
		
		if (layers.source->visible() == visible)
			return false;

		layers.source->setVisible(visible);

		osgEarth::ImageLayer* imageLayer = layers.imageLayer.get();
		if (imageLayer)
			imageLayer->setEnabled(visible);

		osgEarth::ModelLayer* modelLayer = layers.modelLayer.get();
		if (modelLayer)
			modelLayer->setEnabled(visible);
		
		dirty();
		emit dataSourceToggled(id, visible);

		return true;
	}

	return false;
}

void
Project::getSources(Godzi::DataSourceVector& out_list) const
{
	out_list.reserve(_sourceLayers.size());

	for (std::vector<SourcedLayers>::const_iterator it = _sourceLayers.begin(); it != _sourceLayers.end(); ++it)
	{
		if (it->valid())
			out_list.push_back(it->source.get());
	}
}

int
Project::getNumSources() const
{
	return _sourceLayers.size();
}

unsigned int
Project::getUID()
{
	return _currentUID++;
}

//void
//Project::setMap(osgEarth::Map* map)
//{
//	if (!map)
//		return;
//
//	osg::ref_ptr<osgEarth::Map> oldMap = _map;
//}

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

void
Project::addSource(osg::ref_ptr<Godzi::DataSource> source, int index)
{
	if (!source.valid())
		return;

	osgEarth::ImageLayer* imageLayer = createImageLayer(source, index);
	osgEarth::ModelLayer* modelLayer = createModelLayer(source, index);
	SourcedLayers layers(source, imageLayer, modelLayer);

	if (index >= 0)
	{
		if (_sourceLayers.size() <= index)
			_sourceLayers.resize(index + 1);

		if (!_sourceLayers[index].valid())
			_sourceLayers[index] = layers;
		else
			_sourceLayers.insert(_sourceLayers.begin() + index, layers);
	}
	else
	{
		_sourceLayers.push_back(layers);
	}
}

int
Project::removeSource(Godzi::DataSource* source, Godzi::DataSource** out_removed)
{
	if (!source || !source->id().isSet())
		return -1;

	int layerIndex = findSourceLayersIndex(source->id().get());
	if (layerIndex >= 0)
	{
		SourcedLayers layers = _sourceLayers[layerIndex];
		osgEarth::ImageLayer* imageLayer = layers.imageLayer.get();
		osgEarth::ModelLayer* modelLayer = layers.modelLayer.get();

		if (imageLayer)
			_map->removeImageLayer(imageLayer);

		if (modelLayer)
			_map->removeModelLayer(modelLayer);

		if (out_removed)
			*out_removed = layers.source.get();

		_sourceLayers.erase(_sourceLayers.begin() + layerIndex);
	}

	return layerIndex;
}


osgEarth::ImageLayer*
Project::createImageLayer(osg::ref_ptr<const Godzi::DataSource> source, int index)
{
	osgEarth::ImageLayer* layer = source->createImageLayer();
	if (layer)
	{
		layer->setEnabled(source->visible());

		int mapIndex = index + _baseLayerOffset;
		if (index >= 0 && mapIndex < _map->getNumImageLayers())
			_map->insertImageLayer(layer, mapIndex);
		else
			_map->addImageLayer(layer);
	}

	return layer;
}

osgEarth::ModelLayer*
Project::createModelLayer(osg::ref_ptr<const Godzi::DataSource> source, int index)
{
	osgEarth::ModelLayer* layer = source->createModelLayer();
	if (layer)
	{
		layer ->setEnabled(source->visible());

		int mapIndex = index + _baseLayerOffset;
		if (index >= 0 && mapIndex < _map->getNumModelLayers())
			_map->insertModelLayer(layer, mapIndex);
		else
			_map->addModelLayer(layer);
	}

	return layer;
}

int
Project::findSourceLayersIndex(unsigned int id)
{
	for (int i=0; i < _sourceLayers.size(); i++)
		if (_sourceLayers[i].valid() && _sourceLayers[i].id() == id)
			return i;

	return -1;
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
				app->setProject(new Godzi::Project(_defaultMap.get(), conf), _location);

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
