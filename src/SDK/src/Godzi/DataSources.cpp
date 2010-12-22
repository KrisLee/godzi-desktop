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
#include <Godzi/Common>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/DataSources>

using namespace Godzi;

const std::vector<std::string> DataSource::NO_LAYERS = std::vector<std::string>();


DataSource::DataSource(const Godzi::Config &conf)
: _error(false), _errorMsg("")
{
	conf.getIfSet("name", _name);
	_visible = osgEarth::as<bool>(conf.value<std::string>("visible", "true"), true);
}

Godzi::Config DataSource::toConfig() const
{
	Godzi::Config conf = Godzi::Config("DataSource");
	conf.addIfSet("name", _name);
	conf.add("visible", osgEarth::toString<bool>(_visible));

	//conf.add("options", getOptions().getConfig());

	return conf;
}

/* --------------------------------------------- */

const std::string TMSSource::TYPE_TMS = "TMS";

TMSSource::TMSSource(const osgEarth::Drivers::TMSOptions& opt, bool visible)
: DataSource(visible)
{
	osgEarth::Config config = opt.getConfig();
	_opt = osgEarth::Drivers::TMSOptions((osgEarth::TileSourceOptions)config);
}

TMSSource::TMSSource(const Godzi::Config& conf)
: DataSource(conf)
{
	_opt = osgEarth::Drivers::TMSOptions(osgEarth::TileSourceOptions(osgEarth::ConfigOptions(conf.child("options"))));
}

Godzi::Config TMSSource::toConfig() const
{
	Godzi::Config conf = DataSource::toConfig();
	conf.add("type", TYPE_TMS);
	conf.add("options", _opt.getConfig());

  return conf;
}

const std::string& TMSSource::getLocation() const
{
	return _opt.url().get();
}

osgEarth::ImageLayer* TMSSource::createImageLayer() const
{
	std::string name = getLocation();
	return new osgEarth::ImageLayer(name, _opt);
}

DataSource* TMSSource::clone() const
{
	// [jas] Following shouldn't be necessary, but the TMSOptions copy
	// constructor does not appear to be working correctly.
	osgEarth::Drivers::TMSOptions cOpt;
	if (_opt.url().isSet())
		cOpt.url() = _opt.url().get();

	//TMSSource* c = new TMSSource(new osgEarth::Drivers::TMSOptions(_opt));
	TMSSource* c = new TMSSource(cOpt);
	if (_name.isSet())
		c->name() = _name;

	if (_id.isSet())
		c->setId(_id.get());
	
	c->setError(_error);
	c->setErrorMsg(_errorMsg);

	return c;
}

/* --------------------------------------------- */

class DataSourceFactoryManagerImpl : public DataSourceFactoryManager //no export
{
public:
	DataSourceFactoryManagerImpl();

	void addFactory(DataSourceFactory* factory);
	DataSourceFactory* getFactory(const Godzi::Config& config);

private:
	std::list<osg::ref_ptr<DataSourceFactory>> _factories;
};

DataSourceFactoryManagerImpl::DataSourceFactoryManagerImpl()
{
	//nop
}

void DataSourceFactoryManagerImpl::addFactory(DataSourceFactory *factory)
{
	_factories.push_back(factory);
}

DataSourceFactory* DataSourceFactoryManagerImpl::getFactory(const Godzi::Config& config)
{
	DataSourceFactory* found=0;
	for (std::list<osg::ref_ptr<DataSourceFactory>>::iterator i = _factories.begin(); i != _factories.end(); ++i)
	{
		if (i->get()->canCreate(config))
		{
			found = i->get();
			break;
		}
	}

	return found;
}

/* --------------------------------------------- */

DataSourceFactoryManager::DataSourceFactoryManager()
{
	//nop
}

DataSourceFactoryManager*
DataSourceFactoryManager::create()
{
	return new DataSourceFactoryManagerImpl();
}

/* --------------------------------------------- */

//TMS
bool TMSSourceFactory::canCreate(const Godzi::Config &config)
{
	osgEarth::optional<std::string> type;
	if (config.key().compare("datasource") == 0 && config.getIfSet<std::string>("type", type) && type.get() == TMSSource::TYPE_TMS)
			return true;

	return false;
}

DataSource* TMSSourceFactory::createDataSource(const Godzi::Config& config)
{
	if (!canCreate(config))
		return 0L;

	return new TMSSource(config);
}
/* --------------------------------------------- */

bool AddorUpdateDataSourceAction::doAction(void *sender, Application *app)
{
	if (!_source)
		return false;

	_wasDirty = app->isProjectDirty();

	Godzi::DataSource* old;
	_wasUpdate = app->getProject()->updateDataSource(_source, _dirtyProject, &old);

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

	if (!_wasDirty)
		app->setProjectClean();

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

bool MoveDataSourceAction::doAction(void *sender, Application *app)
{
	if (!_source || _newIndex < 0)
		return false;

	_oldIndex = app->getProject()->moveDataSource(_source.get(), _newIndex);
	return _oldIndex < 0 ? false : true;
}

bool MoveDataSourceAction::undoAction(void *sender, Application *app)
{
	if (!_source || _oldIndex < 0)
		return false;

	int found = app->getProject()->moveDataSource(_source.get(), _oldIndex);
	return found < 0 ? false : true;
}

/* --------------------------------------------- */

bool ToggleDataSourceAction::doAction(void *sender, Application *app)
{
	if (!_id.isSet())
		return false;

	_changed = app->getProject()->toggleDataSource(_id.get(), _visible);
	return _changed;
}

bool ToggleDataSourceAction::undoAction(void *sender, Application *app)
{
	if (!_changed || !_id.isSet())
		return false;

	app->getProject()->toggleDataSource(_id.get(), !_visible);
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
