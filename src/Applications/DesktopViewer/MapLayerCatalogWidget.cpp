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
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <Godzi/Application>
#include <Godzi/Project>
#include "MapLayerCatalogWidget"

MapLayerCatalogWidget::MapLayerCatalogWidget(Godzi::Application* app)
{
	_app = app;
	initUi();
	update();

	connect(_app, SIGNAL(projectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)), this, SLOT(onProjectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)));
	connect(_sourceTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onTreeItemChanged(QTreeWidgetItem*, int)));
}

void MapLayerCatalogWidget::initUi()
{
	_sourceTree = new QTreeWidget();
	_sourceTree->setColumnCount(1);
	_sourceTree->setHeaderHidden(true);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(2);
	layout->setContentsMargins(3, 0, 3, 3);
	layout->addWidget(_sourceTree);
  setLayout(layout);
}

void MapLayerCatalogWidget::onProjectChanged(osg::ref_ptr<Godzi::Project> oldProject, osg::ref_ptr<Godzi::Project> newProject)
{
	update();
}

void MapLayerCatalogWidget::onTreeItemChanged(QTreeWidgetItem* item, int col)
{
	CustomObjectTreeItem* objItem = dynamic_cast<CustomObjectTreeItem*>(item);
	if (objItem)
	{
		osgEarth::ImageLayer* mapLayer = dynamic_cast<osgEarth::ImageLayer*>(objItem->getObj());
		if (mapLayer)
		{
			mapLayer->setEnabled(objItem->checkState(0) == Qt::Checked);
		}
		else
		{
			osgEarth::ModelLayer* modelLayer = dynamic_cast<osgEarth::ModelLayer*>(objItem->getObj());
			if (modelLayer)
			{
				modelLayer->setEnabled(objItem->checkState(0) == Qt::Checked);
			}
		}
	}
}

void MapLayerCatalogWidget::update()
{
	_sourceTree->clear();

	QTreeWidgetItem* imagesItem = new QTreeWidgetItem();
	imagesItem->setIcon(0, QIcon(":/resources/images/globe.png"));
	imagesItem->setText(0, "Image Layers");
	_sourceTree->addTopLevelItem(imagesItem);

	QTreeWidgetItem* modelsItem = new QTreeWidgetItem();
	modelsItem->setIcon(0, QIcon(":/resources/images/box.png"));
	modelsItem->setText(0, "Model Layers");
	_sourceTree->addTopLevelItem(modelsItem);

	if (_app->getProject() && _app->getProject()->map())
	{
		osgEarth::ImageLayerVector imageLayers;
		_app->getProject()->map()->getImageLayers(imageLayers);
		for (osgEarth::ImageLayerVector::const_iterator it = imageLayers.begin(); it != imageLayers.end(); ++it)
		{
			osgEarth::ImageLayer* mapLayer = it->get();
			if (mapLayer)
			{
				CustomObjectTreeItem* mapLayerItem = new CustomObjectTreeItem(mapLayer);
				mapLayerItem->setText(0, QString::fromStdString(mapLayer->getName()));
				mapLayerItem->setCheckState(0, mapLayer->getEnabled() ? Qt::Checked : Qt::Unchecked);
				imagesItem->addChild(mapLayerItem);
			}
		}

		osgEarth::ModelLayerVector modelLayers;
		_app->getProject()->map()->getModelLayers(modelLayers);
		for (osgEarth::ModelLayerVector::const_iterator it = modelLayers.begin(); it != modelLayers.end(); ++it)
		{
			osgEarth::ModelLayer* modelLayer = it->get();
			if (modelLayer)
			{
				CustomObjectTreeItem* modelLayerItem = new CustomObjectTreeItem(modelLayer);
				modelLayerItem->setText(0, QString::fromStdString(modelLayer->getName()));
				modelLayerItem->setCheckState(0, modelLayer->getEnabled() ? Qt::Checked : Qt::Unchecked);
				modelsItem->addChild(modelLayerItem);
			}
		}
	}
}
