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
#include <osgEarth/DriverOptions>

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
//		
//		if (s->type() == DataSource::TYPE_TMS)
//		{
//			osgEarth::MapLayer* tmsLayer = new ImageMapLayer( "TMS", s->getOptions());
//			_app->getProject()->map()->addMapLayer(tmsLayer);
//		}
//	}
//}

void DataSourceManager::onProjectChanged(osg::ref_ptr<Godzi::Project> oldProject, osg::ref_ptr<Godzi::Project> newProject)
{
	if (newProject.valid())
	{
		connect(newProject.get(), SIGNAL(dataSourceAdded(osg::ref_ptr<const Godzi::DataSource>, int)), this, SLOT(onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource>, int)));
		connect(newProject.get(), SIGNAL(dataSourceRemoved(osg::ref_ptr<const Godzi::DataSource>)), this, SLOT(onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource>)));
		connect(newProject.get(), SIGNAL(dataSourceMoved(osg::ref_ptr<const Godzi::DataSource>, int)), this, SLOT(onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource>, int)));
		connect(newProject.get(), SIGNAL(dataSourceUpdated(osg::ref_ptr<const Godzi::DataSource>)), this, SLOT(onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource>)));
	}

	//TODO: disconnect from old project signal???
}

void DataSourceManager::onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	if (!source.valid())
		return;

	std::string name = source->name().isSet() ? source->name().get() : "Data Source";

	osgEarth::MapLayer* mapLayer=0;
	if (source->type() == Godzi::DataSource::TYPE_TMS || source->type() == Godzi::DataSource::TYPE_WMS)
	{
		mapLayer = new ImageMapLayer(name, source->getOptions());
	}
	else
	{
		//TODO
	}

	if (mapLayer)
	{
		_app->getProject()->map()->addMapLayer(mapLayer);
		_layerMap[source->getLocation()] = mapLayer;
	}
}

void DataSourceManager::onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid())
		return;

	std::map<std::string, osg::ref_ptr<osgEarth::MapLayer>>::iterator it = _layerMap.find(source->getLocation());

	if (it != _layerMap.end())
	{
		_app->getProject()->map()->removeMapLayer(_layerMap[source->getLocation()]);
		_layerMap.erase(it);
	}
}

void DataSourceManager::onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	if (!source.valid() || position < 0)
		return;

	osgEarth::MapLayer* layer = _layerMap[source->getLocation()];
	if (layer)
		_app->getProject()->map()->moveMapLayer(layer, position);
}

void DataSourceManager::onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid())
		return;

	std::map<std::string, osg::ref_ptr<osgEarth::MapLayer>>::iterator it = _layerMap.find(source->getLocation());

	if (it != _layerMap.end())
	{
		osgEarth::MapLayer* mapLayer = _layerMap[source->getLocation()];

		osgEarth::MapLayerList layers = _app->getProject()->map()->getImageMapLayers();
		if (!source->visible())
			_app->getProject()->map()->removeMapLayer(mapLayer);
		else if (std::find(layers.begin(), layers.end(), mapLayer) == layers.end())
			_app->getProject()->map()->addMapLayer(mapLayer);
	}
}