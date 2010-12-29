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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <osgDB/FileNameUtils>
#include <osgEarth/Config>
#include <osgEarthUtil/WMS>
#include <osgEarthDrivers/wms/WMSOptions>
#include <Godzi/Common>
#include <Godzi/DataSources>
#include <Godzi/WMS/WMSActions>
#include <Godzi/WMS/WMSDataSource>

using namespace Godzi::WMS;

namespace
{
  const std::string EMPTY_STRING ="";

	std::string extractBetween(const std::string& str, const std::string &lhs, const std::string &rhs)
	{
			std::string result;
			std::string::size_type start = str.find(lhs);
			if (start != std::string::npos)
			{
					start += lhs.length();
					std::string::size_type count = str.size() - start;
					std::string::size_type end = str.find(rhs, start); 
					if (end != std::string::npos) count = end-start;
					result = str.substr(start, count);
			}
			return result;
	}
}

const std::string WMSDataSource::TYPE_WMS = "WMS";

WMSDataSource::WMSDataSource(const osgEarth::Drivers::WMSOptions& opt, bool visible)
: DataSource(visible), _updateNeeded(true)
{
	osgEarth::Config config = opt.getConfig();
	_opt = osgEarth::Drivers::WMSOptions((osgEarth::TileSourceOptions)config);
}

WMSDataSource::WMSDataSource(const std::string& url, bool visible)
: DataSource(visible), _updateNeeded(true)
{
	_opt = osgEarth::Drivers::WMSOptions();

	if (!url.empty())
	{
		_fullUrl = url;
		_opt.url() = url.substr(0, url.find("?"));
	}
}

WMSDataSource::WMSDataSource(const Godzi::Config& conf)
: DataSource(conf), _updateNeeded(true)
{
	_opt = osgEarth::Drivers::WMSOptions(osgEarth::TileSourceOptions(osgEarth::ConfigOptions(conf.child("options"))));
	conf.getIfSet("fullurl", _fullUrl);
}

Godzi::Config WMSDataSource::toConfig() const
{
	Godzi::Config conf = DataSource::toConfig();
	conf.add("type", TYPE_WMS);
	conf.add("options", _opt.getConfig());
	conf.addIfSet("fullUrl", _fullUrl);

  return conf;
}

const std::string& WMSDataSource::getLocation() const
{
	return !_fullUrl.isSet() || _fullUrl.get().empty() ? _opt.url().get() : _fullUrl.get();
}

const osgEarth::DriverConfigOptions& WMSDataSource::getOptions() const
{
	return _opt;
}

osgEarth::ImageLayer* WMSDataSource::createImageLayer() const
{
	const_cast<WMSDataSource*>(this)->update();

	if (_layers.size() > 0)
	{
		//std::string name = _name.isSet() ? _name.get() : "WMS Source";
		std::string name = (_opt.url().isSet() ? _opt.url().get() : "WMS") + "__" + (_opt.layers().isSet() ? _opt.layers().get() : "nolayers");
		return new osgEarth::ImageLayer(name, _opt);
	}
	else
	{
		return 0;
	}
}

Godzi::DataSource* WMSDataSource::clone() const
{
	// [jas] Following shouldn't be necessary, but the TMSOptions copy
	// constructor does not appear to be working correctly.
	osgEarth::Drivers::WMSOptions cOpt;
	cOpt.url() = _opt.url();
	cOpt.layers() = _opt.layers();
	cOpt.format() = _opt.format();
	cOpt.srs() = _opt.srs();
	cOpt.style() = _opt.style();

	//WMSDataSource* c = new WMSDataSource(new osgEarth::Drivers::WMSOptions(_opt), _visible);
	WMSDataSource* c;
	if (_fullUrl.isSet())

	WMSDataSource* c = new WMSDataSource(cOpt, _visible);
	if (_fullUrl.isSet())
		c->setFullUrl(_fullUrl.get());

	if (_name.isSet())
		c->name() = _name;

	if (_id.isSet())
		c->setId(_id.get());

	c->setError(_error);
	c->setErrorMsg(_errorMsg);

	//c->setLayers(_layers);

	return c;
}

void WMSDataSource::setFullUrl(const std::string& url)
{
	if (url.empty())
		_fullUrl.unset();
	else
		_fullUrl = url;
}

void WMSDataSource::update()
{
	if (_layers.size() > 0)
		return;

	std::string url = getLocation();
	if (url.length() == 0)
		return;

	std::vector<std::string> specifiedLayers = Godzi::csvToVector(parseWMSOptions(url));
	if (specifiedLayers.size() == 0 && _opt.layers().isSet())
		specifiedLayers = Godzi::csvToVector(_opt.layers().get());

	char sep = url.find_first_of('?') == std::string::npos ? '?' : '&';
	std::string capUrl = url + sep + "SERVICE=WMS" + "&REQUEST=GetCapabilities";

	//Try to read the WMS capabilities
	osg::ref_ptr<osgEarth::Util::WMSCapabilities> capabilities = osgEarth::Util::WMSCapabilitiesReader::read(capUrl, 0L);
	if (capabilities.valid())
	{
		//NOTE: Currently this flattens any layer heirarchy into a single list of layers
		_layers.clear();
		std::vector<std::string> opt_layers;
		processLayerList(capabilities->getLayers(), specifiedLayers, opt_layers);
		_opt.layers() = Godzi::vectorToCSV(opt_layers);

		_availableFormats.clear();
		osgEarth::Util::WMSCapabilities::FormatList formats = capabilities->getFormats();
		for (osgEarth::Util::WMSCapabilities::FormatList::const_iterator it = formats.begin(); it != formats.end(); ++it)
		{
			std::string format = *it;

			int pos = format.find("image/");
			if (pos != std::string::npos && int(pos) == 0)
				format.erase(0, 6);

			_availableFormats.push_back(format);
		}

		setError(false);
		setErrorMsg("");
	  _updateNeeded = false;
	}
	else
	{
		setError(true);
		setErrorMsg("Could not get WMS capabilities.");
	}
}

std::string WMSDataSource::parseWMSOptions(const std::string& url)
{
	std::string lower = osgDB::convertToLowerCase( url );

	std::string layers = "";
	if (lower.find("layers=", 0) != std::string::npos)
		layers = extractBetween(lower, "layers=", "&");

	if (lower.find("styles=", 0) != std::string::npos)
		_opt.style() = extractBetween(lower, "styles=", "&");

	if (lower.find("srs=", 0) != std::string::npos)
		_opt.srs() = extractBetween(lower, "srs=", "&");

	if (lower.find("format=image/", 0) != std::string::npos)
		_opt.format() = extractBetween(lower, "format=image/", "&");

	return layers;
}

void WMSDataSource::processLayerList(const osgEarth::Util::WMSLayer::LayerList& layerList, const std::vector<std::string>& subset, std::vector<std::string>& out_layers)
{
	for (int i=0; i < layerList.size(); i++)
	{
		if (subset.size() == 0 || std::find(subset.begin(), subset.end(), layerList[i]->getName()) != subset.end())
		{
			_layers.push_back(layerList[i]);
			out_layers.push_back(layerList[i]->getName());
		}

		processLayerList(layerList[i]->getLayers(), subset, out_layers);
	}
}

const std::vector<std::string>& WMSDataSource::getAvailableFormats() const
{
	const_cast<WMSDataSource*>(this)->update();

	return _availableFormats;
}

const std::string& WMSDataSource::getLayerName(int id)
{
	const_cast<WMSDataSource*>(this)->update();
	return (id >= 0 && _layers.size() > id) ? _layers[id]->getName() : EMPTY_STRING;
}

const osgEarth::Util::WMSLayer* WMSDataSource::getLayer(int id) const
{
	const_cast<WMSDataSource*>(this)->update();
	return (id >= 0 && _layers.size() > id) ? _layers[id] : 0L;
}

bool WMSDataSource::getDataObjectSpecs( Godzi::DataObjectSpecVector& out_objectSpecs ) const
{
	const_cast<WMSDataSource*>(this)->update();

	out_objectSpecs.clear();
	for (int i=0; i < _layers.size(); i++)
		out_objectSpecs.push_back(Godzi::DataObjectSpec(i, getDisplayName(_layers[i].get())));

	return true;
}

bool WMSDataSource::getDataObjectActionSpecs( Godzi::DataObjectActionSpecVector& out_actionSpecs ) const
{
	out_actionSpecs.push_back( new Godzi::DataObjectActionSpec<ZoomToWMSLayerAction>(
        "Locate",
        "Moves the camera to look at the layer",
        true ) );

    return true;
}

std::string WMSDataSource::getDisplayName (osgEarth::Util::WMSLayer* layer) const
{
	return layer->getTitle().size() > 0 ? layer->getTitle() + " (" + layer->getName() + ")" : layer->getName();
}

/* --------------------------------------------- */

bool WMSDataSourceFactory::canCreate(const Godzi::Config &config)
{
	osgEarth::optional<std::string> type;
	if (config.key().compare("datasource") == 0 && config.getIfSet<std::string>("type", type) && type.get() == WMSDataSource::TYPE_WMS)
			return true;

	return false;
}

Godzi::DataSource* WMSDataSourceFactory::createDataSource(const Godzi::Config& config)
{
	if (!canCreate(config))
		return 0L;

	return new WMSDataSource(config);
}