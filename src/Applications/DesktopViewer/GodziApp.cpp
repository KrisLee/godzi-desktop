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

#include <QFile>
#include <Godzi/Common>
#include "GodziApp"

GodziApp::GodziApp(const osgEarth::CacheOptions& cacheOpt, const Godzi::Config& conf) : Godzi::Application(cacheOpt, conf)
{
  initSky(conf.child("sky_config"));
}

Godzi::Config GodziApp::toConfig()
{
  Godzi::Config conf = Godzi::Application::toConfig();

  Godzi::Config skyConf("sky_config");
  skyConf.add("sky_enabled", osgEarth::toString<bool>(_skyEnabled));
  skyConf.add("sun_mode", osgEarth::toString<int>(_sunMode));
  skyConf.add("sun_lat", osgEarth::toString(_sunLat));
  skyConf.add("sun_lon", osgEarth::toString(_sunLon));
  conf.addChild(skyConf);

  return conf;
}

void GodziApp::initSky(Godzi::Config conf)
{
  osgEarth::optional<std::string> skyEnabled;
	_skyEnabled = conf.getIfSet("sky_enabled", skyEnabled) ? osgEarth::as<bool>(skyEnabled.get(), true) : true;
  
  _sunMode = (SunMode)conf.value<int>("sun_mode", SunMode::Ubiquitous);
  _sunLat = conf.value<double>("sun_lat", 0.0);
  _sunLon = conf.value<double>("sun_lon", -98.5);
}

void GodziApp::setSkyEnabled(bool enabled)
{
  //TODO: Toggle actual sky rendering
  _skyEnabled = enabled;
}

osgEarth::Util::SkyNode* GodziApp::createSkyNode()
{
  _sky = _project ? new osgEarth::Util::SkyNode(_project->map()) : 0L;

  if (_sky.valid())
    _sky->setSunPosition(osg::DegreesToRadians(_sunLat), osg::DegreesToRadians(_sunLon));

  return _sky.get();
}

void GodziApp::setSunMode(SunMode mode)
{
  //TODO: Implement actual sun mode functionality
  _sunMode = mode;
}

/** Get the sun position (lat/lon in degrees) */
void GodziApp::getSunPosition(double& out_lat, double& out_lon) const
{
  out_lat = _sunLat;
  out_lon = _sunLon;
}

/** Set the sun position (lat/lon in degrees) */
void GodziApp::setSunPosition(double lat, double lon)
{
  //TODO: Implement actual sun position setting
  _sunLat = lat;
  _sunLon = lon;

  if (_sky.valid())
    _sky->setSunPosition(osg::DegreesToRadians(_sunLat), osg::DegreesToRadians(_sunLon));
}

osgEarth::optional<unsigned int> GodziApp::getCacheMaxSize() const
{
  osgEarth::optional<unsigned int> max;

  if (_mapCache)
    _mapCache->getCacheOptions().getConfig().getIfSet("max_size", max);

	return max;
}

void GodziApp::clearCache() const
{
  //TODO: Fix cache clearing

  //std::string cachePath = getCachePath();
  //if (!cachePath.empty() && QFile::exists(tr(cachePath.c_str())))
  //{
  //  bool tempEnabled = getCacheEnabled();

  //  const_cast<GodziApp*>(this)->setCacheEnabled(false);
  //  QFile::remove(tr(cachePath.c_str()));
  //  const_cast<GodziApp*>(this)->setCacheEnabled(tempEnabled);
  //}
}