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
#include <osgEarthFeatures/FeatureModelSource>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/DataSources>
#include <Godzi/Features/Feature>
#include <Godzi/Features/KMLFeatureModelSource>
#include <Godzi/Features/KMLFeatureSource>

using namespace Godzi;

const std::vector<std::string> DataSource::NO_LAYERS = std::vector<std::string>();

/* --------------------------------------------- */

const std::string WMSSource::TYPE_WMS = "WMS";

WMSSource::WMSSource(const osgEarth::Drivers::WMSOptions& opt, bool visible)
: DataSource(visible)
{
	osgEarth::Config config = opt.getConfig();
	_opt = osgEarth::Drivers::WMSOptions((osgEarth::TileSourceOptions)config);
}

WMSSource::WMSSource(const osgEarth::Drivers::WMSOptions& opt, const std::string& fullUrl, bool visible)
: DataSource(visible)
{
	osgEarth::Config config = opt.getConfig();
	_opt = osgEarth::Drivers::WMSOptions((osgEarth::TileSourceOptions)config);

	_fullUrl = fullUrl;
}

Godzi::Config WMSSource::toConfig() const
{
	Godzi::Config conf = DataSource::toConfig();
	conf.add("type", TYPE_WMS);
	conf.addIfSet("name", _name);
	conf.add("visible", osgEarth::toString<bool>(_visible));
	conf.add("options", _opt.getConfig());
	conf.addIfSet("fullUrl", _fullUrl);

  return conf;
}

const std::string& WMSSource::getLocation() const
{
	return !_fullUrl.isSet() || _fullUrl.get().empty() ? _opt.url().get() : _fullUrl.get();
}

const osgEarth::DriverConfigOptions& WMSSource::getOptions() const
{
	return _opt;
}

osgEarth::ImageLayer* WMSSource::createImageLayer() const
{
	if (getActiveLayers().size() > 0)
	{
		std::string name = _name.isSet() ? _name.get() : "WMS Source";
		return new osgEarth::ImageLayer(name, _opt);
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
	osgEarth::Drivers::WMSOptions cOpt;
	cOpt.url() = _opt.url();
	cOpt.layers() = _opt.layers();

	//WMSSource* c = new WMSSource(new osgEarth::Drivers::WMSOptions(_opt), _visible);
	WMSSource* c = new WMSSource(cOpt, _visible);
	if (_fullUrl.isSet())
		c->fullUrl() = _fullUrl.get();

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

	if (_opt.layers().isSet())
	{
		char* cstr = new char [_opt.layers().get().size() + 1];
		strcpy (cstr, _opt.layers().get().c_str());

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

		_opt.layers() = layerStr;
	}
	else
	{
		_opt.layers().unset();
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

TMSSource::TMSSource(const osgEarth::Drivers::TMSOptions& opt, bool visible)
: DataSource(visible)
{
	osgEarth::Config config = opt.getConfig();
	_opt = osgEarth::Drivers::TMSOptions((osgEarth::TileSourceOptions)config);
}

Godzi::Config TMSSource::toConfig() const
{
	Godzi::Config conf = DataSource::toConfig();
	conf.add("type", TYPE_TMS);
	conf.addIfSet("name", _name);
	conf.add("visible", osgEarth::toString<bool>(_visible));
	conf.add("options", _opt.getConfig());

  return conf;
}

const std::string& TMSSource::getLocation() const
{
	return _opt.url().get();
}

const osgEarth::DriverConfigOptions& TMSSource::getOptions() const
{
	return _opt;
}

osgEarth::ImageLayer* TMSSource::createImageLayer() const
{
	std::string name = _name.isSet() ? _name.get() : "TMS Source";
	return new osgEarth::ImageLayer(name, _opt);
}

DataSource* TMSSource::clone() const
{
	// [jas] Following shouldn't be necessary, but the TMSOptions copy
	// constructor does not appear to be working correctly.
	osgEarth::Drivers::TMSOptions cOpt;
	cOpt.url() = _opt.url();

	//TMSSource* c = new TMSSource(new osgEarth::Drivers::TMSOptions(_opt));
	TMSSource* c = new TMSSource(cOpt);
	if (_name.isSet())
		c->name() = _name;
	
	c->setError(_error);
	c->setErrorMsg(_errorMsg);

	return c;
}

/* --------------------------------------------- */

//bool TMSSourceFactory::canCreate(const Godzi::Config& config)
//{
//	osgEarth::optional<std::string> type;
//	if(config.key().compare("DataSource") == 0 && config.getIfSet<std::string>("type", type) && type.get().compare(TMSSource::TYPE_TMS) == 0)
//		return true;
//
//	return false;
//}
//
//DataSource* TMSSourceFactory::createDataSource(const Godzi::Config& config)
//{
//	if (!canCreate(config))
//		return 0;
//
//	osgEarth::Drivers::TMSOptions *opt = new osgEarth::Drivers::TMSOptions(new osgEarth::PluginOptions(config));
//	if (opt)
//	{
//		TMSSource* source = new TMSSource(opt);
//
//		osgEarth::optional<std::string> name;
//		if(config.getIfSet("name", name))
//			source->name() = name.get();
//
//		osgEarth::optional<std::string> visible;
//		if (config.getIfSet("visible", visible))
//			source->setVisible(osgEarth::as<bool>(visible, true));
//	}
//	return opt;
//}

/* --------------------------------------------- */

KMLSource::KMLSource(const Godzi::Features::KMLFeatureSourceOptions& opt, bool visible)
: DataSource(visible)
{
	osgEarth::Config config = opt.getConfig();
	_opt = Godzi::Features::KMLFeatureSourceOptions((osgEarth::TileSourceOptions)config);

	_fs = new Godzi::Features::KMLFeatureSource(_opt);
	_fs->initialize();
}

KMLSource::KMLSource(const Godzi::Features::KMLFeatureSourceOptions& opt, bool visible, Godzi::Features::KMLFeatureSource* source)
: DataSource(visible), _fs(source)
{
	osgEarth::Config config = opt.getConfig();
	_opt = Godzi::Features::KMLFeatureSourceOptions((osgEarth::TileSourceOptions)config);

	if (!source)
	{
		_fs = new Godzi::Features::KMLFeatureSource(_opt);
		_fs->initialize();
	}
}

const std::string KMLSource::TYPE_KML = "KML";

Godzi::Config KMLSource::toConfig() const
{
	Godzi::Config conf = DataSource::toConfig();
	conf.add("type", TYPE_KML);
	conf.addIfSet("name", _name);
	conf.add("visible", osgEarth::toString<bool>(_visible));
	conf.add("options", _opt.getConfig());

  return conf;
}

const std::string& KMLSource::getLocation() const
{
	return _opt.url().get();
}

const osgEarth::DriverConfigOptions& KMLSource::getOptions() const
{
	return _opt;
}

const std::vector<std::string> KMLSource::getAvailableLayers() const
{
	std::vector<std::string> layers;

	osgEarth::Features::FeatureList features = _fs->getFeaturesList();
	for (osgEarth::Features::FeatureList::iterator it = features.begin(); it != features.end(); ++it)
	{
		osgEarth::Features::Feature* f = it->get();
		layers.push_back(f->getName());
	}

	return layers;
}

const std::vector<std::string> KMLSource::getActiveLayers() const
{
	return getAvailableLayers();
}

void KMLSource::setActiveLayers(const std::vector<std::string>& layers)
{
}

osgEarth::ModelLayer* KMLSource::createModelLayer() const
{
	std::string name = _name.isSet() ? _name.get() : "KML Source";

  osgEarth::Features::FeatureModelSourceOptions option;
  option.featureSource() = _fs;

  return new osgEarth::ModelLayer(name, new Godzi::Features::KMLFeatureModelSource(option));
}

DataSource* KMLSource::clone() const
{
	// [jas] Following shouldn't be necessary, but the TMSOptions copy
	// constructor does not appear to be working correctly.
	Godzi::Features::KMLFeatureSourceOptions cOpt;
	cOpt.url() = _opt.url();

	//TMSSource* c = new TMSSource(new osgEarth::Drivers::TMSOptions(_opt));
	KMLSource* c = new KMLSource(cOpt, true, _fs);
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

/* --------------------------------------------- */

//bool SelectDataSourceAction::doAction(void *sender, Application *app)
//{
//	if (!_source)
//		return false;
//
//	return true;
//}
