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

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPoint>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QModelIndex>
#include <Godzi/Application>
#include "ServerTreeWidget"
#include "DataObjectActionAdapter"

//------------------------------------------------------------------------
//
//// This is a custom object to store in QTreeWidgetItem->data() that holds
//// a reference to the data object being represented by the tree item. Since
//// data() holds a QVariant, we have to declare this as a Qt metatype in 
//// order to store it there.
//namespace
//{
//    struct TreeItemData
//    {
//        const Godzi::DataSource* _source;
//        Godzi::DataObjectSpec _spec;
//    };
//}
//Q_DECLARE_METATYPE(TreeItemData);


//------------------------------------------------------------------------

ServerTreeWidget::ServerTreeWidget(Godzi::Application *app)
: _app(app)
{
	setColumnCount(1);
	setHeaderHidden(true);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
	setDropIndicatorShown(true);

	connect(_app, SIGNAL(projectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)), this, SLOT(onProjectChanged(osg::ref_ptr<Godzi::Project>, osg::ref_ptr<Godzi::Project>)));
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onTreeItemChanged(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*,int)));
}

void ServerTreeWidget::processDataSource(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	if (!source.valid())
		return;

	QTreeWidgetItem* item = createDataSourceTreeItem(source);

	if (position < 0 || position >= topLevelItemCount())
		addTopLevelItem(item);
	else
		insertTopLevelItem(position, item);
}

QTreeWidgetItem* ServerTreeWidget::createDataSourceTreeItem(osg::ref_ptr<const Godzi::DataSource> source)
{
	if (!source.valid())
		return new QTreeWidgetItem();

	CustomDataSourceTreeItem* item = new CustomDataSourceTreeItem(source->clone());
	item->setFlags(item->flags() & ~(Qt::ItemIsDropEnabled));
	updateDataSourceTreeItem(source, item);
	//item->setFlags(item->flags() & ~(Qt::ItemIsEnabled));

	return item;
}

void ServerTreeWidget::updateDataSourceTreeItem(osg::ref_ptr<const Godzi::DataSource> source, CustomDataSourceTreeItem* item)
{
	if (!source.valid() || !item)
		return;

	item->setSource(source->clone());

    std::string text = source->name().isSet() ? source->name().get() : source->getLocation();
	item->setText(0, QString( text.c_str() ));

	if (source->error())
	{
		item->setForeground(0, Qt::red);
		item->setToolTip(0, QString( source->errorMsg().c_str() ));
	}
	else
	{
		item->setForeground(0, Qt::black);
		item->setToolTip(0, tr(""));
	}

	item->setCheckState(0, source->visible() ? Qt::Checked : Qt::Unchecked);

    //Remove old child items and free memory
    QList<QTreeWidgetItem*> oldChildren = item->takeChildren();
    //while (!oldChildren.isEmpty())
    //   delete oldChildren.takeFirst();

    // first, try data objects.
    Godzi::DataObjectSpecVector objSpecs;
    if ( source->getDataObjectSpecs( objSpecs ) )
    {
        for( Godzi::DataObjectSpecVector::const_iterator i = objSpecs.begin(); i != objSpecs.end(); ++i )
        {
            const Godzi::DataObjectSpec& spec = *i;

            QTreeWidgetItem* child = new QTreeWidgetItem( QStringList( QString( spec.getText().c_str() ) ) );

            WidgetUserDataToken token;
            token._source = source.get();
            token._spec = spec;

            // store the object spec in the data so we can reference it later during an action:
            child->setData( 0, Qt::UserRole, QVariant::fromValue(token) ); //spec.getObjectUID() );

            // disable the drop zone:
            child->setFlags( child->flags() & ~(Qt::ItemIsDropEnabled));

            item->addChild( child );
        }
    }

    else
    {
        // otherwise, try the old method:
	    std::vector<std::string> layers = source->getAvailableLayers();
	    std::vector<std::string> active = source->getActiveLayers();

	    for (int i=0; i < layers.size(); i++)
	    {
		    QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(QString(source->layerDisplayName(layers[i]).c_str())));
		    child->setData(0, Qt::UserRole, QString( layers[i].c_str() ) );
		    child->setFlags(child->flags() & ~(Qt::ItemIsDropEnabled));
		    //child->setCheckState(0, std::find(active.begin(), active.end(), layers[i]) == active.end() ? Qt::Unchecked : Qt::Checked);
		    item->addChild(child);
	    }
    }
}


int ServerTreeWidget::findDataSourceTreeItem(osg::ref_ptr<const Godzi::DataSource> source, CustomDataSourceTreeItem** out_item)
{
	if (!source.valid() || !source->id().isSet())
		return -1;

	return findDataSourceTreeItem(source->id().get(), out_item);
}

int ServerTreeWidget::findDataSourceTreeItem(unsigned int id, CustomDataSourceTreeItem** out_item)
{
	int index = -1;
	for (int i=0; i < topLevelItemCount(); i++)
	{
		CustomDataSourceTreeItem* item = dynamic_cast<CustomDataSourceTreeItem*>(topLevelItem(i));
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

CustomDataSourceTreeItem* ServerTreeWidget::findParentSourceItem(QTreeWidgetItem* item)
{
	if (!item)
		return 0;

	CustomDataSourceTreeItem* parent = dynamic_cast<CustomDataSourceTreeItem*>(item);
	if (!parent)
		parent = findParentSourceItem(item->parent());

	return parent;
}

void ServerTreeWidget::updateVisibilitiesFromTree(CustomDataSourceTreeItem* item)
{
	Godzi::DataSource* source = item->getSource();
	source->setVisible(item->checkState(0) == Qt::Checked);

	std::vector<std::string> activeLayers;
	for (int i=0; i < item->childCount(); i++)
	{
		QTreeWidgetItem* child = item->child(i);

		if (child->checkState(0) == Qt::Checked)
			activeLayers.push_back(child->data(0, Qt::UserRole).toString().toUtf8().data());
	}
	source->setActiveLayers(activeLayers);

	_app->actionManager()->doAction(this, new Godzi::AddorUpdateDataSourceAction(source));
}

void ServerTreeWidget::onProjectChanged(osg::ref_ptr<Godzi::Project> oldProject, osg::ref_ptr<Godzi::Project> newProject)
{
	clear();

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

void ServerTreeWidget::onTreeItemChanged(QTreeWidgetItem* item, int col)
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

void
ServerTreeWidget::onItemDoubleClicked(QTreeWidgetItem* item, int col)
{
    // pull our data token from the clicked item:
    QVariant v = item->data(col, Qt::UserRole);
    if ( !v.isNull() )
    {
        WidgetUserDataToken token = v.value<WidgetUserDataToken>();

        // query the data source for the list of available actions:
        Godzi::DataObjectActionSpecVector specs;
        if ( token._source->getDataObjectActionSpecs( specs ) )
        {
            // find the default action, if there is one:
            for(Godzi::DataObjectActionSpecVector::const_iterator i = specs.begin(); i != specs.end(); ++i )
            {
                Godzi::DataObjectActionSpecBase* actionSpec = i->get();
                if ( actionSpec->isDefaultAction() )
                {
                    // found it; fire off the default action:
                    Godzi::Action* action = actionSpec->createAction( token._source, token._spec.getObjectUID() );
                    _app->actionManager()->doAction( this, action );
                    break;
                }
            }
        }
    }
}

void ServerTreeWidget::onDataSourceAdded(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	processDataSource(source, position);
}

void ServerTreeWidget::onDataSourceUpdated(osg::ref_ptr<const Godzi::DataSource> source)
{
	CustomDataSourceTreeItem* item;
	findDataSourceTreeItem(source, &item);
	if (item)
	  updateDataSourceTreeItem(source, item);
}

void ServerTreeWidget::onDataSourceRemoved(osg::ref_ptr<const Godzi::DataSource> source)
{
	int index = findDataSourceTreeItem(source);
	if (index != -1)
		takeTopLevelItem(index);
}

void ServerTreeWidget::onDataSourceMoved(osg::ref_ptr<const Godzi::DataSource> source, int position)
{
	if (!source.valid() || position < 0)
		return;

	CustomDataSourceTreeItem* item;
	int index = findDataSourceTreeItem(source, &item);
	if (item && index != -1)
	{
		takeTopLevelItem(index);
		
		if (position >= topLevelItemCount())
			addTopLevelItem(item);
		else
			insertTopLevelItem(position, item);
	}
}

void ServerTreeWidget::onDataSourceToggled(unsigned int id, bool visible)
{
	CustomDataSourceTreeItem* item;
	findDataSourceTreeItem(id, &item);
	if (item && (item->checkState(0) == Qt::Checked) != visible)
		item->setCheckState(0, visible ? Qt::Checked : Qt::Unchecked);
}

void
ServerTreeWidget::contextMenuEvent( QContextMenuEvent* e )
{
    // pull the data token associated with the item under the mouse:
    QTreeWidgetItem* item = this->itemAt( e->pos() );
    if ( item )
    {
        QVariant qdata = item->data( 0, Qt::UserRole );
        if ( !qdata.isNull() && qdata.canConvert<WidgetUserDataToken>() )
        {
            WidgetUserDataToken token = qdata.value<WidgetUserDataToken>();

            // query the data source for the list of available actions:
            Godzi::DataObjectActionSpecVector actionSpecs;
            if ( token._source->getDataObjectActionSpecs( actionSpecs ) )
            {
                // make a context menu to hold all the action buttons:
                QMenu* contextMenu = new QMenu(this);

                DataObjectActionAdapterAutoList adapters;

                for( Godzi::DataObjectActionSpecVector::const_iterator i = actionSpecs.begin(); i != actionSpecs.end(); ++i )
                {
                    Godzi::DataObjectActionSpecBase* actionSpec = i->get();
                    DataObjectActionAdapter* adapter = new DataObjectActionAdapter( this, token, actionSpec, _app );
                    adapter->addActionTo( contextMenu );
                    adapters.push_back( adapter );
                }

                contextMenu->exec( e->globalPos() );
                delete contextMenu;
            }
        }
    }
}

void ServerTreeWidget::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->source() != this)
		e->ignore();
	else
		QTreeWidget::dragMoveEvent(e);
}

void ServerTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
	QModelIndex index = indexAt(e->pos());
	if (index.isValid() && index.parent().isValid())
		e->ignore();
	//else if (index.isValid() && dropIndicatorPosition() == QAbstractItemView::BelowItem && topLevelItem(index.row())->childCount() > 0 && topLevelItem(index.row())->isExpanded())
	//	e->ignore();
	else
		QTreeWidget::dragMoveEvent(e);
}

void ServerTreeWidget::dropEvent(QDropEvent* e)
{
	CustomDataSourceTreeItem* item = dynamic_cast<CustomDataSourceTreeItem*>(currentItem());
	if (item)
	{
		QModelIndex index = indexAt(e->pos());
		if (!index.isValid())
		{
			_app->actionManager()->doAction(this, new Godzi::MoveDataSourceAction(item->getSource(), _app->getProject()->getNumSources() - 1));
		}
		else if (!index.parent().isValid())
		{
			int newIndex = index.row();

			if (dropIndicatorPosition() == QAbstractItemView::BelowItem)
			{
				if (indexOfTopLevelItem(item) > newIndex)
					newIndex += 1;
			}
			else if ((dropIndicatorPosition() == QAbstractItemView::AboveItem || dropIndicatorPosition() == QAbstractItemView::OnItem) && indexOfTopLevelItem(item) < newIndex)
			{
				newIndex -= 1;
			}

			if (newIndex >= 0)
				_app->actionManager()->doAction(this, new Godzi::MoveDataSourceAction(item->getSource(), newIndex));
		}
	}
}