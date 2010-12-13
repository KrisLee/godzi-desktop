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
#include "AppSettingsDialog"

AppSettingsDialog::AppSettingsDialog(bool cacheEnabled, const std::string& cachePath)
{
	initUi(cacheEnabled, cachePath);
}

void AppSettingsDialog::initUi(bool cacheEnabled, const std::string& cachePath)
{
	_ui.setupUi(this);

	_ui.cacheEnabledCheckBox->setChecked(cacheEnabled);

	if (!cachePath.empty())
	{
		std::string tempStr(cachePath);
		_ui.cachePathLineEdit->setText( QString(tempStr.c_str()) );
	}

	//_ui.cacheMaxSpinBox->setValue(cacheMax);

	QObject::connect(_ui.cacheBrowseButton, SIGNAL(clicked()), this, SLOT(showBrowse()));
	QObject::connect(_ui.cacheEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateCacheStates()));

	updateCacheStates();
}

void AppSettingsDialog::updateCacheStates()
{
	bool cacheEnabled = _ui.cacheEnabledCheckBox->isChecked();
	_ui.cachePathLabel->setEnabled(cacheEnabled);
	_ui.cachePathLineEdit->setEnabled(cacheEnabled);
	_ui.cacheBrowseButton->setEnabled(cacheEnabled);
	//_ui.cacheMaxLabel->setEnabled(cacheEnabled);
	//_ui.cacheMaxSpinBox->setEnabled(cacheEnabled);
}

void AppSettingsDialog::showBrowse()
{
	QString cachePath = QFileDialog::getExistingDirectory(this);

	if (!cachePath.isNull())
		_ui.cachePathLineEdit->setText(cachePath);
}