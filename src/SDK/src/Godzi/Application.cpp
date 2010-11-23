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

using namespace Godzi;

DataSourceFactoryManager* const Application::dataSourceFactoryManager = DataSourceFactoryManager::create();

Application::Application(std::string defaultCachePath, unsigned int defaultCacheMax, const Godzi::Config& conf)
: _defaultCachePath(defaultCachePath), _defaultCacheMax(defaultCacheMax), _mapCache(0L), _mapCacheEnabled(true)
{
	initApp(conf);
}

Application::Application(const Godzi::Config& conf)
: _defaultCachePath(""), _defaultCacheMax(300), _mapCache(0L), _mapCacheEnabled(true)
{
	initApp(conf);
}

void Application::initApp(const Godzi::Config& conf)
{
	_actionMgr = ActionManager::create(this);

	Application::dataSourceFactoryManager->addFactory(new WMSSourceFactory());
	Application::dataSourceFactoryManager->addFactory(new TMSSourceFactory());
	Application::dataSourceFactoryManager->addFactory(new KMLSourceFactory());

	initMapCache(conf.child("cache_config"));
}

Godzi::Config
Application::toConfig()
{
	  Godzi::Config conf( "godzi_app" );

		Godzi::Config cacheConf("cache_config");
		if (_mapCache)
			cacheConf.add("cache_opt", _mapCacheOpt.getConfig());

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

				if (_mapCacheEnabled && _mapCache)
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

	if (_mapCacheEnabled)
	{
		if (_mapCache)
			_project->map()->setCache(_mapCache);
	}
	else
	{
		_project->map()->setCache(0L);
	}
}

void
Application::setCache(const std::string& path, unsigned int max)
{
	//osgEarth::Drivers::Sqlite3CacheOptions newOpt;
	//newOpt.fromConfig(_mapCacheOpt.getConfig());
	newOpt.path() = path;
	newOpt.maxSize() = max;
	//_mapCacheOpt = newOpt;

	if (newOpt.path().isSet() && !newOpt.path().get().empty() &&
			newOpt.maxSize().isSet() && newOpt.maxSize() > 0)
		_mapCache = osgEarth::CacheFactory::create(newOpt);
	else
		_mapCache = 0L;

	_project->map()->setCache(_mapCache);
	_mapCacheOpt = newOpt;
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

void
Application::initMapCache(const Godzi::Config& cacheConf)
{
	_mapCacheOpt.fromConfig(cacheConf.child("cache_opt"));

	if (!_mapCacheOpt.path().isSet())
		_mapCacheOpt.path() = _defaultCachePath;

	if (!_mapCacheOpt.maxSize().isSet())
		_mapCacheOpt.maxSize() = _defaultCacheMax;

	if (_mapCacheOpt.path().isSet() && !_mapCacheOpt.path().get().empty())
		_mapCache = osgEarth::CacheFactory::create(_mapCacheOpt);

	osgEarth::optional<std::string> cacheEnabled;
	_mapCacheEnabled = cacheConf.getIfSet("cache_enabled", cacheEnabled) ? osgEarth::as<bool>(cacheEnabled.get(), true) : true;
}
