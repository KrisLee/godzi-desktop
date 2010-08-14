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
#include <QListWidget>
#include <QAction>
#include <QIcon>
#include "ServerManagementWidget"

ServerManagementWidget::ServerManagementWidget()
{
	createActions();
	initUi();
}

void ServerManagementWidget::createActions()
{
	_addServerAction = new QAction(QIcon(":/resources/images/add.png"), tr("&Add"), this);
	connect(_addServerAction, SIGNAL(triggered()), this, SLOT(addServer()));

	_removeServerAction = new QAction(QIcon(":/resources/images/remove.png"), tr("&Remove"), this);
	connect(_removeServerAction, SIGNAL(triggered()), this, SLOT(removeServer()));
}

void ServerManagementWidget::initUi()
{
	_toolbar = new QToolBar();
	_toolbar->setIconSize(QSize(16, 16));
	_toolbar->setStyleSheet("QToolBar { border: 0px } QToolButton { autoRise: true }");
	_toolbar->addAction(_addServerAction);
	_toolbar->addAction(_removeServerAction);

	_serverList = new QListWidget();

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(2);
	layout->setContentsMargins(3, 0, 3, 3);
	layout->addWidget(_toolbar);
	layout->addWidget(_serverList);
	layout->addStretch();
  setLayout(layout);
}

void ServerManagementWidget::addServer()
{
}

void ServerManagementWidget::removeServer()
{
}