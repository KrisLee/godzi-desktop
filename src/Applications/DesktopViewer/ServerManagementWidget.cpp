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
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QAction>
#include <QIcon>
#include <osgEarth/TileSource>
#include <osgEarth/DriverOptions>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgEarthDrivers/wms/WMSOptions>
#include <Godzi/Application>
#include <Godzi/Project>
#include <Godzi/DataSources>
#include "OpenFileDialog"
#include "ServerManagementWidget"

ServerManagementWidget::ServerManagementWidget(Godzi::Application* app)
{
	_app = app;
	createActions();
	initUi();

	connect(_app, SIGNAL(projectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)), this, SLOT(onProjectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)));
}

void ServerManagementWidget::createActions()
{
	_addSourceAction = new QAction(QIcon(":/resources/images/add.png"), tr("&Add"), this);
	connect(_addSourceAction, SIGNAL(triggered()), this, SLOT(addSource()));

	_removeSourceAction = new QAction(QIcon(":/resources/images/remove.png"), tr("&Remove"), this);
	connect(_removeSourceAction, SIGNAL(triggered()), this, SLOT(removeSource()));
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

	_sourceTree = new QTreeWidget();
	_sourceTree->setColumnCount(1);
	_sourceTree->setHeaderHidden(true);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(2);
	layout->setContentsMargins(3, 0, 3, 3);
	layout->addWidget(_toolbar);
	layout->addWidget(_sourceTree);
	layout->addStretch();
  setLayout(layout);
}

void ServerManagementWidget::addSource()
{
	OpenFileDialog* ofd;

	QString type = _typeBox->currentText();
	if (!type.compare("KML/KMZ"))
	{
		ofd = new OpenFileDialog(tr("Location..."), tr(""), tr("KML/KMZ files (*.kml *.kmz);;All files (*.*)"));
	}
	else
	{
		ofd = new OpenFileDialog(false);
	}

	if (ofd->exec() == QDialog::Accepted)
	{
		QString url = ofd->getUrl();
		if (!url.isNull() && !url.isEmpty())
		{
			Godzi::DataSource* s;
			if (!type.compare("WMS"))
			{
				osgEarth::Drivers::WMSOptions* opt = new osgEarth::Drivers::WMSOptions();
				opt->url() = url.toStdString();
				s = new Godzi::WMSSource(Godzi::DataSource::TYPE_WMS, opt);
			}
			else if (!type.compare("TMS"))
			{
				osgEarth::Drivers::TMSOptions* opt = new osgEarth::Drivers::TMSOptions();
				opt->url() = url.toStdString();
				s = new Godzi::TMSSource(Godzi::DataSource::TYPE_TMS, opt);
			}
			else if (!type.compare("KML/KMZ"))
			{
				//TODO
			}

			if (s)
				_app->actionManager()->doAction(this, new Godzi::AddDataSourceAction(s));
		}
	}
}

void ServerManagementWidget::removeSource()
{
}

void ServerManagementWidget::onProjectChanged(osg::ref_ptr<Godzi::Project> oldProject, osg::ref_ptr<Godzi::Project> newProject)
{
	Godzi::Project* p = _app->getProject();

	if (p)
		connect(p, SIGNAL(dataSourceAdded(Godzi::DataSource*)), this, SLOT(onDataSourceAdded(Godzi::DataSource*)));

	//TODO: disconnect from old project signal???
}

void ServerManagementWidget::onDataSourceAdded(Godzi::DataSource* source)
{
	QString s;
	if (source->name().isSet())
		s = QString::fromStdString(source->name().get());
	else
		s = QString::fromStdString(source->getLocation());

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(s));
	item->setCheckState(0, Qt::Checked);
	//item->setFlags(item->flags() & ~(Qt::ItemIsEnabled));
	_sourceTree->addTopLevelItem(item);
}