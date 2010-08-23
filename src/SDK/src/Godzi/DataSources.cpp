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

#include <osgEarth/Config>
#include <osgEarthDrivers/wms/WMSOptions>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/DataSources>

using namespace Godzi;

WMSSource::WMSSource(Godzi::DataSource::Type type, osgEarth::Drivers::WMSOptions* opt)
{
	_type = type;
	_opt = opt;
}

Godzi::Config WMSSource::toConfig() const
{
	Godzi::Config conf;
	//conf.add("type", _type);
	conf.add("options", ((osgEarth::DriverOptions*)_opt)->toConfig());
  return conf;
}

const std::string& WMSSource::getLocation()
{
	return _opt->url().get();
}

osgEarth::DriverOptions* WMSSource::getOptions()
{
	return _opt;
}

/* --------------------------------------------- */

TMSSource::TMSSource(Godzi::DataSource::Type type, osgEarth::Drivers::TMSOptions* opt)
{
	_type = type;
	_opt = opt;
}

Godzi::Config TMSSource::toConfig() const
{
	Godzi::Config conf;
	//conf.add("type", _type);
	conf.add("options", ((osgEarth::DriverOptions*)_opt)->toConfig());
  return conf;
}

const std::string& TMSSource::getLocation()
{
	return _opt->url().get();
}

osgEarth::DriverOptions* TMSSource::getOptions()
{
	return _opt;
}

/* --------------------------------------------- */

bool AddDataSourceAction::doAction(void *sender, Application *app)
{
	if (!_source)
		return false;

	app->getProject()->addDataSource(_source);
	return true;
}

bool AddDataSourceAction::undoAction(void *sender, Application *app)
{
	if (!_source)
		return false;

	//app->getProject()->removeDataSource(_source);
	return true;
}

/* --------------------------------------------- */

DataSourceManager::DataSourceManager(Godzi::Application* app)
//: _app(app)
{
	connect(app, SIGNAL(projectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)), this, SLOT(onProjectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)));
}

void DataSourceManager::onProjectChanged(osg::ref_ptr<Godzi::Project> oldProject, osg::ref_ptr<Godzi::Project> newProject)
{
	if (newProject.valid())
		connect(newProject.get(), SIGNAL(dataSourceAdded(Godzi::DataSource*)), this, SLOT(onDataSourceAdded(Godzi::DataSource*)));

	//TODO: disconnect from old project signal???
}

void DataSourceManager::onDataSourceAdded(Godzi::DataSource* source)
{
	/*
	TMSOptions* opt = new TMSOptions();
	opt->url() = "http://tile.openstreetmap.org/";
	opt->tmsType() = "google";
	opt->profileConfig() = ProfileConfig( "global-mercator" );
	MapLayer* osmLayer = new ImageMapLayer( "OSM", opt );
	mapNode->getMap()->addMapLayer( osmLayer );
	*/

	if (!source)
		return;

	//if (source->
}