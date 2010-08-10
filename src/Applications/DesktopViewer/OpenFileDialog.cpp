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

#include <QFileDialog>
#include "OpenFileDialog"

OpenFileDialog::OpenFileDialog()
{
	this->caption = tr("Select file...");
	this->dir = tr("");
	this->filter = tr("All files (*.*)");

	InitUi();
}

OpenFileDialog::OpenFileDialog(const QString &caption, const QString &dir, const QString &filter)
{
	this->caption = caption;
	this->dir = dir;
	this->filter = filter;

	InitUi();
}

void OpenFileDialog::InitUi()
{
	ui.setupUi(this);
	QObject::connect(ui.browseButton, SIGNAL(clicked()), this, SLOT(showBrowse()));
}

QString OpenFileDialog::getUrl()
{
	return ui.urlText->text();
}

void OpenFileDialog::showBrowse()
{
	QString filename = QFileDialog::getOpenFileName(this, caption, dir, filter);

	if (!filename.isNull())
		ui.urlText->setText(filename);
}