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

OpenFileDialog::OpenFileDialog(bool canBrowse, QWidget* options)
: _options(options), _caption("Location..."), _dir(""), _filter("All files (*.*)")
{
	InitUi(canBrowse);
}

OpenFileDialog::OpenFileDialog(const QString &caption, const QString &dir, const QString &filter, bool canBrowse, QWidget* options)
: _options(options), _caption(caption), _dir(dir), _filter(filter)
{
	InitUi(canBrowse);
}

void OpenFileDialog::InitUi(bool canBrowse)
{
	_ui.setupUi(this);

	if (canBrowse)
	{
	  QObject::connect(_ui.browseButton, SIGNAL(clicked()), this, SLOT(showBrowse()));
	}
	else
	{
		_ui.browseButton->setVisible(false);
	}

	if (_options)
	{
		QVBoxLayout *vbox = new QVBoxLayout;
		vbox->addWidget(_options);
		_ui.optionsBox->setLayout(vbox);
		_ui.optionsBox->setMinimumHeight(_options->height());
	}
	else
	{
		_ui.optionsBox->setVisible(false);
	}
}

QString OpenFileDialog::getUrl()
{
	return _ui.urlText->text();
}

void OpenFileDialog::showBrowse()
{
	QString filename = QFileDialog::getOpenFileName(this, _caption, _dir, _filter);

	if (!filename.isNull())
		_ui.urlText->setText(filename);
}