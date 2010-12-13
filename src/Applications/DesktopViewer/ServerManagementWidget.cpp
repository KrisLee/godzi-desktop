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
#include <QMouseEvent>
#include <QAction>
#include <QIcon>
#include <osgDB/FileNameUtils>
#include <osgEarth/TileSource>
#include <osgEarth/Config>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgEarthDrivers/wms/WMSOptions>
#include <Godzi/KML/KMLDataSource>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/DataSources>
#include "OpenFileDialog"
#include "WMSOptionsWidget"
#include "WMSEditDialog"
#include "ServerTreeWidget"
#include "ServerManagementWidget"

using namespace Godzi;
using namespace Godzi::KML;

ServerManagementWidget::ServerManagementWidget(Godzi::Application* app)
{
	_app = app;
	createActions();
	initUi();
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

	//_sourceTree = new QTreeWidget();
	_sourceTree = new ServerTreeWidget(_app);
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

void ServerManagementWidget::addTMSSource()
{
	OpenFileDialog ofd(false);

	if (ofd.exec() == QDialog::Accepted)
	{
		QString url = ofd.getUrl();
		if (!url.isNull() && !url.isEmpty())
		{
			osgEarth::Drivers::TMSOptions opt;
			opt.url() = url.toUtf8().data();
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

            KMLFeatureSourceOptions opt;
            opt.url() = url.toUtf8().data();
            _app->actionManager()->doAction(this, new Godzi::AddorUpdateDataSourceAction(new KMLDataSource(opt)));
		}
	}
}
