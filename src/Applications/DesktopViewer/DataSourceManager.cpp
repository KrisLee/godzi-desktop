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
: _app(app)
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
	_layerMap.clear();
	_layerModel.clear();

	if (newProject.valid())
	{
		connect(newProject.get(), SIGNAL(dataSourceAdded(osg::ref_ptr<const Godzi::DataSource>, int)), this, SLOT(onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource>, int)));
		connect(newProject.get(), SIGNAL(dataSourceRemoved(osg::ref_ptr<const Godzi::DataSource>)), this, SLOT(onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource>)));
		connect(newProject.get(), SIGNAL(dataSourceMoved(osg::ref_ptr<const Godzi::DataSource>, int)), this, SLOT(onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource>, int)));
		connect(newProject.get(), SIGNAL(dataSourceUpdated(osg::ref_ptr<const Godzi::DataSource>)), this, SLOT(onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource>)));

		for (std::vector<osg::ref_ptr<Godzi::DataSource>>::const_iterator it = newProject->sources().begin(); it != newProject->sources().end(); ++it)
		{
			processDataSource(*it);
		}
	}

	//TODO: disconnect from old project signal???
}

void DataSourceManager::onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
		processDataSource(source);
}

void DataSourceManager::onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid())
		return;

	std::map<std::string, osg::ref_ptr<osgEarth::ImageLayer> >::iterator it = _layerMap.find(source->getLocation());
	if (it != _layerMap.end())
	{
		_app->getProject()->map()->removeImageLayer(_layerMap[source->getLocation()]);
		_layerMap.erase(it);
	}

	std::map<std::string, osg::ref_ptr<osgEarth::ModelLayer> >::iterator itModel = _layerModel.find(source->getLocation());
	if (itModel != _layerModel.end())
	{
		_app->getProject()->map()->removeModelLayer(_layerModel[source->getLocation()]);
		_layerModel.erase(itModel);
	}
}

void DataSourceManager::onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	if (!source.valid() || position < 0)
		return;

	osgEarth::ImageLayer* layer = _layerMap[source->getLocation()];
	if (layer)
		_app->getProject()->map()->moveImageLayer(layer, position);

}

void DataSourceManager::onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid())
		return;

	std::map<std::string, osg::ref_ptr<osgEarth::ImageLayer> >::iterator it = _layerMap.find(source->getLocation());

	if (it != _layerMap.end())
	{
		_app->getProject()->map()->removeImageLayer(_layerMap[source->getLocation()]);
		_layerMap.erase(it);
	}

	createImageLayer(source);

	std::map<std::string, osg::ref_ptr<osgEarth::ModelLayer> >::iterator itModel = _layerModel.find(source->getLocation());

	if (itModel != _layerModel.end())
	{
      _app->getProject()->map()->removeModelLayer(_layerModel[source->getLocation()]);
      _layerModel.erase(itModel);
	}

	createModelLayer(source);
}

void DataSourceManager::processDataSource(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid())
		return;

	createImageLayer(source);
	createModelLayer(source);
}

osgEarth::ImageLayer* DataSourceManager::createImageLayer(osg::ref_ptr<const Godzi::DataSource> source)
{
	osgEarth::ImageLayer* mapLayer = source->createImageLayer();
	if (mapLayer)
	{
		_layerMap[source->getLocation()] = mapLayer;

		if (source->visible())
			_app->getProject()->map()->addImageLayer(mapLayer);
	}

	return mapLayer;
}

osgEarth::ModelLayer* DataSourceManager::createModelLayer(osg::ref_ptr<const Godzi::DataSource> source)
{
	osgEarth::ModelLayer* layer = source->createModelLayer();
	if (layer)
	{
		_layerModel[source->getLocation()] = layer;

		if (source->visible())
			_app->getProject()->map()->addModelLayer(layer);
	}

	return layer;
}