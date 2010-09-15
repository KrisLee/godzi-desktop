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

#include <QVariant>
#include <QModelIndex>
#include <osgEarth/Config>
#include <osgEarthDrivers/wms/WMSOptions>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/DataSources>

using namespace Godzi;

const std::vector<std::string> DataSource::NO_LAYERS = std::vector<std::string>();

/* --------------------------------------------- */

Godzi::Config WMSSource::toConfig() const
{
	Godzi::Config conf;
	//conf.add("type", _type);
	conf.add("options", ((osgEarth::DriverOptions*)_opt)->toConfig());
  return conf;
}

const std::string& WMSSource::getLocation() const
{
	return _opt->url().get();
}

const osgEarth::DriverOptions* WMSSource::getOptions() const
{
	return _opt;
}

DataSource* WMSSource::clone() const
{
	WMSSource* c = new WMSSource(_type, new osgEarth::Drivers::WMSOptions(_opt));
	if (_name.isSet())
		c->name() = _name;

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
			layerStr = layerStr + ", " + layers[i];

		_opt->layers() = layerStr;
	}
	else
	{
		_opt->layers().unset();
	}
}

/* --------------------------------------------- */

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
	return _opt;
}

DataSource* TMSSource::clone() const
{
	// [jas] Following shouldn't be necessary, but the TMSOptions copy
	// constructor does not appear to be working correctly.
	osgEarth::Drivers::TMSOptions* cOpt = new osgEarth::Drivers::TMSOptions();
	cOpt->url() = _opt->url();

	//TMSSource* c = new TMSSource(_type, new osgEarth::Drivers::TMSOptions(_opt));
	TMSSource* c = new TMSSource(_type, cOpt);
	if (_name.isSet())
		c->name() = _name;
	

	return c;
}

//const std::vector<std::string>& TMSSource::getAvailableLayers() const
//{
//	return NO_LAYERS;
//}

//const std::vector<std::string>& TMSSource::getActiveLayers() const
//{
//	return NO_LAYERS;
//}

/* --------------------------------------------- */

//QModelIndex ProjectDataSourceListModel::index(int row, int column, const QModelIndex &parent) const
//{
//	if (column == 0)
//	{
//		if (!parent.isValid())
//		{
//			if (row < _project->sources().size())
//				return QAbstractItemModel::createIndex(row, 0, _project->sources()[row]);
//		}
//		else
//		{
//			if (!parent.parent().isValid() &&
//				  parent.row() < _project->sources().size() &&
//					row < _project->sources()[parent.row()]->getAvailableLayers().size())
//			{
//				return QAbstractItemModel::createIndex(row, 0, &_project->sources()[parent.row()]->getAvailableLayers()[row]);
//			}
//		}
//	}
//
//	return QModelIndex();
//}
//
//QModelIndex ProjectDataSourceListModel::parent(const QModelIndex &index) const
//{
//	if (index.isValid())
//	{
//		Godzi::DataSource* source = static_cast<Godzi::DataSource*>(index.internalPointer());
//		if (source) return 
//
//		//std::string
//	}
//
//	return QModelIndex();
//}
//
//int ProjectDataSourceListModel::rowCount(const QModelIndex &parent) const
//{
//	if (!parent.isValid())
//	{
//		return _project->sources().size();
//	}
//	else if (parent.parent().isValid() || parent.row() > _project->sources().size())
//	{
//		return 0;
//	}
//	else
//	{
//		return _project->sources()[parent.row]->getAvailableLayers().size();
//	}
//}
//
//int ProjectDataSourceListModel::columnCount(const QModelIndex &parent) const
//{
//	return 1;
//}
//
//QVariant ProjectDataSourceListModel::data(const QModelIndex &index, int role) const
//{
//	if (!index.isValid())
//		return QVariant();
//
//	if (role == Qt::DisplayRole)
//	{
//		if (index.parent().isValid())
//		{
//			if (index.parent().parent().isValid() ||
//				  index.parent().row() > _project->sources().size())// ||
//					//_project->sources()[index.parent().row()]->
//			{
//				return QVariant();
//			}
//			else 
//			{
//				DataSource* s = _project->sources()[index.parent().row()];
//				if (index.row() > s->getAvailableLayers().size())
//					return QVariant();
//				else
//					return QString::fromStdString(_project->sources()[index.parent().row()]->getAvailableLayers()[index.row()]);
//			}
//		}
//		else
//		{
//			if (index.row() > _project->sources().size())
//			{
//				return QVariant();
//			}
//			else
//			{
//				Godzi::DataSource* src = _project->sources()[index.row()];
//				return src->name().isSet() ? QString::fromStdString(src->name().get()) : QString::fromStdString(src->getLocation());
//			}
//		}
//	}
//	else
//	{
//		return QVariant();
//	}
//}
//
//QVariant ProjectDataSourceListModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//	if (role != Qt::DisplayRole)
//		return QVariant();
//	
//	if (orientation == Qt::Horizontal)
//		return QString("Column %1").arg(section);
//	else
//		return QString("Row %1").arg(section);
//}

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