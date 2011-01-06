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
//#include <osgEarthDrivers/cache_sqlite3/Sqlite3CacheOptions>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/WMS/WMSDataSource>
#include <Godzi/KML/KMLDataSource>

using namespace Godzi;

DataSourceFactoryManager* const Application::dataSourceFactoryManager = DataSourceFactoryManager::create();

Application::Application(const osgEarth::CacheOptions& cacheOpt, const Godzi::Config& conf)
: _mapCache(0L), _mapCacheEnabled(true)
{
	initApp(conf);

	initMapCache(conf.child("cache_config"), cacheOpt);
}

Application::Application(const Godzi::Config& conf)
: _mapCache(0L), _mapCacheEnabled(false)
{
	initApp(conf);

	Godzi::Config cacheConf = conf.child("cache_config");
	initMapCache(cacheConf, osgEarth::CacheOptions(osgEarth::ConfigOptions(cacheConf.child("cache_opt"))));
}

void
Application::initApp(const Godzi::Config& conf)
{
	_actionMgr = ActionManager::create(this);

	Application::dataSourceFactoryManager->addFactory(new WMS::WMSDataSourceFactory());
	Application::dataSourceFactoryManager->addFactory(new TMSSourceFactory());
  Application::dataSourceFactoryManager->addFactory(new KML::KMLDataSourceFactory());
}

void
Application::initMapCache(const Godzi::Config& cacheConf, const osgEarth::CacheOptions& cacheOpt)
{
  _mapCacheEnabled = osgEarth::as<bool>(cacheConf.value<std::string>("cache_enabled", "false"), false);

	if (cacheOpt.getDriver().empty())
	{
		osgEarth::CacheOptions newOpt = osgEarth::CacheOptions(cacheOpt);

		osgEarth::optional<std::string> cacheDriver;
		if (cacheConf.getIfSet("cache_driver", cacheDriver))
			newOpt.setDriver(cacheDriver.get());

		setCache(newOpt);
	}
	else
	{
		setCache(cacheOpt);
	}
}

Godzi::Config
Application::toConfig()
{
	  Godzi::Config conf( "godzi_app" );

		Godzi::Config cacheConf("cache_config");
		if (_mapCache)
		{
			cacheConf.add("cache_driver", _mapCache->getCacheOptions().getDriver());
			cacheConf.add("cache_opt", _mapCache->getCacheOptions().getConfig());
		}

		cacheConf.add("cache_enabled", osgEarth::toString<bool>(_mapCacheEnabled));
		conf.addChild(cacheConf);

    return conf;
}

void
Application::setProject( Project* project, const std::string& projectLocation )
{
    if ( project && project != _project )
    {
				osg::ref_ptr<Project> oldProject = _project;
        _project = project;
        _projectLocation = projectLocation;
        _project->sync( _projectCheckpoint );

				if (_project->map() && !_project->map()->getCache() && _mapCacheEnabled && _mapCache)
					_project->map()->setCache(_mapCache);

				emit projectChanged(oldProject, _project);
    }
}

void
Application::setCacheEnabled(bool enabled)
{
	if (_mapCacheEnabled == enabled)
		return;

	_mapCacheEnabled = enabled;

  
  //TODO: add functionality in osgEarth to allow disabling of the map cache

	if (_mapCacheEnabled)
	{
		if (_mapCache)
			_project->map()->setCache(_mapCache);
	}
	//else
	//{
	//	_project->map()->setCache(0L);
	//}
}

std::string
Application::getCachePath() const
{
  if (_mapCache)
    return _mapCache->getCacheOptions().getConfig().value("path");

	return "";
}

void
Application::setCache(const osgEarth::CacheOptions& cacheOpt)
{
	_mapCache = osgEarth::CacheFactory::create(cacheOpt);
	if (_mapCache && _mapCacheEnabled && _project.valid())
		_project->map()->setCache(_mapCache);
}

bool
Application::isProjectDirty() const
{
    return !_project.valid() || _project->outOfSyncWith( _projectCheckpoint );
}

void
Application::setProjectClean()
{
    if ( _project.valid() )
        _project->sync( _projectCheckpoint );
}
