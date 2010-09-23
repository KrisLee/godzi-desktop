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

const std::vector<std::string> DataSource::NO_LAYERS = std::vector<std::string>();

/* --------------------------------------------- */

const std::string WMSSource::TYPE_WMS = "WMS";

Godzi::Config WMSSource::toConfig() const
{
	Godzi::Config conf;
	//conf.add("type", _type);
	conf.add("options", ((osgEarth::DriverOptions*)_opt)->toConfig());
  return conf;
}

const std::string& WMSSource::getLocation() const
{
	return _fullUrl.empty() ? _opt->url().get() : _fullUrl;
}

const osgEarth::DriverOptions* WMSSource::getOptions() const
{
	return _opt.get();
}

osgEarth::MapLayer* WMSSource::createMapLayer() const
{
	if (getActiveLayers().size() > 0)
	{
		std::string name = _name.isSet() ? _name.get() : "WMS Source";
		return new ImageMapLayer(name, _opt.get());
	}
	else
	{
		return 0;
	}
}

DataSource* WMSSource::clone() const
{
	// [jas] Following shouldn't be necessary, but the TMSOptions copy
	// constructor does not appear to be working correctly.
	osgEarth::Drivers::WMSOptions* cOpt = new osgEarth::Drivers::WMSOptions();
	cOpt->url() = _opt->url();
	cOpt->layers() = _opt->layers();

	//WMSSource* c = new WMSSource(new osgEarth::Drivers::WMSOptions(_opt), _visible, _fullUrl);
	WMSSource* c = new WMSSource(cOpt, _visible, _fullUrl);

	if (_name.isSet())
		c->name() = _name;

	c->setError(_error);
	c->setErrorMsg(_errorMsg);
	c->setAvailableLayers(_availableLayers);
	c->setLayerDisplayNames(_displayNames);

	return c;
}

const std::vector<std::string> WMSSource::getAvailableLayers() const
{
	return _availableLayers;
}

void WMSSource::setAvailableLayers(const std::vector<std::string>& layers)
{
	_availableLayers.assign(layers.begin(), layers.end());
}

const std::vector<std::string> WMSSource::getActiveLayers() const
{
	std::vector<std::string> layers;

	if (_opt->layers().isSet())
	{
		char* cstr = new char [_opt->layers().get().size() + 1];
		strcpy (cstr, _opt->layers().get().c_str());

		char* p = strtok(cstr, ",");

		while (p != NULL)
		{
			layers.push_back(std::string(p));
			p = strtok(NULL,",");
		}

		delete[] cstr;
	}

	return layers;
}

void WMSSource::setActiveLayers(const std::vector<std::string>& layers)
{
	if (layers.size() > 0)
	{
		std::string layerStr = layers[0];

		for (int i=1; i < layers.size(); i++)
			layerStr = layerStr + "," + layers[i];

		_opt->layers() = layerStr;
	}
	else
	{
		_opt->layers().unset();
	}
}

const std::string& WMSSource::layerDisplayName (const std::string& layerName) const
{
	std::map<std::string, std::string>::const_iterator it = _displayNames.find(layerName);
	if (it != _displayNames.end())
		return (*it).second;

	return layerName;
}

/* --------------------------------------------- */

const std::string TMSSource::TYPE_TMS = "TMS";

Godzi::Config TMSSource::toConfig() const
{
	Godzi::Config conf;
	//conf.add("type", _type);
	conf.add("options", ((osgEarth::DriverOptions*)_opt)->toConfig());
  return conf;
}

const std::string& TMSSource::getLocation() const
{
	return _opt->url().get();
}

const osgEarth::DriverOptions* TMSSource::getOptions() const
{
	return _opt.get();
}

osgEarth::MapLayer* TMSSource::createMapLayer() const
{
	std::string name = _name.isSet() ? _name.get() : "TMS Source";
	return new ImageMapLayer(name, _opt.get());
}

DataSource* TMSSource::clone() const
{
	// [jas] Following shouldn't be necessary, but the TMSOptions copy
	// constructor does not appear to be working correctly.
	osgEarth::Drivers::TMSOptions* cOpt = new osgEarth::Drivers::TMSOptions();
	cOpt->url() = _opt->url();

	//TMSSource* c = new TMSSource(new osgEarth::Drivers::TMSOptions(_opt));
	TMSSource* c = new TMSSource(cOpt);
	if (_name.isSet())
		c->name() = _name;
	
	c->setError(_error);
	c->setErrorMsg(_errorMsg);

	return c;
}

/* --------------------------------------------- */

bool AddorUpdateDataSourceAction::doAction(void *sender, Application *app)
{
	if (!_source)
		return false;

	Godzi::DataSource* old;
	_wasUpdate = app->getProject()->updateDataSource(_source, &old);

	if (_wasUpdate)
		_oldSource = old;
	else
		app->getProject()->addDataSource(_source);

	return true;
}

bool AddorUpdateDataSourceAction::undoAction(void *sender, Application *app)
{
	if (!_source)
		return false;

	if (_wasUpdate)
		app->getProject()->updateDataSource(_oldSource);
	else
		app->getProject()->removeDataSource(_source);

	return true;
}

/* --------------------------------------------- */

bool RemoveDataSourceAction::doAction(void *sender, Application *app)
{
	if (!_source)
		return false;

	app->getProject()->removeDataSource(_source);
	return true;
}

bool RemoveDataSourceAction::undoAction(void *sender, Application *app)
{
	if (!_source)
		return false;

	app->getProject()->addDataSource(_source);
	return true;
}