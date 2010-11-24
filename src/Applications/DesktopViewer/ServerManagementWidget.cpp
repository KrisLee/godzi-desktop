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

#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeView>
#include <QAction>
#include <QIcon>
#include <osgDB/FileNameUtils>
#include <osgEarth/TileSource>
#include <osgEarth/Config>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgEarthDrivers/wms/WMSOptions>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/DataSources>
#include "OpenFileDialog"
#include "WMSOptionsWidget"
#include "WMSEditDialog"
#include "ServerManagementWidget"

ServerManagementWidget::ServerManagementWidget(Godzi::Application* app)
{
	_app = app;
	createActions();
	initUi();

	connect(_app, SIGNAL(projectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)), this, SLOT(onProjectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)));
	connect(_sourceTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onTreeItemChanged(QTreeWidgetItem*, int)));
}

void ServerManagementWidget::createActions()
{
	_addSourceAction = new QAction(QIcon(":/resources/images/add.png"), tr("&Add"), this);
	connect(_addSourceAction, SIGNAL(triggered()), this, SLOT(addSource()));

	_removeSourceAction = new QAction(QIcon(":/resources/images/remove.png"), tr("&Remove"), this);
	_removeSourceAction->setEnabled(false);
	connect(_removeSourceAction, SIGNAL(triggered()), this, SLOT(removeSource()));

	_editSourceAction = new QAction(QIcon(":/resources/images/edit.png"), tr("&Edit"), this);
	_editSourceAction->setEnabled(false);
	connect(_editSourceAction, SIGNAL(triggered()), this, SLOT(editSource()));
}

void ServerManagementWidget::initUi()
{
	QStringList types;
	types << "WMS" << "TMS" << "KML/KMZ";
	_typeBox = new QComboBox();
	_typeBox->setEditable(false);
	_typeBox->addItems(types);

	_toolbar = new QToolBar();
	_toolbar->setIconSize(QSize(16, 16));
	_toolbar->setStyleSheet("QToolBar { border: 0px }");
	_toolbar->addWidget(_typeBox);
	_toolbar->addAction(_addSourceAction);
	_toolbar->addAction(_removeSourceAction);
	_toolbar->addAction(_editSourceAction);

	_sourceTree = new QTreeWidget();
	_sourceTree->setColumnCount(1);
	_sourceTree->setHeaderHidden(true);
	connect(_sourceTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(2);
	layout->setContentsMargins(3, 0, 3, 3);
	layout->addWidget(_toolbar);
	layout->addWidget(_sourceTree);
  setLayout(layout);
}

void ServerManagementWidget::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (_sourceTree->indexOfTopLevelItem(current) != -1)
		_removeSourceAction->setEnabled(true);
	else
		_removeSourceAction->setEnabled(false);

	CustomDataSourceTreeItem* item = dynamic_cast<CustomDataSourceTreeItem*>(current);
	if (item && item->getSource() && item->getSource()->type() == Godzi::WMSSource::TYPE_WMS)
		_editSourceAction->setEnabled(true);
	else
		_editSourceAction->setEnabled(false);
}

void ServerManagementWidget::onTreeItemChanged(QTreeWidgetItem* item, int col)
{
	CustomDataSourceTreeItem* sourceItem = findParentSourceItem(item);

	if (sourceItem)
	{
		if (sourceItem == item)
			_app->actionManager()->doAction(this, new Godzi::ToggleDataSourceAction(sourceItem->getSource(), sourceItem->checkState(0) == Qt::Checked));
		else
			updateVisibilitiesFromTree(sourceItem);
	}
}

void ServerManagementWidget::addSource()
{
	QString type = _typeBox->currentText();
	if (!type.compare("TMS"))
	{
		addTMSSource();
	}
	else if (!type.compare("WMS"))
	{
		addOrUpdateWMSSource();
	}
	else if (!type.compare("KML/KMZ"))
	{
		addKMLSource();
	}
}

void ServerManagementWidget::removeSource()
{
	CustomDataSourceTreeItem* item = dynamic_cast<CustomDataSourceTreeItem*>(_sourceTree->currentItem());
	if (item)
		_app->actionManager()->doAction(this, new Godzi::RemoveDataSourceAction(item->getSource()));
}

void ServerManagementWidget::editSource()
{
	CustomDataSourceTreeItem* item = dynamic_cast<CustomDataSourceTreeItem*>(_sourceTree->currentItem());
	if (item && item->getSource()->type() == Godzi::WMSSource::TYPE_WMS)
		addOrUpdateWMSSource((Godzi::WMSSource*)item->getSource());
}

void ServerManagementWidget::onProjectChanged(osg::ref_ptr<Godzi::Project> oldProject, osg::ref_ptr<Godzi::Project> newProject)
{
	_sourceTree->clear();

	Godzi::Project* p = _app->getProject();
	if (p)
	{
		connect(p, SIGNAL(dataSourceAdded(osg::ref_ptr<const Godzi::DataSource>, int)), this, SLOT(onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource>, int)));
		connect(p, SIGNAL(dataSourceUpdated(osg::ref_ptr<const Godzi::DataSource>)), this, SLOT(onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource>)));
		connect(p, SIGNAL(dataSourceRemoved(osg::ref_ptr<const Godzi::DataSource>)), this, SLOT(onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource>)));
		connect(p, SIGNAL(dataSourceMoved(osg::ref_ptr<const Godzi::DataSource>, int)), this, SLOT(onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource>, int)));
		connect(p, SIGNAL(dataSourceToggled(unsigned int, bool)), this, SLOT(onDataSourceToggled(unsigned int, bool)));

		Godzi::DataSourceVector sources;
		p->getSources(sources);
		for (Godzi::DataSourceVector::const_iterator it = sources.begin(); it != sources.end(); ++it)
			processDataSource(*it);
	}

	//TODO: disconnect from old project signal???
}

void ServerManagementWidget::onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	processDataSource(source, position);
}

void ServerManagementWidget::onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource> source)
{
	CustomDataSourceTreeItem* item;
	findDataSourceTreeItem(source, &item);
	if (item)
	  updateDataSourceTreeItem(source, item);
}

void ServerManagementWidget::onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource> source)
{
	int index = findDataSourceTreeItem(source);
	if (index != -1)
		_sourceTree->takeTopLevelItem(index);
}

void ServerManagementWidget::onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	if (!source.valid() || position < 0)
		return;

	CustomDataSourceTreeItem* item;
	int index = findDataSourceTreeItem(source, &item);
	if (item && index != -1)
	{
		_sourceTree->takeTopLevelItem(index);
		
		if (position >= _sourceTree->topLevelItemCount())
			_sourceTree->addTopLevelItem(item);
		else
			_sourceTree->insertTopLevelItem(position, item);
	}
}

void ServerManagementWidget::onDataSourceToggled(unsigned int id, bool visible)
{
	CustomDataSourceTreeItem* item;
	findDataSourceTreeItem(id, &item);
	if (item && (item->checkState(0) == Qt::Checked) != visible)
		item->setCheckState(0, visible ? Qt::Checked : Qt::Unchecked);
}

void ServerManagementWidget::processDataSource(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	if (!source.valid())
		return;

	QTreeWidgetItem* item = createDataSourceTreeItem(source);

	if (position < 0 || position >= _sourceTree->topLevelItemCount())
		_sourceTree->addTopLevelItem(item);
	else
		_sourceTree->insertTopLevelItem(position, item);
}

QTreeWidgetItem* ServerManagementWidget::createDataSourceTreeItem(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid())
		return new QTreeWidgetItem();

	CustomDataSourceTreeItem* item = new CustomDataSourceTreeItem(source->clone());
	updateDataSourceTreeItem(source, item);
	//item->setFlags(item->flags() & ~(Qt::ItemIsEnabled));

	return item;
}

void ServerManagementWidget::updateDataSourceTreeItem(osg::ref_ptr<const Godzi::DataSource> source, CustomDataSourceTreeItem* item)
{
	if (!source.valid() || !item)
		return;

	item->setSource(source->clone());

	item->setText(0, QString::fromStdString(source->name().isSet() ? source->name().get() : source->getLocation()));

	if (source->error())
	{
		item->setForeground(0, Qt::red);
		item->setToolTip(0, QString::fromStdString(source->errorMsg()));
	}
	else
	{
		item->setForeground(0, Qt::black);
		item->setToolTip(0, tr(""));
	}

	item->setCheckState(0, source->visible() ? Qt::Checked : Qt::Unchecked);
	
	std::vector<std::string> layers = source->getAvailableLayers();
	std::vector<std::string> active = source->getActiveLayers();

	//Remove old child items and free memory
	QList<QTreeWidgetItem*> oldChildren = item->takeChildren();
	//while (!oldChildren.isEmpty())
  //   delete oldChildren.takeFirst();

	for (int i=0; i < layers.size(); i++)
	{
		QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(QString::fromStdString(source->layerDisplayName(layers[i]))));
		child->setData(0, Qt::UserRole, QString::fromStdString(layers[i]));
		child->setCheckState(0, std::find(active.begin(), active.end(), layers[i]) == active.end() ? Qt::Unchecked : Qt::Checked);
		item->addChild(child);
	}
}

int ServerManagementWidget::findDataSourceTreeItem(osg::ref_ptr<const Godzi::DataSource> source, CustomDataSourceTreeItem** out_item)
{
	if (!source.valid() || !source->id().isSet())
		return -1;

	return findDataSourceTreeItem(source->id().get(), out_item);
}

int ServerManagementWidget::findDataSourceTreeItem(unsigned int id, CustomDataSourceTreeItem** out_item)
{
	int index = -1;
	for (int i=0; i < _sourceTree->topLevelItemCount(); i++)
	{
		CustomDataSourceTreeItem* item = dynamic_cast<CustomDataSourceTreeItem*>(_sourceTree->topLevelItem(i));
		if (item && item->getSource()->id().get() == id)
		{
			index = i;

			if (out_item)
				*out_item = item;

			break;
		}
	}

	return index;
}

CustomDataSourceTreeItem* ServerManagementWidget::findParentSourceItem(QTreeWidgetItem* item)
{
	if (!item)
		return 0;

	CustomDataSourceTreeItem* parent = dynamic_cast<CustomDataSourceTreeItem*>(item);
	if (!parent)
		parent = findParentSourceItem(item->parent());

	return parent;
}

void ServerManagementWidget::updateVisibilitiesFromTree(CustomDataSourceTreeItem* item)
{
	Godzi::DataSource* source = item->getSource();
	source->setVisible(item->checkState(0) == Qt::Checked);

	std::vector<std::string> activeLayers;
	for (int i=0; i < item->childCount(); i++)
	{
		QTreeWidgetItem* child = item->child(i);

		if (child->checkState(0) == Qt::Checked)
			activeLayers.push_back(child->data(0, Qt::UserRole).toString().toStdString());
	}
	source->setActiveLayers(activeLayers);

	_app->actionManager()->doAction(this, new Godzi::AddorUpdateDataSourceAction(source));
}

void ServerManagementWidget::addTMSSource()
{
	OpenFileDialog ofd(false);

	if (ofd.exec() == QDialog::Accepted)
	{
		QString url = ofd.getUrl();
		if (!url.isNull() && !url.isEmpty())
		{
			osgEarth::Drivers::TMSOptions opt;
			opt.url() = url.toStdString();
			_app->actionManager()->doAction(this, new Godzi::AddorUpdateDataSourceAction(new Godzi::TMSSource(opt)));
		}
	}
}

void ServerManagementWidget::addOrUpdateWMSSource(Godzi::WMSSource* source)
{
	WMSEditDialog wed(source);
	if (wed.exec() == QDialog::Accepted)
		_app->actionManager()->doAction(this, new Godzi::AddorUpdateDataSourceAction(wed.getSource()));
}

void ServerManagementWidget::addKMLSource()
{
	OpenFileDialog ofd(tr("Location..."), tr(""), tr("KML/KMZ files (*.kml *.kmz);;All files (*.*)"));
	if (ofd.exec() == QDialog::Accepted)
	{
		QString url = ofd.getUrl();
		if (!url.isNull() && !url.isEmpty())
		{

      Godzi::Features::KMLFeatureSourceOptions opt;
			opt.url() = url.toStdString();
			_app->actionManager()->doAction(this, new Godzi::AddorUpdateDataSourceAction(new Godzi::KMLSource(opt)));
		}
	}
}
