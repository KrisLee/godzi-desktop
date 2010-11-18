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

#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/DataSources>
#include <osgEarth/Config>
#include <osgEarthUtil/WMS>
#include <osgDB/FileNameUtils>

#include "DataSourceManager"

DataSourceManager::DataSourceManager(Godzi::Application* app)
: _app(app), _baseLayerOffset(0)
{
	
	connect(app, SIGNAL(projectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)), this, SLOT(onProjectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)));
	//app->actionManager()->addAfterActionCallback(this);
}

//void DataSourceManager::operator()( void* sender, Godzi::Action* action )
//{
//	AddorUpdateDataSourceAction* addAction = dynamic_cast<AddorUpdateDataSourceAction*>(action);
//	if (addAction)
//	{
//		DataSource* s = addAction->getDataSource();
//	}
//}

void DataSourceManager::onProjectChanged(osg::ref_ptr<Godzi::Project> oldProject, osg::ref_ptr<Godzi::Project> newProject)
{
	_sourceLayers.clear();

	if (newProject.valid())
	{
		connect(newProject.get(), SIGNAL(dataSourceAdded(osg::ref_ptr<const Godzi::DataSource>, int)), this, SLOT(onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource>, int)));
		connect(newProject.get(), SIGNAL(dataSourceRemoved(osg::ref_ptr<const Godzi::DataSource>)), this, SLOT(onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource>)));
		connect(newProject.get(), SIGNAL(dataSourceMoved(osg::ref_ptr<const Godzi::DataSource>, int)), this, SLOT(onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource>, int)));
		connect(newProject.get(), SIGNAL(dataSourceUpdated(osg::ref_ptr<const Godzi::DataSource>)), this, SLOT(onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource>)));

		_baseLayerOffset = newProject->map()->getNumImageLayers();

		for (std::vector<osg::ref_ptr<Godzi::DataSource>>::const_iterator it = newProject->sources().begin(); it != newProject->sources().end(); ++it)
		{
			addDataSource(*it);
		}
	}

	//TODO: disconnect from old project signal???
}

void DataSourceManager::onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
		addDataSource(source, position);
}

void DataSourceManager::onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource> source)
{
	removeDataSource(source);
}

void DataSourceManager::onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	if (!source.valid() || !source->id().isSet() || position < 0)
		return;

	int layerIndex = findSourceLayersIndex(source->id().get());
	if (layerIndex >= 0)
	{
		SourcedLayers layers = _sourceLayers[layerIndex];
		osgEarth::ImageLayer* imageLayer = layers.imageLayer.get();
		osgEarth::ModelLayer* modelLayer = layers.modelLayer.get();

		int mapIndex = position + _baseLayerOffset;

		if (imageLayer)
			_app->getProject()->map()->moveImageLayer(imageLayer, mapIndex);

		if (modelLayer)
			_app->getProject()->map()->moveModelLayer(modelLayer, mapIndex);

		_sourceLayers.erase(_sourceLayers.begin() + layerIndex);
		if (_sourceLayers.size() < position)
			_sourceLayers.resize(position);

		_sourceLayers.insert(_sourceLayers.begin() + position, layers);
	}
}

void DataSourceManager::onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid() || !source->id().isSet())
		return;

	int layerIndex = removeDataSource(source);
	addDataSource(source, layerIndex);
}

void DataSourceManager::addDataSource(osg::ref_ptr<const Godzi::DataSource> source, int index)
{
	if (!source.valid() || !source->id().isSet())
		return;

	osgEarth::ImageLayer* imageLayer = createImageLayer(source, index);
	osgEarth::ModelLayer* modelLayer = createModelLayer(source, index);

	SourcedLayers layers(source->id().get(), imageLayer, modelLayer);
	if (index >= 0)
	{
		if (_sourceLayers.size() <= index)
			_sourceLayers.resize(index + 1);

		if (!_sourceLayers[index].valid)
			_sourceLayers[index] = layers;
		else
			_sourceLayers.insert(_sourceLayers.begin() + index, layers);
	}
	else
	{
		_sourceLayers.push_back(layers);
	}
}

int DataSourceManager::removeDataSource(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid() || !source->id().isSet())
		return -1;

	int layerIndex = findSourceLayersIndex(source->id().get());
	if (layerIndex >= 0)
	{
		SourcedLayers layers = _sourceLayers[layerIndex];
		osgEarth::ImageLayer* imageLayer = layers.imageLayer.get();
		osgEarth::ModelLayer* modelLayer = layers.modelLayer.get();

		if (imageLayer)
			_app->getProject()->map()->removeImageLayer(imageLayer);

		if (modelLayer)
			_app->getProject()->map()->removeModelLayer(modelLayer);

		_sourceLayers.erase(_sourceLayers.begin() + layerIndex);
	}

	return layerIndex;
}

osgEarth::ImageLayer* DataSourceManager::createImageLayer(osg::ref_ptr<const Godzi::DataSource> source, int index)
{
	osgEarth::ImageLayer* layer = source->createImageLayer();
	if (layer && source->visible())
	{
		int mapIndex = index + _baseLayerOffset;
		if (index >= 0 && mapIndex < _app->getProject()->map()->getNumImageLayers())
			_app->getProject()->map()->insertImageLayer(layer, mapIndex);
		else
			_app->getProject()->map()->addImageLayer(layer);
	}

	return layer;
}

osgEarth::ModelLayer* DataSourceManager::createModelLayer(osg::ref_ptr<const Godzi::DataSource> source, int index)
{
	osgEarth::ModelLayer* layer = source->createModelLayer();
	if (layer && source->visible())
	{
		int mapIndex = index + _baseLayerOffset;
		if (index >= 0 && mapIndex < _app->getProject()->map()->getNumModelLayers())
			_app->getProject()->map()->insertModelLayer(layer, mapIndex);
		else
			_app->getProject()->map()->addModelLayer(layer);
	}

	return layer;
}

int DataSourceManager::findSourceLayersIndex(unsigned int id)
{
	for (int i=0; i < _sourceLayers.size(); i++)
		if (_sourceLayers[i].valid && _sourceLayers[i].id == id)
			return i;

	return -1;
}